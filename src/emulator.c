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

#include "emulator.h"
#include "processor.h"
#include "reader.h"
#include "cart.h"

// Initialize the state of the emulator
void emulator_init(Emulator *nes, const char *rom_filepath) {
    cartridge_init(&nes->cart, rom_filepath);
    processor_init(&nes->proc, nes);
    memset(nes->ram, 0, 2048);
}

// Start the emulator's operation
void emulator_start(Emulator *nes) {
    int quit = 0, i = 0;
    printf("Initial state of the processor:\n");
    processor_display_info(&nes->proc);
    while(!quit) {
        processor_clock(&nes->proc);
        printf("\nClock cycle #%d done! Processor state:\n", i);
        processor_display_info(&nes->proc);
        printf("\nContinue? [y/n] ");
        char ans = getchar();
        getchar(); // to remove newline
        if(ans == 'n') quit = 1;
        ++i;
    }
    printf("Happy debugging!\n");
}

// Read data from a particular address in memory
uint8_t emulator_read(const Emulator *nes, uint16_t addr) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        return nes->ram[addr & 0x07FF];
    else if(addr >= 0x8000 && addr <= 0xFFFF)
        // This range provides access to the contents of the cartridge
        return cartridge_read(&nes->cart, addr - 0x8000);
    return 0;
}

// Write data to a particular address in memory
void emulator_write(Emulator *nes, uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // The main memory is mirrored throught this range
        nes->ram[addr & 0x07FF] = data;
}

// Free all heap memory associated with the emulator
void emulator_free(Emulator *nes) {
    cartridge_free(&nes->cart);
}
