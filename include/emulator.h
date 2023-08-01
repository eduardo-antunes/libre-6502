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

#ifndef LIBRE_NES_EMULATOR_H
#define LIBRE_NES_EMULATOR_H

#include <stdint.h>

#include "cartridge.h"
#include "cpu/processor.h"
#include "picture/ppu.h"

// Structure representing the console itself, as well as the main data bus
typedef struct nes {
    Cartridge cart;    // the connected game cartrige
    Processor proc;    // the 6502-like CPU
    Picture_proc ppu;  // the picture processing unit
    uint8_t ram[2048]; // 2KiB of main memory
} Emulator;

// Initialize the state of the emulator, loading an iNES rom file
void emulator_init(Emulator *nes, const char *rom_filepath);

// Start the emulator's operation
void emulator_start(Emulator *nes);

// Read data from a particular address in the main bus
uint8_t emulator_read(Emulator *nes, uint16_t addr);

// Write data to a particular address in the main bus
void emulator_write(Emulator *nes, uint16_t addr, uint8_t data);

// Free all heap memory associated with the emulator
void emulator_free(Emulator *nes);

#endif // LIBRE_NES_EMULATOR_H
