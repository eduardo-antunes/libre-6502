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

#ifndef LIBRE_NES_PICTURE_BUS_H
#define LIBRE_NES_PICTURE_BUS_H

#include <stdint.h>
#include "cartridge.h"

// Structure representing the independent PPU bus ("picture bus")
typedef struct {
    Cartridge *cart;     // the connected game cartridge
    uint8_t vram[2048];  // 2KiB of video RAM (for nametables)
    uint8_t pallete[32]; // pallete indexes
} Picture_bus;

// Connect the game cartridge to the picture bus
void picture_connect(Picture_bus *bus, Cartridge *cart);

// Read data from a particular address in the picture bus
uint8_t picture_read(Picture_bus *bus, uint16_t addr);

// Write data to a particular address in the picture bus
void picture_write(Picture_bus *bus, uint16_t addr, uint8_t data);

#endif // LIBRE_NES_PICTURE_BUS_H
