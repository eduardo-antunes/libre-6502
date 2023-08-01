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
#include "emulator.h"
#include "picture/bus.h"
#include "picture/ppu.h"
#include "picture/registers.h"

// Connect the PPU to the rest of the console
void ppu_connect(Picture_proc *ppu, Emulator *nes) {
    ppu->nes = nes;
    // We need to connect the cartridge to the picture bus too
    picture_connect(&ppu->bus, &nes->cart);
}

// Read from a particular PPU register by its id
uint8_t ppu_register_read(Picture_proc *ppu, Picture_reg_id reg) {
    return picture_reg_read(&ppu->reg, reg);
}

// Write to a particular PPU register by its id
void ppu_register_write(Picture_proc *ppu, Picture_reg_id reg, uint8_t data) {
    picture_reg_write(&ppu->reg, reg, data);
}
