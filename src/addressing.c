/*
   Copyright 2024 Eduardo Antunes S. Vieira <eduardoantunes986@gmail.com>

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
#include <stdint.h>

#include "addressing.h"
#include "definitions.h"
#include "processor.h"
#include "computer.h"

// Read the 8-bit value following the opcode
static inline uint8_t next_byte(const Processor *proc) {
    return address_read(proc->c, proc->pc);
}

// Read the 16-bit address following the opcode
static inline uint16_t next_address(const Processor *proc) {
    uint16_t address = address_read(proc->c, proc->pc);
    address |= address_read(proc->c, proc->pc + 1) << 8;
    return address;
}

// Based on the current addressing mode, get an absolute address for the
// current instruction to work with. Does not advance the PC!
uint16_t get_address(const Processor *proc) {
    uint16_t addr = 0, ptr = 0;
    switch(proc->inst.mode) {
        case MODE_IMPLIED:
            // No absolute address to fetch
            addr = 0;
            break;
        case MODE_ACCUMULATOR:
            // No absolute address to fetch
            addr = 0;
            break;
        case MODE_IMMEDIATE:
            // No absolute address to fetch
            addr = 0;
            break;
        case MODE_ZEROPAGE:
            // A zero page address is stored in the following byte
            addr = next_byte(proc);
            break;
        case MODE_ZEROPAGE_X:
            // The contents of the x register are added to the zero page
            // address in the following byte to produce the final address
            addr = (next_byte(proc) + proc->x) & 0xFF;
            break;
        case MODE_ZEROPAGE_Y:
            // The contents of the y register are added to the zero page
            // address in the following byte to produce the final address
            addr = (next_byte(proc) + proc->x) & 0xFF;
            break;
        case MODE_RELATIVE:
            // Exclusive to branching instructions. The following byte contains
            // a signed jump offset, which should be added to the current value
            // of the PC (after reading the instruction) to get the raw address
            addr = next_byte(proc);
            if(addr & 0x80) addr |= 0xFF00; // sign extension
            addr += proc->pc;
            break;
        case MODE_ABSOLUTE:
            // The following two bytes contain an absolute, 16-bit address
            addr = next_address(proc);
            break;
        case MODE_ABSOLUTE_X:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the x register
            addr = next_address(proc) + proc->x;
            break;
        case MODE_ABSOLUTE_Y:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the y register
            addr = next_address(proc) + proc->y;
            break;
        case MODE_INDIRECT:
            // The following two bytes contain a 16-bit pointer to the real
            // absolute address. NOTE this thing had a bug in the original CPU
            ptr = next_address(proc);
            addr = address_read(proc->c, ptr);
            // NOTE the original bug is reproduced by this line:
            ptr = (ptr & 0x00FF) == 0x00FF ? ptr & 0xFF00 : ptr + 1;
            addr |= address_read(proc->c, ptr) << 8;
            break;
        case MODE_INDIRECT_X:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the x register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = (next_byte(proc) + proc->x) & 0xFF;
            addr = address_read(proc->c, ptr);
            addr |= address_read(proc->c, (ptr + 1) & 0xFF) << 8;
            break;
        case MODE_INDIRECT_Y:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the y register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = (next_byte(proc) + proc->x) & 0xFF;
            addr = address_read(proc->c, ptr);
            addr |= address_read(proc->c, (ptr + 1) & 0xFF) << 8;
            break;
        default:
            fprintf(stderr, "[!] Unrecognized addressing mode: %d\n",
                proc->inst.mode);
    }
    return addr;
}

// Based on the current addressing mode, get an 8-bit value for the current
// instruction to work with. Calls get_address internally and thus also does
// not advance the PC. If the caller also needs the address that corresponds
// to the data, they may optionally get it through the address parameter
uint8_t get_data(const Processor *proc, uint16_t *address) {
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
            return next_byte(proc);
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
            return address_read(proc->c, addr);
    }
}

// Based on the current addressing mode, determine by how much the PC should
// be incremented to get to the next instruction
uint8_t get_inc(const Processor *proc) {
    if(proc->inst.mode <= MODE_ACCUMULATOR) return 0;
    if(proc->inst.mode <= MODE_RELATIVE) return 1;
    return 2;
}
