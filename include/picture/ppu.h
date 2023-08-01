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

#ifndef LIBRE_NES_PICTURE_PPU_H
#define LIBRE_NES_PICTURE_PPU_H

// NES graphics hardware emulation

#include <stdint.h>
#include "cartridge.h"
#include "picture/bus.h"
#include "picture/registers.h"

typedef struct nes Emulator; // forward declaration

// Structure representing the NES PPU (picture processing unit). It holds its
// own, independent picture bus, as well as a set of registers that configure
// and expose some of its internal flags
typedef struct {
    Emulator *nes;    // reference to the outside world
    Picture_bus bus;  // the independent PPU bus ("picture bus")
    Picture_regs reg; // set of PPU registers ("picture registers")
} Picture_proc;

// Connect the PPU to the rest of the console
void ppu_connect(Picture_proc *ppu, Emulator *nes);

// Read from a particular PPU register by its id
uint8_t ppu_register_read(Picture_proc *ppu, Picture_reg_id reg);

// Write to a particular PPU register by its id
void ppu_register_write(Picture_proc *ppu, Picture_reg_id reg, uint8_t data);

// Run a single clock cycle of operations on the PPU
void ppu_step(Picture_proc *ppu);

#endif // LIBRE_NES_PICTURE_PPU_H
