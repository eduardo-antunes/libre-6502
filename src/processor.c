/*
   Copyright 2023 Eduardo Antunes S. Vieira <eduardoantunes986@gmail.com>

   This file is part of libre-nes.

   libre-nes is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

   libre-nes is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with
   libre-nes. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>

#include "decoder.h"
#include "emulator.h"
#include "processor.h"

// Get the initial value for the PC register
uint16_t processor_init_pc(const Emulator *nes) {
    uint16_t pc = emulator_read(nes, 0xFFFC);
    pc |= emulator_read(nes, 0xFFFD) << 8;
    return pc;
}

// Initialize/reset the state of the CPU
void processor_init(Processor *proc, Emulator *nes) {
    proc->x = 0;
    proc->y = 0;
    proc->acc = 0;
    proc->status = 0;
    proc->sp = 0xFF;
    proc->nes = nes; // connect to the outside world
    proc->pc = processor_init_pc(nes); // read PC from memory
}

// Push a byte to the stack in main memory
static void stack_push(Processor *proc, uint8_t data) {
    // NOTE the stack grows downward in the 6502
    emulator_write(proc->nes, STACK_BASE | proc->sp--, data);
}

// Push a 16-bit value to the stack in main memory
static void stack_push16(Processor *proc, uint16_t word) {
    // NOTE the stack grows downward in the 6502
    emulator_write(proc->nes, STACK_BASE | proc->sp, word & 0x00FF);
    emulator_write(proc->nes, STACK_BASE | (proc->sp - 1), word >> 8);
    proc->sp -= 2;
}

// Pop/pull a byte from the stack in main memory
static uint8_t stack_pull(Processor *proc) {
    // NOTE the stack shrinks upward in the 6502
    return emulator_read(proc->nes, STACK_BASE | ++proc->sp);
}

// Pop/pull a 16-bit value from the stack in main memory
static uint16_t stack_pull16(Processor *proc) {
    // NOTE the stack shrinks upward in the 6502
    uint16_t word = emulator_read(proc->nes, STACK_BASE | proc->sp) << 8;
    word |= emulator_read(proc->nes, STACK_BASE | (proc->sp + 1));
    proc->sp += 2;
    return word;
}

// Get the current state of a particular flag in the status register
static uint8_t get_flag(Processor *proc, Processor_flag flag) {
    return (proc->status & (1 << flag)) >> flag;
}

// Set or clear a particular flag in the status register
static void set_flag(Processor *proc, Processor_flag flag, bool state) {
    if(state) proc->status |= (1 << flag);
    else proc->status &= ~(1 << flag);
}

// Set or clear the zero and negative flags based on some value
static void set_zn(Processor *proc, uint8_t data) {
    set_flag(proc, FLAG_ZERO, data == 0);
    set_flag(proc, FLAG_NEGATIVE, data & 0x80);
}

// Based on the current addressing mode, get an absolute address for the
// current instruction to work with. Advances the PC
static uint16_t get_address(Processor *proc) {
    uint16_t addr = 0, ptr = 0;
    switch(proc->inst.mode) {
        case MODE_IMPLIED:
            // No absolute address to fetch
            addr = 0;
            break;
        case MODE_IMMEDIATE:
            // No absolute address to fetch
            addr = 0;
            break;
        case MODE_ZEROPAGE:
            // A zero page address is stored in the following byte
            addr = emulator_read(proc->nes, proc->pc++);
            break;
        case MODE_ZEROPAGE_X:
            // The contents of the x register are added to the zero page
            // address in the following byte to produce the final address
            addr = (emulator_read(proc->nes, proc->pc++) + proc->x) & 0x00FF;
            break;
        case MODE_ZEROPAGE_Y:
            // The contents of the y register are added to the zero page
            // address in the following byte to produce the final address
            addr = (emulator_read(proc->nes, proc->pc++) + proc->y) & 0x00FF;
            break;
        case MODE_RELATIVE:
            // Exclusive to branching instructions. The following byte contains
            // a signed jump offset, which should be added to the address of
            // the instruction itself to produce the final address to jump to
            addr = emulator_read(proc->nes, proc->pc++);
            if(addr & 0x80) addr |= 0xFF00; // sign extension
            addr += proc->pc - 2;
            break;
        case MODE_ABSOLUTE:
            // The following two bytes contain an absolute, 16-bit address
            addr = emulator_read(proc->nes, proc->pc++);
            addr |= emulator_read(proc->nes, proc->pc++) << 8;
            break;
        case MODE_ABSOLUTE_X:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the x register
            addr = emulator_read(proc->nes, proc->pc++);
            addr |= emulator_read(proc->nes, proc->pc++) << 8;
            addr += proc->x;
            break;
        case MODE_ABSOLUTE_Y:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the y register
            addr = emulator_read(proc->nes, proc->pc++);
            addr |= emulator_read(proc->nes, proc->pc++) << 8;
            addr += proc->y;
            break;
        case MODE_INDIRECT:
            // The following two bytes contain a 16-bit pointer to the real
            // absolute address. NOTE this thing had a bug in the original CPU
            ptr = emulator_read(proc->nes, proc->pc++);
            ptr |= emulator_read(proc->nes, proc->pc++) << 8;
            addr = emulator_read(proc->nes, ptr);
            // NOTE the original bug is reproduced by this line:
            ptr = (ptr & 0x00FF) == 0x00FF ? ptr & 0xFF00 : ptr + 1;
            addr |= emulator_read(proc->nes, ptr) << 8;
            break;
        case MODE_INDIRECT_X:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the x register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = emulator_read(proc->nes, proc->pc++);
            ptr = (ptr + proc->x) & 0x00FF;
            addr = emulator_read(proc->nes, ptr);
            addr |= emulator_read(proc->nes, (ptr + 1) & 0x00FF) << 8;
            break;
        case MODE_INDIRECT_Y:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the y register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = emulator_read(proc->nes, proc->pc++);
            ptr = (ptr + proc->y) & 0x00FF;
            addr = emulator_read(proc->nes, ptr);
            addr |= emulator_read(proc->nes, (ptr + 1) & 0x00FF) << 8;
            break;
        default:
            fprintf(stderr, "[!] Unrecognized addressing mode: %d\n",
                proc->inst.mode);
    }
    return addr;
}

// Based on the current addressing mode, get an 8-bit value for the current
// instruction to work with. Calls get_address internally and thus also
// advances the PC
static uint8_t get_data(Processor *proc, uint16_t *address) {
    uint16_t addr;
    switch(proc->inst.mode) {
        case MODE_IMPLIED:
            // No need to fetch data
            return 0;
        case MODE_ACCUMULATOR:
            // The contents of the accumulator register are used as data
            return proc->acc;
        case MODE_IMMEDIATE:
            // The data is the byte following the instruction
            return emulator_read(proc->nes, proc->pc++);
        case MODE_RELATIVE:
            // It makes no sense to fetch data here, given that the only
            // instructions that use this are branching instructions, which
            // only require addresses to work
            return 0;
        default:
            // For other addressing modes, it's really just a matter of
            // fetching an 8-bit value from the address they specify
            addr = get_address(proc);
            if(address != NULL) *address = addr;
            return emulator_read(proc->nes, addr);
    }
}

// Implementation of the ADC instruction
static void processor_adc(Processor *proc) {
    // ADC: add the given data and the carry flag to the accumulator
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

// Implementation of the SBC instruction
static void processor_sbc(Processor *proc) {
    // SBC: subtract the given data and the negation of the carry flag (which
    // represents a borrow) from the accumulator
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
    if(f == state) proc->pc = get_address(proc);
}

// Run a single clock cycle of execution
void processor_clock(Processor *proc) {
    uint8_t data, aux; uint16_t addr;
    uint8_t opcode = emulator_read(proc->nes, proc->pc++);
    proc->inst = decode(opcode);
    switch(proc->inst.op) {
        case NOP:
            // NOP: do nothing
            break;
        case ERR:
            // ERR: indicative of a decoder error, reported by the decoder itself
            break;
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
            emulator_write(proc->nes, addr, proc->acc);
            break;
        case STX:
            // STX: store the contents of the x register into the given address
            addr = get_address(proc);
            emulator_write(proc->nes, addr, proc->x);
            break;
        case STY:
            // STY: store the contents of the y register into the given address
            addr = get_address(proc);
            emulator_write(proc->nes, addr, proc->y);
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
            processor_adc(proc);
            break;
        case SBC:
            // SBC: subtract the given data and the negation of the carry flag (which
            // represents a borrow) from the accumulator
            processor_sbc(proc);
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
            emulator_write(proc->nes, addr, data + 1);
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
            emulator_write(proc->nes, addr, data - 1);
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
                emulator_write(proc->nes, addr, data);
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
                emulator_write(proc->nes, addr, data);
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
                emulator_write(proc->nes, addr, data);
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
                emulator_write(proc->nes, addr, data);
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
        default:
            fprintf(stderr, "[!] Invalid opcode: %02X\n", opcode);
    }
}

// Display the processor's internal state in a readable format
void processor_display_info(const Processor *proc) {
    printf("PC [0x%04X]\n", proc->pc);
    printf("X [%u] Y [%u] A [%u]\n", proc->x, proc->y, proc->acc);
    // TODO print flags
    printf("STACK_PTR [0x%02X]\n", proc->sp);
}
