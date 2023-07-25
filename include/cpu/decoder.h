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

#ifndef LIBRE_NES_CPU_DECODER_H
#define LIBRE_NES_CPU_DECODER_H

// Decoding logic for the 6502-like processor

#include <stdint.h>
#include "cpu/definitions.h"

// Decode an 8-bit opcode, translating it into an operation-addressing mode
// pair that can be more easily processed by the CPU
Instruction decode(uint8_t opcode);

#endif // LIBRE_NES_CPU_DECODER_H
