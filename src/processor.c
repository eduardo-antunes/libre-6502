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

// Pop/pull a byte from the stack in main memory
static uint8_t stack_pull(Processor *proc) {
    // NOTE the stack shrinks upward in the 6502
    return emulator_read(proc->nes, STACK_BASE | ++proc->sp);
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

// Based on the current addressing mode, get an absolute address for the
// current instruction to work with. Advances the PC
static uint16_t get_address(Processor *proc) {
    uint16_t addr, ptr;
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

// Run a single step of execution, reading code from memory
void processor_step(Processor *proc) {
    uint8_t opcode = emulator_read(proc->nes, proc->pc++);
    processor_decode(&proc->inst, opcode);
}
