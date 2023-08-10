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

// Enumeration that represents the order in which the PPU registers may be
// accessed in the main address space
typedef enum : uint8_t {
    PPU_CONTROL     = 0,
    PPU_MASK        = 1,
    PPU_STATUS      = 2,
    PPU_OAM_ADDRESS = 3,
    PPU_OAM_DATA    = 4,
    PPU_SCROLL      = 5,
    PPU_ADDR        = 6,
    PPU_DATA        = 7,
} PPU_reg;

// Connect the PPU to the rest of the console
void ppu_connect(Picture_proc *ppu, Emulator *nes) {
    ppu->nes = nes;
    // We need to connect the cartridge to the picture bus too
    picture_connect(&ppu->bus, &nes->cart);
}

// Read from a particular PPU register by its address
uint8_t ppu_register_read(Picture_proc *ppu, uint16_t addr) {
    // Even though you can read the PPU registers from a wide range of
    // addresses, there are really only 8 of them (for now)
    PPU_reg reg = addr & 7;
    // Only certain registers are considered readable. When you try to read
    // others, this results in the so-called open bus behavior
    switch(reg) {
        case PPU_STATUS:
            // NOTE reading this register mutates it. It should also be noted
            // that the lowest 5 bits of the value returned by reading this
            // register reflect the previous value in the bus (open bus)
            ppu->open_bus = ppu->status.reg & (ppu->open_bus & 0x1F);
            ppu->status.vblank = 0;
            break;
        case PPU_OAM_DATA:
            ppu->open_bus = ppu->oam_data;
            break;
        case PPU_DATA:
            ppu->open_bus = ppu->ppu_data;
            break;
        default:
            // Open bus behavior (just returns whatever was in the bus before)
            break;
    }
    return ppu->open_bus;
}

// Write to a particular PPU register by its address
void ppu_register_write(Picture_proc *ppu, uint16_t addr, uint8_t data) {
    // Even though you can read the PPU registers from a wide range of
    // addresses, there are really only 8 of them (for now)
    PPU_reg reg = addr & 7;
    // You can write to most registers
    ppu->open_bus = data;
    switch(reg) {
        case PPU_CONTROL:
            ppu->control.reg = data;
            break;
        case PPU_MASK:
            ppu->mask.reg = data;
        case PPU_STATUS:
            // Can't write to this one though
            break;
        case PPU_OAM_ADDRESS:
            ppu->oam_addr = data;
        case PPU_OAM_DATA:
            ppu->oam_data = data;
            break;
        case PPU_SCROLL:
            ppu->scroll = data;
            break;
        case PPU_ADDR:
            ppu->ppu_addr = data;
            break;
        case PPU_DATA:
            ppu->ppu_data = data;
            break;
    }
}
