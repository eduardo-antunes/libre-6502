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

#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "emulator.h"
#include "processor.h"

// Temporary starting point of emulated programs
#define PROG_START 0x0200

// Initialize the state of the emulator
void emulator_init(Emulator *nes) {
    processor_init(&nes->proc, nes);
    memset(nes->ram, 0, 2048);
}

// Read data from a particular address in memory
uint8_t emulator_read(const Emulator *nes, uint16_t addr) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        return nes->ram[addr & 0x07FF];
    else if(addr == 0xFFFC)
        // This address must contain the low byte of the program start
        return PROG_START & 0x00FF;
    else if(addr == 0xFFFD)
        // This address must contain the high byte of the program start
        return (PROG_START & 0xFF00) >> 8;
    return 0;
}

// Write data to a particular address in memory
void emulator_write(Emulator *nes, uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        nes->ram[addr & 0x07FF] = data;
}

// Load some instructions, for testing purposes
void emulator_load_prog(Emulator *nes, uint8_t prog[], int n) {
    uint16_t prog_addr = PROG_START;
    for(int i = 0; i < n; ++i)
        emulator_write(nes, prog_addr + i, prog[i]);
}
