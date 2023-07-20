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

#ifndef LIBRE_NES_CART_H
#define LIBRE_NES_CART_H

#include <stdint.h>

#define PRG_BANK_SIZE 16384
#define CHR_BANK_SIZE 8192

// Structure representing a NES cartridge
typedef struct {
    uint8_t *prg, *chr;
    int prg_size, chr_size;
    uint8_t prg_banks, chr_banks;
    uint8_t mapper_id;
} Cartrige;

// Initialize a new empty cartridge
void cart_init(Cartrige *cart);

// Load cartridge contents from an external iNES file
int cart_load(Cartrige *cart, const char *filepath);

// Read data from the cartridge
uint8_t cart_read(const Cartrige *cart, uint16_t addr);

// Write data to the cartridge
void cart_write(Cartrige *cart, uint16_t addr, uint8_t data);

// Free an existing cartridge and initialize to be empty again
void cart_free(Cartrige *cart);

#endif // LIBRE_NES_CART_H
