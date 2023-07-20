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

#include "cart.h"

#define SIZE_OFFSET 4
#define FLAGS_OFFSET 6
#define PRG_CODE_OFFSET 16

// Initialize a new empty cartridge
void cart_init(Cartrige *cart) {
    cart->prg = cart->chr = NULL;
    cart->prg_size = cart->chr_size = 0;
    cart->prg_banks = cart->chr_banks = 0;
    cart->mapper_id = 0;
}

// Load cartridge contents from an external iNES file
int cart_load(Cartrige *cart, const char *filepath) {
    // Open file for binary reading
    FILE *rom = fopen(filepath, "rb");
    if(rom == NULL) {
        fprintf(stderr, "[!] Could not open %s\n", filepath);
        return 1;
    }

    // Every iNES file must start with the hardcoded string "NES\r\n"
    const char magic[5] = "NES\x1A"; char buf[5] = {0};
    fread(buf, sizeof(char), 4, rom);
    if(strcmp(magic, buf) != 0) {
        fprintf(stderr, "[!] Invalid iNES file: %s\n", filepath);
        return 2;
    }

    // Read the PRG and CHR sizes, given in units of banks
    fseek(rom, SIZE_OFFSET, SEEK_SET);
    fread(&cart->prg_banks, sizeof(uint8_t), 1, rom);
    fread(&cart->chr_banks, sizeof(uint8_t), 1, rom);
    cart->prg_size = cart->prg_banks * PRG_BANK_SIZE;
    cart->chr_size = cart->chr_banks * CHR_BANK_SIZE;

    // Read the mapper ID (only NROM, #0, is supported now)
    uint8_t f6, f7;
    fseek(rom, FLAGS_OFFSET, SEEK_SET);
    fread(&f6, sizeof(uint8_t), 1, rom);
    fread(&f7, sizeof(uint8_t), 1, rom);
    cart->mapper_id = f6 >> 4;
    cart->mapper_id |= f7 & 0xF0;
    assert(cart->mapper_id == 0); // TEMP

    // Read the program code, allocating memory for it
    fseek(rom, PRG_CODE_OFFSET, SEEK_SET);
    cart->prg = (uint8_t*) malloc(cart->prg_size * sizeof(uint8_t));
    fread(cart->prg, sizeof(uint8_t), cart->prg_size, rom);

    // Finish things off
    fclose(rom);
    return 0;
}

// Read data from the cartridge
uint8_t cart_read(const Cartrige *cart, uint16_t addr) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    if(addr >= 0x8000 && addr <= 0xBFFF) {
        return cart->prg[addr - 0x8000];
    } else if(addr >= 0xC000 && addr <= 0xFFFF) {
        return (cart->prg_banks == 1) ? cart->prg[addr - 0xC000]
            : cart->prg[addr - 0x8000];
    }
    return 0;
}

// Write data to the cartridge
void cart_write(Cartrige *cart, uint16_t addr, uint8_t data) {
    return; // all of the memory is ROM
}

// Free an existing cartridge and initialize to be empty again
void cart_free(Cartrige *cart) {
    if(cart->prg != NULL) free(cart->prg);
    if(cart->chr != NULL) free(cart->chr);
    cart_init(cart);
}
