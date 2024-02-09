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
// current instruction to work with. Advances the PC
uint16_t get_address(Processor *proc);

// Based on the current addressing mode, get an 8-bit value for the current
// instruction to work with. Calls get_address internally and thus also
// advances the PC. If the caller also needs the result of the call to
// get_address, which happens in instructions that modify memory locations, it
// can optionally get it through the address pointer
uint8_t get_data(Processor *proc, uint16_t *address);

#endif // LIBRE_6502_ADDRESSING_H
