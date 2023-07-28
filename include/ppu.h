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

#ifndef LIBRE_NES_PPU_H
#define LIBRE_NES_PPU_H

#include <stdint.h>
#include "cartridge.h"

// Bitfield definition used to represent the PPU control register
typedef union {
    struct {
        uint8_t nmi_enable    : 1;
        uint8_t master_slave  : 1;
        uint8_t sprite_height : 1;
        uint8_t bg_select     : 1;
        uint8_t inc_mode      : 1;
        uint8_t tbl_select    : 2;
    };
    uint8_t raw;
} PPU_control;

// Bitfield definition used to represent the PPU mask register
typedef union {
    struct {
        uint8_t blue               : 1;
        uint8_t green              : 1;
        uint8_t red                : 1;
        uint8_t sprite_enable      : 1;
        uint8_t bg_enable          : 1;
        uint8_t left_sprite_enable : 1;
        uint8_t left_bg_enable     : 1;
        uint8_t greyscale          : 1;
    };
    uint8_t raw;
} PPU_mask;

// Bitfield used to represent the PPU status register
typedef union {
    struct {
        uint8_t vblank          : 1;
        uint8_t sprite_hit      : 1;
        uint8_t sprite_overflow : 1;
        uint8_t open_bus        : 5;
    };
    uint8_t raw;
} PPU_status;

// Structure representing the NES PPU (picture processing unit). It also
// represents the independent bus associated with the PPU, with its own address
// space separate from that of the main bus
typedef struct {
    Cartridge *cart;    // the connected game cartridge
    uint8_t vram[2048]; // 2KiB of video RAM, used to store nametables

    // PPU registers, used by the CPU to interact with and configure the PPU's
    // operation. They are accessed through the ppu_register functions, where
    // they are each associated with an index in the range [0;7]
    PPU_control control; // various control flags
    PPU_mask mask;       // configures color effects and the rendering of sprites
    PPU_status status;   // reflects the inner state of the unit
    uint8_t oam_addr;    // read/write address for OAM
    uint8_t oam_data;    // read/write data for OAM
    uint8_t scroll;      // determine the current scroll position
    uint8_t ppu_addr;    // read/write address for VRAM
    uint8_t ppu_data;    // read/write data for VRAM
    uint8_t oam_dma;     // OAM DMA register
} PPU;

// Connect the game cartridge to the PPU's independent bus
void ppu_connect(PPU *ppu, Cartridge *cart);

// Initialize/reset the state of the PPU
void ppu_reset(PPU *ppu);

// Read from a particular PPU register
uint8_t ppu_register_read(PPU *ppu, uint8_t index);

// Write to a particular PPU register
void ppu_register_write(PPU *ppu, uint8_t index, uint8_t reg);

// Read data from the independent PPU bus
uint8_t ppu_read(const PPU *ppu, uint16_t addr);

// Write data to the independent PPU bus
void ppu_write(PPU *ppu, uint16_t addr, uint8_t data);

// Run a single clock cycle of operations on the PPU
void ppu_clock(PPU *ppu);

#endif // LIBRE_NES_PPU_H
