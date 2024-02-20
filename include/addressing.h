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

#ifndef LIBRE_6502_ADDRESSING_H
#define LIBRE_6502_ADDRESSING_H

// Addressing mode logic for the 6502 CPU. Addressing modes can be understood
// as hardware-level polymorphism: they enable the same operation to take
// different kinds of arguments. The main purpose of this module is to decouple
// the execution of each operation from the exact addressing mode that it is
// using.

#include <stdint.h>
#include "processor.h"

// Based on the current addressing mode, get an absolute address for the
// current instruction to work with. Does not advance the PC!
uint16_t get_address(const Processor *proc);

// Based on the current addressing mode, get an 8-bit value for the current
// instruction to work with. Calls get_address internally and thus also does
// not advance the PC. If the caller also needs the address that corresponds
// to the data, they may optionally get it through the address parameter
uint8_t get_data(const Processor *proc, uint16_t *address);

// Based on the current addressing mode, determine by how much the PC should
// be incremented to get to the next instruction
uint8_t get_inc(const Processor *proc);

#endif // LIBRE_6502_ADDRESSING_H
