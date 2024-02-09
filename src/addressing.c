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
#include "processor.h"
#include "computer.h"

// Read an 8-bit value from the memory location specified by the PC register,
// advancing it to the following location in the process
static uint8_t read_forward(Processor *proc) {
    return address_read(proc->c, proc->pc++);
}

// Read a 16-bit address from the memory location specified by the PC register,
// advancing it two locations forward in the process
static uint16_t read_address_forward(Processor *proc) {
    uint16_t address = address_read(proc->c, proc->pc++);
    address |= address_read(proc->c, proc->pc++) << 8;
    return address;
}

// Based on the current addressing mode, get an absolute address for the
// current instruction to work with. Advances the PC
uint16_t get_address(Processor *proc) {
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
            addr = read_forward(proc);
            break;
        case MODE_ZEROPAGE_X:
            // The contents of the x register are added to the zero page
            // address in the following byte to produce the final address
            addr = (read_forward(proc) + proc->x) & 0xFF;
            break;
        case MODE_ZEROPAGE_Y:
            // The contents of the y register are added to the zero page
            // address in the following byte to produce the final address
            addr = (read_forward(proc) + proc->x) & 0xFF;
            break;
        case MODE_RELATIVE:
            // Exclusive to branching instructions. The following byte contains
            // a signed jump offset, which should be added to the current value
            // of the PC (after reading the instruction) to get the raw address
            addr = read_forward(proc);
            if(addr & 0x80) addr |= 0xFF00; // sign extension
            addr += proc->pc;
            break;
        case MODE_ABSOLUTE:
            // The following two bytes contain an absolute, 16-bit address
            addr = read_address_forward(proc);
            break;
        case MODE_ABSOLUTE_X:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the x register
            addr = read_address_forward(proc) + proc->x;
            break;
        case MODE_ABSOLUTE_Y:
            // The following two bytes contain an absolute, 16-bit address,
            // which is to be added with the contents of the y register
            addr = read_address_forward(proc) + proc->y;
            break;
        case MODE_INDIRECT:
            // The following two bytes contain a 16-bit pointer to the real
            // absolute address. NOTE this thing had a bug in the original CPU
            ptr = read_address_forward(proc);
            addr = address_read(proc->c, ptr);
            // NOTE the original bug is reproduced by this line:
            ptr = (ptr & 0x00FF) == 0x00FF ? ptr & 0xFF00 : ptr + 1;
            addr |= address_read(proc->c, ptr) << 8;
            break;
        case MODE_INDIRECT_X:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the x register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = (read_forward(proc) + proc->x) & 0xFF;
            addr = address_read(proc->c, ptr);
            addr |= address_read(proc->c, (ptr + 1) & 0xFF) << 8;
            break;
        case MODE_INDIRECT_Y:
            // The following byte contains a zero page address, which is to be
            // added to the contents of the y register, with zero page wrap
            // around, to get a pointer to the real absolute address
            ptr = (read_forward(proc) + proc->x) & 0xFF;
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
// instruction to work with. Calls get_address internally and thus also
// advances the PC. If the caller also needs the result of the call to
// get_address, which happens in instructions that modify memory locations, it
// can optionally get it through the address pointer
uint8_t get_data(Processor *proc, uint16_t *address) {
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
            return read_forward(proc);
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
