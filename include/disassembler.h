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

// NES machine code disassembler, for developers

#ifndef LIBRE_NES_DISASSEMBLER_H
#define LIBRE_NES_DISASSEMBLER_H

#include <stdio.h>
#include "emulator.h"

// Disassemble a program stored in main memory
void disassemble(FILE *fp, const Emulator *nes, int nr_instructions);

#endif // LIBRE_NES_DISASSEMBLER_H
