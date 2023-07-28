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

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "cpu/processor.h"
#include "cartridge.h"
#include "emulator.h"
#include "reader.h"
#include "ppu.h"

// Initialize the state of the emulator, loading an iNES rom file
void emulator_init(Emulator *nes, const char *rom_filepath) {
    cartridge_init(&nes->cart, rom_filepath);
    processor_connect(&nes->proc, nes);
    ppu_connect(&nes->ppu, &nes->cart);
    memset(nes->ram, 0, 2048);
}

// Start the emulator's operation
void emulator_start(Emulator *nes) {
    char op;
    int quit = 0, i = 0;
    processor_reset(&nes->proc);
    while(!quit) {
        processor_clock(&nes->proc);
        printf("Clock cycle #%d done. Continue? [y/n] ", i);
        while(op != 'y' && op != 'n') op = getchar();
        if(op == 'n') quit = 1;
        ++i;
    }
}

// Read data from a particular address in the main bus
uint8_t emulator_read(Emulator *nes, uint16_t addr) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        return nes->ram[addr & 0x07FF];
    else if(addr >= 0x2000 && addr <= 0x3FFF)
        // This range provides access to the PPU register, which provide a way
        // for the CPU to interact with and configure the PPU's operation
        return ppu_register_read(&nes->ppu, addr & 7);
    else if(addr >= 0x8000 && addr <= 0xFFFF)
        // This 32KiB range provides access to the contents of the cartridge
        return cartridge_read(&nes->cart, addr - 0x4020);
    return 0;
}

// Write data to a particular address in the main bus
void emulator_write(Emulator *nes, uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        nes->ram[addr & 0x07FF] = data;
    else if(addr >= 0x2000 && addr <= 0x3FFF)
        // This range provides access to the PPU register, which provide a way
        // for the CPU to interact with and configure the PPU's operation
        ppu_register_write(&nes->ppu, addr & 7, data);
    else if(addr >= 0x8000 && addr <= 0xFFFF)
        // This 32KiB range provides access to the contents of the cartridge
        cartridge_write(&nes->cart, addr - 0x8000, data);
}

// Free all heap memory associated with the emulator
void emulator_free(Emulator *nes) {
    cartridge_free(&nes->cart);
}
