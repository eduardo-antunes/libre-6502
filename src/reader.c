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

#include "reader.h"
#include "cartridge.h"

// Read an external iNES file, loading its contents into a cartridge
void ines_read(const char *filepath, Cartridge *cart) {
    // Open file for reading, binary mode
    FILE *rom = fopen(filepath, "rb");
    if(rom == NULL) {
        fprintf(stderr, "[!] Could not open %s\n", filepath);
        exit(1);
    }

    // Every valid iNES file starts with the string NES\x1A
    char actual_start[4];
    const char start[4] = {'N','E','S','\x1A'};
    fread(actual_start, sizeof(char), 4, rom);
    if(memcmp(actual_start, start, 4) != 0) {
        fprintf(stderr, "[!] %s is an invalid iNES file\n", filepath);
        exit(2);
    }

    // Read the PRG and CHR rom sizes and resize the cartridge
    uint8_t prg_banks, chr_banks;
    fread(&prg_banks, sizeof(uint8_t), 1, rom);
    fread(&chr_banks, sizeof(uint8_t), 1, rom);
    cartridge_resize(cart, prg_banks, chr_banks);

    // Read and process iNES flags (simplified for now)
    uint8_t f0, f1, id;
    fread(&f0, sizeof(uint8_t), 1, rom);
    fread(&f1, sizeof(uint8_t), 1, rom);
    id = (f0 >> 4) | (f1 & 0xF0);
    cartridge_set_mapper(cart, id);

    // Read PRG and CHR roms
    fseek(rom, 16, SEEK_SET); // skip a couple bytes for now
    fread(cart->prg, sizeof(uint8_t), cart->prg_size, rom);
    fread(cart->chr, sizeof(uint8_t), cart->chr_size, rom);

    fclose(rom); // close the file
}
