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
#include "reader.h"

// Initialize a new cartridge, loading its contents from an external iNES file
void cartridge_init(Cartrige *cart, const char *rom_filepath) {
    cart->prg = cart->chr = NULL;
    ines_read(rom_filepath, cart);
}

// Read data from the cartridge
uint8_t cartridge_read(const Cartrige *cart, uint16_t addr) {
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
void cartridge_write(Cartrige *cart, uint16_t addr, uint8_t data) {
    assert(cart->mapper_id == 0); // NROM #0 hardcoded
    return; // all of the memory is ROM in NROM #0
}

// Free the heap memory associated with the given cartridge
void cartridge_free(Cartrige *cart) {
    free(cart->prg);
    free(cart->chr);
}
