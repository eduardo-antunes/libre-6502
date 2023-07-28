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

#include <stdint.h>
#include <string.h>

#include "cartridge.h"
#include "ppu.h"

// Connect the game cartridge to the PPU's independent bus
void ppu_connect(PPU *ppu, Cartridge *cart) {
    ppu->cart = cart;
}

// Initialize/reset the state of the PPU
void ppu_reset(PPU *ppu) {
    memset(ppu->vram, 0, 2048);
}

// Read from a particular PPU register
uint8_t ppu_register_read(PPU *ppu, uint8_t index);

// Write to a particular PPU register
void ppu_register_write(PPU *ppu, uint8_t index, uint8_t reg);
