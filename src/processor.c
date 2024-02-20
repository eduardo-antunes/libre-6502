/*
   Copyright 2024 Eduardo Antuc S. Vieira <eduardoantuc986@gmail.com>

   This file is part of libre-6502.

   libre-6502 is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

   libre-6502 is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with
   libre-6502. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "decoder.h"
#include "processor.h"
#include "addressing.h"
#include "debug.h"

// Base address of the stack. It is confined to page #01, and its primary use
// is to store return addresses (though it can be used for arbitrary data
// storage). It grows downward and shrinks upward
#define STACK_BASE 0x0100

// Push a byte to the stack in main memory
static void stack_push(Processor *proc, uint8_t u) {
    proc->write(proc->u, STACK_BASE | proc->sp--, u);
}

// Push a 16-bit value to the stack in main memory
static void stack_push16(Processor *proc, uint16_t w) {
    proc->write(proc->u, STACK_BASE | proc->sp--, w & 0x00FF);
    proc->write(proc->u, STACK_BASE | proc->sp--, w >> 8);
}

// Pop/pull a byte from the stack in main memory
static uint8_t stack_pull(Processor *proc) {
    return proc->read(proc->u, STACK_BASE | ++proc->sp);
}

// Pop/pull a 16-bit value from the stack in main memory
static uint16_t stack_pull16(Processor *proc) {
    uint16_t w = proc->read(proc->u, STACK_BASE | ++proc->sp) << 8;
    w |= proc->read(proc->u, STACK_BASE | ++proc->sp);
    return w;
}

// Read a 16-bit address from the address space
static uint16_t read_address(Processor *proc, uint16_t addr) {
    uint16_t address = proc->read(proc->u, addr);
    address |= proc->read(proc->u, addr + 1) << 8;
    return address;
}

// Get the current state of a particular flag in the status register
static uint8_t get_flag(Processor *proc, Processor_flag flag) {
    return proc->status & flag;
}

// Set or clear a particular flag in the status register
static void set_flag(Processor *proc, Processor_flag flag, bool state) {
    if(state) proc->status |= flag;
    else proc->status &= ~flag;
}

// Set or clear the zero and negative flags based on a value; this is a very
// common idiom in the processor
static void set_zn(Processor *proc, uint8_t data) {
    set_flag(proc, FLAG_ZERO, data == 0);
    set_flag(proc, FLAG_NEGATIVE, data & 0x80);
}

// Initializes a new processor instance, connecting it to its address space
void processor_init(Processor *proc, AddrReader read,
    AddrWriter write, void *userdata) {
    proc->read = read;
    proc->write = write;
    proc->u = userdata;
    processor_reset(proc);
}

// Initialize/reset the state of the CPU
void processor_reset(Processor *proc) {
    proc->x = 0;
    proc->y = 0;
    proc->acc = 0;
    proc->sp = 0xFD;
    proc->status = 0x34; // IRQ starts disabled
    proc->pc = read_address(proc, RESET_VECTOR);
}

// Request a CPU interruption (IRQ)
void processor_request(Processor *proc) {
    // If IRQ has been disabled, ignore this request
    if(get_flag(proc, FLAG_ID)) return;
    // When an interrupt happens, the PC and status registers are pushed onto
    // the stack and a new value for the PC is loaded from an interrupt vector,
    // stored at a fixed location in memory
    stack_push16(proc, proc->pc);
    stack_push(proc, proc->status);
    set_flag(proc, FLAG_ID, true); // disable IRQ
    proc->pc = read_address(proc, IRQ_VECTOR);
}

// Generate a non-maskable CPU interruption (NMI)
void processor_interrupt(Processor *proc) {
    // When an interrupt happens, the PC and status registers are pushed onto
    // the stack and a new value for the PC is loaded from an interrupt vector,
    // stored at a fixed location in memory
    stack_push16(proc, proc->pc);
    stack_push(proc, proc->status);
    set_flag(proc, FLAG_ID, true); // disable IRQ
    proc->pc = read_address(proc, NMI_VECTOR);
}

// Operation of addition in the processor
static void processor_add(Processor *proc) {
    uint8_t data = get_data(proc, NULL);
    // The result has to be stored in 16 bits to detect carry out. This is a
    // poor man's substitute for the carry out signal in the original hardware
    uint16_t sum = proc->acc + data + get_flag(proc, FLAG_CARRY);

    // Here we check for carry out
    set_flag(proc, FLAG_CARRY, sum & 0x0100);
    // Here we ignore a potential carry out
    set_flag(proc, FLAG_ZERO, (sum & 0xFF) == 0);
    set_flag(proc, FLAG_NEGATIVE, sum & 0x80);

    // The overflow flag must be set if the sign of the result is incorrect
    // from a mathematical standpoint. That will be the case if its sign bit
    // is different from that of both operands, because pos + pos can't equal
    // a negative and neg + neg can't equal a positive
    set_flag(proc, FLAG_OVERFLOW, (sum ^ proc->acc) & (sum ^ data) & 0x80);
    proc->acc = (uint8_t) (sum & 0xFF);
}

// Operation of subtraction in the processor
static void processor_sub(Processor *proc) {
    uint8_t data = get_data(proc, NULL);
    // The result has to be stored in 16 bits to detect carry out. This is a
    // poor man's substitute for the carry out signal in the original hardware
    uint16_t diff = proc->acc - data - !get_flag(proc, FLAG_CARRY);

    // Here we check for a borrow
    set_flag(proc, FLAG_CARRY, !(diff & 0x0100));
    // Here we ignore a potential borrow
    set_flag(proc, FLAG_ZERO, (diff & 0xFF) == 0);
    set_flag(proc, FLAG_NEGATIVE, diff & 0x80);

    // The overflow flag must be set if the sign of the result is incorrect
    // from a mathematical standpoint. That will be the case if its sign bit
    // is different from that of the first operand and equal to that of the
    // second, because pos - neg can't equal a negative and neg - pos can't
    // equal a positive
    set_flag(proc, FLAG_OVERFLOW, (diff ^ proc->acc) & ~(diff ^ data) & 0x80);
    proc->acc = (uint8_t) (diff & 0xFF);
}

// General logic of the branching instructions
static void branch(Processor *proc, Processor_flag flag, bool state) {
    bool f = get_flag(proc, flag);
    uint16_t addr = get_address(proc);
    if(f == state) proc->pc = addr;
}

// Execute a single instruction (it must have been read already)
static void processor_execute(Processor *proc) {
    uint16_t addr;
    uint8_t data, aux;
    switch(proc->inst.op) {
        // Load and store operations:
        case LDA:
            // LDA: load given data into the accumulator
            proc->acc = get_data(proc, NULL);
            set_zn(proc, proc->acc);
            break;
        case LDX:
            // LDX: load given data into the x register
            proc->x = get_data(proc, NULL);
            set_zn(proc, proc->x);
            break;
        case LDY:
            // LDY: load given data into the y register
            proc->y = get_data(proc, NULL);
            set_zn(proc, proc->y);
            break;
        case STA:
            // STA: store the contents of the accumulator into the given address
            addr = get_address(proc);
            proc->write(proc->u, addr, proc->acc);
            break;
        case STX:
            // STX: store the contents of the x register into the given address
            addr = get_address(proc);
            proc->write(proc->u, addr, proc->x);
            break;
        case STY:
            // STY: store the contents of the y register into the given address
            addr = get_address(proc);
            proc->write(proc->u, addr, proc->y);
            break;
        // Register transfer operations:
        case TAX:
            // TAX: copy the accumulator into the x register
            proc->x = proc->acc;
            set_zn(proc, proc->x);
            break;
        case TAY:
            // TAY: copy the accumulator into the y register
            proc->y = proc->acc;
            set_zn(proc, proc->y);
            break;
        case TXA:
            // TXA: copy the x register into the accumulator
            proc->acc = proc->x;
            set_zn(proc, proc->acc);
            break;
        case TYA:
            // TYA: copy the y register into the accumulator
            proc->acc = proc->y;
            set_zn(proc, proc->acc);
            break;
        case TSX:
            // TSX: copy the stack pointer into the x register
            proc->x = proc->sp;
            set_zn(proc, proc->x);
            break;
        case TXS:
            // TXS: copy the x register into the stack pointer
            proc->sp = proc->x;
            break;
        // Stack operations:
        case PHA:
            // PHA: push the accumulator on the stack
            stack_push(proc, proc->acc);
            break;
        case PHP:
            // PHP: push the status register on the stack
            set_flag(proc, FLAG_BRK, true);
            stack_push(proc, proc->status);
            break;
        case PLA:
            // PLA: pull a byte from the stack and put it in the accumulator
            proc->acc = stack_pull(proc);
            set_zn(proc, proc->acc);
            break;
        case PLP:
            // PLP: pull a byte from the stack and put it in the status register
            proc->status = stack_pull(proc);
            break;
        // Logic operations:
        case AND:
            // AND: bitwise AND data into the accumulator
            proc->acc &= get_data(proc, NULL);
            set_zn(proc, proc->acc);
            break;
        case EOR:
            // EOR: bitwise XOR data into the accumulator
            proc->acc ^= get_data(proc, NULL);
            set_zn(proc, proc->acc);
            break;
        case ORA:
            // ORA: bitwise OR data into the accumulator
            proc->acc |= get_data(proc, NULL);
            set_zn(proc, proc->acc);
            break;
        case BIT:
            // BIT: bitwise AND data with the accumulator, but the result isn't
            // kept. It is instead used to set some CPU flags
            data = get_data(proc, NULL) & proc->acc;
            set_flag(proc, FLAG_OVERFLOW, data & 0x40);
            set_zn(proc, data);
            break;
        // Arithmetic instructions:
        case ADC:
            // ADC: Add the given data and the carry flag to the accumulator
            processor_add(proc);
            break;
        case SBC:
            // SBC: subtract the given data and the negation of the carry flag (which
            // represents a borrow) from the accumulator
            processor_sub(proc);
            break;
        case CMP:
            // CMP: compare the contents of the accumulator and the given data,
            // setting the appropriate flags in the status register
            data = get_data(proc, NULL);
            set_flag(proc, FLAG_CARRY, proc->acc >= data);
            set_flag(proc, FLAG_ZERO, proc->acc == data);
            set_flag(proc, FLAG_NEGATIVE, proc->acc < data);
            break;
        case CPX:
            // CPX: compare the contents of the x register and the given data,
            // setting the appropriate flags in the status register
            data = get_data(proc, NULL);
            set_flag(proc, FLAG_CARRY, proc->x >= data);
            set_flag(proc, FLAG_ZERO, proc->x == data);
            set_flag(proc, FLAG_NEGATIVE, proc->x < data);
            break;
        case CPY:
            // CPY: compare the contents of the y register and the given data,
            // setting the appropriate flags in the status register
            data = get_data(proc, NULL);
            set_flag(proc, FLAG_CARRY, proc->y >= data);
            set_flag(proc, FLAG_ZERO, proc->y == data);
            set_flag(proc, FLAG_NEGATIVE, proc->y < data);
            break;
        // Increment operations:
        case INC:
            // INC: increment the memory location at the given address
            data = get_data(proc, &addr);
            proc->write(proc->u, addr, data + 1);
            set_zn(proc, data + 1);
            break;
        case INX:
            // INX: increment the x register
            set_zn(proc, ++proc->x);
            break;
        case INY:
            // INY: increment the y register
            set_zn(proc, ++proc->y);
            break;
        // Decrement operations:
        case DEC:
            // DEC: decrement the memory location at the given address
            data = get_data(proc, &addr);
            proc->write(proc->u, addr, data - 1);
            set_zn(proc, data + 1);
            break;
        case DEX:
            // DEX: decrement the x register
            set_zn(proc, --proc->x);
            break;
        case DEY:
            // INY: decrement the x register
            set_zn(proc, --proc->y);
            break;
        // Shift operations:
        case ASL:
            // ASL: arithmetic left shift of the memory location at the given
            // address or the accumulator, depending on the addressing mode
            data = get_data(proc, &addr);
            set_flag(proc, FLAG_CARRY, data & 0x80);
            data <<= 1;
            set_zn(proc, data);
            if(proc->inst.mode != MODE_ACCUMULATOR)
                proc->write(proc->u, addr, data);
            else
                proc->acc = data;
            break;
        case LSR:
            // LSR: logical right shift of the memory location at the given
            // address or the accumulator, depending on the addressing mode
            data = get_data(proc, &addr);
            set_flag(proc, FLAG_CARRY, data & 0x01);
            data >>= 1;
            set_zn(proc, data);
            if(proc->inst.mode != MODE_ACCUMULATOR)
                proc->write(proc->u, addr, data);
            else
                proc->acc = data;
            break;
        case ROL:
            // ROL: rotate to the left the memory location at the given address
            // or the accumulator, depending on the addressing mode
            data = get_data(proc, &addr);
            aux = data & 0x80; // leftmost bit (7), to be put in the carry flag
            data <<= 1;
            // The rightmost bit (0) is filled with the current carry flag
            data |= get_flag(proc, FLAG_CARRY);
            set_flag(proc, FLAG_CARRY, aux);
            set_zn(proc, data);
            if(proc->inst.mode != MODE_ACCUMULATOR)
                proc->write(proc->u, addr, data);
            else
                proc->acc = data;
            break;
        case ROR:
            // ROR: rotate to the right the memory location at the given
            // address or the accumulator, depending on the addressing mode
            data = get_data(proc, &addr);
            aux = data & 0x01; // rightmost bit (0), to be put in the carry flag
            data >>= 1;
            // The leftmost bit (7) is filled with the current carry flag
            data |= get_flag(proc, FLAG_CARRY) << 7;
            set_flag(proc, FLAG_CARRY, aux);
            set_zn(proc, data);
            if(proc->inst.mode != MODE_ACCUMULATOR)
                proc->write(proc->u, addr, data);
            else
                proc->acc = data;
            break;
        // Jump operations:
        case JMP:
            // JMP: unconditional jump to the given address
            proc->pc = get_address(proc);
            break;
        case JSR:
            // JSR: jump to subroutine. It pushes the current value of the PC to
            // the stack and then does an unconditional jump to the given address.
            // This way, a future RTS can return to the calling code
            stack_push16(proc, proc->pc);
            proc->pc = get_address(proc);
            break;
        case RTS:
            // RTS: return from subroutine. It pulls a 16-bit address from the
            // stack and puts it into the PC, thus returning to the calling code
            proc->pc = stack_pull16(proc);
            break;
        // Branch operations:
        case BEQ:
            // BEQ: branch if equal (zero flag is set)
            branch(proc, FLAG_ZERO, true);
            break;
        case BNE:
            // BNE: branch if not equal (zero flag is clear)
            branch(proc, FLAG_ZERO, false);
            break;
        case BCS:
            // BCS: branch if carry is set
            branch(proc, FLAG_CARRY, true);
            break;
        case BCC:
            // BCC: branch if carry is clear
            branch(proc, FLAG_CARRY, false);
            break;
        case BMI:
            // BCS: branch if negative (negative flag is set)
            branch(proc, FLAG_NEGATIVE, true);
            break;
        case BPL:
            // BPL: branch if positive (negative flag is clear)
            branch(proc, FLAG_NEGATIVE, false);
            break;
        case BVS:
            // BVS: branch if an overflow happened (overflow flag is set)
            branch(proc, FLAG_OVERFLOW, true);
            break;
        case BVC:
            // BVC: branch if no overflow happened (overflow flag is clear)
            branch(proc, FLAG_OVERFLOW, false);
            break;
        // Flag operations:
        case SEC:
            // SEC: set carry flag
            set_flag(proc, FLAG_CARRY, true);
            break;
        case SEI:
            // SEI: set interrupt disable flag
            set_flag(proc, FLAG_ID, true);
            break;
        case SED:
            // SED: set decimal flag (no effect in the NES)
            set_flag(proc, FLAG_DEC, true);
            break;
        case CLC:
            // CLC: clear carry flag
            set_flag(proc, FLAG_CARRY, false);
            break;
        case CLI:
            // CLI: clear interrupt disable flag
            set_flag(proc, FLAG_ID, false);
            break;
        case CLD:
            // CLD: clear decimal flag (no effect in the NES)
            set_flag(proc, FLAG_DEC, false);
            break;
        case CLV:
            // CLV: clear overflow flag
            set_flag(proc, FLAG_OVERFLOW, false);
            break;

        // System/symbolic operations:
        case BRK:
            // BRK: force an interrupt (IRQ), setting the BRK flag
            set_flag(proc, FLAG_BRK, true);
            processor_request(proc);
            break;
        case NOP:
            // NOP: do nothing
            break;
        case RTI:
            // RTI: return from an interrupt handler
            proc->status = stack_pull(proc); // restore status register
            set_flag(proc, FLAG_BRK, false);
            set_flag(proc, FLAG_NIL, false);
            proc->pc = stack_pull16(proc);
            break;
        case ERR:
            // ERR: indicative of a decoder error, reported by the decoder itself
            break;
    }
    proc->pc += get_inc(proc->inst.mode); // advance to the next instruction
}

// Run a single clock cycle of execution
void processor_step(Processor *proc) {
    uint8_t opcode = proc->read(proc->u, proc->pc++);
    proc->inst = decode(opcode);
    processor_execute(proc);
}

// Run a single clock cycle of execution (debugging version)
void processor_step_debug(Processor *proc) {
    uint8_t opcode = proc->read(proc->u, proc->pc++);
    proc->inst = decode(opcode);
    printf("Read opcode: %02X\nIn assembly: ", opcode);
    disassemble(proc);
    processor_execute(proc);
}
