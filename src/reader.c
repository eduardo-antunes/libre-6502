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

// Offsets for reading into the iNES file
#define SIZE_OFFSET     4
#define FLAGS_OFFSET    6
#define PRG_CODE_OFFSET 16

// Read an external iNES file, loading its contents into a cartridge
void ines_read(const char *filepath, Cartrige *cart) {
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

    // Read the PRG and CHR sizes, given in banks
    fseek(rom, SIZE_OFFSET, SEEK_SET);
    fread(&cart->prg_banks, sizeof(uint8_t), 1, rom);
    fread(&cart->chr_banks, sizeof(uint8_t), 1, rom);
    cart->prg_size = cart->prg_banks * PRG_BANK_SIZE;
    cart->chr_size = cart->chr_banks * CHR_BANK_SIZE;

    // Read the mapper ID
    uint8_t f0, f1;
    fseek(rom, FLAGS_OFFSET, SEEK_SET);
    fread(&f0, sizeof(uint8_t), 1, rom);
    fread(&f1, sizeof(uint8_t), 1, rom);
    cart->mapper_id = f0 >> 4;
    cart->mapper_id |= f1 & 0xF0;

    // Read the code for the program
    fseek(rom, PRG_CODE_OFFSET, SEEK_SET);
    cart->prg = (uint8_t*) malloc(cart->prg_size * sizeof(uint8_t));
    fread(cart->prg, sizeof(uint8_t), cart->prg_size, rom);

    fclose(rom); // close the file
}
