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
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdbool.h>

#include "cartridge.h"
#include "reader.h"

// Create a new, empty cartridge, with no associated memory
void cartridge_new(Cartridge *cart) {
    cart->prg = cart->chr = NULL;
    cart->prg_size = cart->chr_size = 0;
    cart->prg_banks = cart->chr_banks = 0;
}

// Update cartridge size information, resizing the PRG and CHR roms
void cartridge_resize(Cartridge *cart, uint8_t prg_banks, uint8_t chr_banks) {
    // Resize PRG ROM
    cart->prg_banks = prg_banks;
    cart->prg_size = prg_banks * PRG_BANK_SIZE;
    cart->prg = (uint8_t*) realloc(cart->prg, cart->prg_size * sizeof(uint8_t));
    // Resize CHR ROM
    cart->chr_banks = chr_banks;
    cart->chr_size = chr_banks * CHR_BANK_SIZE;
    cart->chr = (uint8_t*) realloc(cart->prg, cart->chr_size * sizeof(uint8_t));
    // Error handling
    if(cart->prg == NULL || cart->chr == NULL) {
        fprintf(stderr, "[!] Could not allocate sufficient memory for the cartridge\n");
        exit(60);
    }
}

// Set the appropriate mapper for the cartridge
void cartridge_set_mapper(Cartridge *cart, uint8_t mapper_id) {
    cart->mapper_id = mapper_id; // trust me, this will become more complex
}

// Initialize a new cartridge, loading its contents from an external iNES file
void cartridge_init(Cartridge *cart, const char *rom_filepath) {
    cartridge_new(cart);
    ines_read(rom_filepath, cart);
}

// Read data from the cartridge.
uint8_t cartridge_read(const Cartridge *cart, uint16_t addr) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    if(addr >= 0x0000 && addr <= 0x3FFF) {
        return cart->prg[addr];
    } else if(addr >= 0x4000 && addr <= 0x7FFF) {
        return (cart->prg_banks == 1) ? cart->prg[addr & 0x3FFF]
            : cart->prg[addr];
    }
    return 0;
}

// Write data to the cartridge
void cartridge_write(Cartridge *cart, uint16_t addr, uint8_t data) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    return; // all memory is ROM in NROM #0
}

// Read data from the cartridge (PPU)
uint8_t cartridge_ppu_read(const Cartridge *cart, uint16_t addr) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    return cart->chr[addr];
}

// Write data to the cartridge (PPU)
void cartridge_ppu_write(Cartridge *cart, uint16_t addr, uint8_t data) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    return; // all memory is ROM in NROM #0
}

// Free the heap memory associated with the given cartridge
void cartridge_free(Cartridge *cart) {
    free(cart->prg);
    free(cart->chr);
}
