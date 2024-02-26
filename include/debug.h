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

#ifndef LIBRE_6502_DEBUG_H
#define LIBRE_6502_DEBUG_H

// Debugging facilities for libre-6502

#include <stdio.h>
#include <stdint.h>
#include "processor.h"

// Read code from the given addressing space (provided via the userdata and
// read parameters) at the given address and with the given length,
// decoding and disassembling it to the given file
void disassemble(FILE *out, void *userdata, AddrReader read,
        uint16_t addr, size_t code_length);

#endif // LIBRE_6502_DEBUG_H
