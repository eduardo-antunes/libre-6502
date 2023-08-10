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

typedef struct nes Emulator; // forward declaration

// Bitfield representing the PPU control register
typedef union {
    struct {
        uint8_t nmi_enable    : 1;
        uint8_t master_slave  : 1;
        uint8_t sprite_height : 1;
        uint8_t bg_select     : 1;
        uint8_t inc_mode      : 1;
        uint8_t tbl_select    : 2;
    };
    uint8_t reg;
} PPU_control;

// Bitfield representing the PPU mask register
typedef union {
    struct {
        uint8_t blue           : 1;
        uint8_t green          : 1;
        uint8_t red            : 1;
        uint8_t sprite_enable  : 1;
        uint8_t bg_enable      : 1;
        uint8_t lsprite_enable : 1;
        uint8_t lbg_enable     : 1;
        uint8_t greyscale      : 1;
    };
    uint8_t reg;
} PPU_mask;

// Bitfield representing the PPU status register
typedef union {
    struct {
        uint8_t vblank      : 1;
        uint8_t sprite_hit  : 1;
        uint8_t sprite_over : 1;
        uint8_t open_bus    : 5;
    };
    uint8_t reg;
} PPU_status;

// Structure representing the NES PPU (picture processing unit). It holds its
// own, independent picture bus, as well as a set of registers that configure
// and expose some of its internal flags
typedef struct {
    Emulator *nes;    // reference to the outside world
    Picture_bus bus;  // the independent PPU bus ("picture bus")

    PPU_control control; // various control flags
    PPU_mask mask;       // color effects and rendering
    PPU_status status;   // status info
    uint8_t oam_addr;    // r/w address for OAM
    uint8_t oam_data; // r/w data for OAM
    uint8_t scroll;   // current scroll position
    uint8_t ppu_addr; // r/w address for the PPU bus
    uint8_t ppu_data; // r/w address for the PPU bus
    uint8_t open_bus;
} Picture_proc;

// Connect the PPU to the rest of the console
void ppu_connect(Picture_proc *ppu, Emulator *nes);

// Read from a particular PPU register by its address
uint8_t ppu_register_read(Picture_proc *ppu, uint16_t addr);

// Write to a particular PPU register by its address
void ppu_register_write(Picture_proc *ppu, uint16_t addr, uint8_t data);

// Run a single clock cycle of operations on the PPU
void ppu_step(Picture_proc *ppu);

#endif // LIBRE_NES_PICTURE_PPU_H
