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

#ifndef LIBRE_6502_DECODER_H
#define LIBRE_6502_DECODER_H

// Decoding logic for the 6502 processor

#include <stdint.h>
#include "definitions.h"

// Decode an 8-bit opcode, translating it into an operation-addressing mode
// pair that can be more easily processed by the CPU
Instruction decode(uint8_t opcode);

#endif // LIBRE_6502_DECODER_H
