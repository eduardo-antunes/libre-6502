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

// Initialize/reset the state of the CPU
void processor_init(Processor *proc, Emulator *nes) {
    proc->x = 0;
    proc->y = 0;
    proc->acc = 0;
    proc->status = 0;
    proc->sp = 0xFF;
    // read the PC from memory
    proc->pc = emulator_read(nes, 0xFFFC);
    proc->pc |= emulator_read(nes, 0xFFFD) << 8;
    // connect to the outside world
    proc->nes = nes;
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
    emulator_write(proc->nes, STACK_BASE | proc->sp - 1, word >> 8);
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
    word |= emulator_read(proc->nes, STACK_BASE | proc->sp + 1);
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
            fprintf(stderr, "(!) Unrecognized addressing mode: %d\n",
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

// General logic of the branching instructions
static void processor_branch(Processor *proc, Processor_flag flag, bool state) {
    bool f = get_flag(proc, flag);
    if(f == state) proc->pc = get_address(proc);
}

// Run a single step of execution, reading code from the main memory
void processor_step(Processor *proc) {
    uint8_t data; uint16_t addr;
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
        // TODO arithmetic instructions: ADC, SBC, CMP, CPX, CPY
        // Incremente operations:
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
        // TODO Shift operations: LSR, ROL, ROR
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
            processor_branch(proc, FLAG_ZERO, true);
            break;
        case BNE:
            // BNE: branch if not equal (zero flag is clear)
            processor_branch(proc, FLAG_ZERO, false);
            break;
        case BCS:
            // BCS: branch if carry is set
            processor_branch(proc, FLAG_CARRY, true);
            break;
        case BCC:
            // BCC: branch if carry is clear
            processor_branch(proc, FLAG_CARRY, false);
            break;
        case BMI:
            // BCS: branch if negative (negative flag is set)
            processor_branch(proc, FLAG_NEGATIVE, true);
            break;
        case BPL:
            // BPL: branch if positive (negative flag is clear)
            processor_branch(proc, FLAG_NEGATIVE, false);
            break;
        case BVS:
            // BVS: branch if an overflow happened (overflow flag is set)
            processor_branch(proc, FLAG_OVERFLOW, true);
            break;
        case BVC:
            // BVC: branch if no overflow happened (overflow flag is clear)
            processor_branch(proc, FLAG_OVERFLOW, false);
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
            // Should not happen
            fprintf(stderr, "(?) Unimplemented instruction: %02X\n", opcode);
    }
}
