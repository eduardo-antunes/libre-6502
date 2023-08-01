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

#ifndef LIBRE_NES_PICTURE_REGISTERS_H
#define LIBRE_NES_PICTURE_REGISTERS_H

// The PPU has a set of registers, refered to here as "picture registers", that
// serve as the primary interface between the CPU and itself. Reading and
// writing to them can be done via the main bus to which the PPU is connected
// and affects its operation in various way. This module is only concerned with
// defining what registers are available and how the interaction with them from
// the main bus is done. Given that this interaction is quite complex, it makes
// sense to separate with from the core of the PPU emulation.

#include <stdint.h>

// Enumeration representing the PPU register IDs. The address the CPU writes to
// in the main bus can be trivially converted to these IDs via masking, which
// makes them useful in the CPU-PPU interface
typedef enum : uint8_t {
    PPU_CONTROL     = 0,
    PPU_MASK        = 1,
    PPU_STATUS      = 2,
    PPU_OAM_ADDRESS = 3,
    PPU_OAM_DATA    = 4,
    PPU_SCROLL      = 5,
    PPU_ADDR        = 6,
    PPU_DATA        = 7,
} Picture_reg_id;

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
} Picture_control;

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
} Picture_mask;

// Bitfield representing the PPU status register
typedef union {
    struct {
        uint8_t vblank      : 1;
        uint8_t sprite_hit  : 1;
        uint8_t sprite_over : 1;
        uint8_t open_bus    : 5;
    };
    uint8_t reg;
} Picture_status;

// Structure representing the set of PPU registers ("picture registers")
typedef struct {
    Picture_control control; // control flags
    Picture_mask mask;       // color effects and rendering
    Picture_status status;   // PPU status information
    uint8_t oam_addr;        // rw address for OAM
    uint8_t oam_data;        // rw data for OAM
    uint8_t scroll;          // current PPU scroll position
    uint8_t ppu_addr;        // rw address for the PPU
    uint8_t ppu_data;        // rw data for the PPU
    uint8_t open_bus;        // open bus behavior
} Picture_regs;

// Read a particular PPU register by its id, with open bus behavior
uint8_t picture_reg_read(Picture_regs *regs, Picture_reg_id id);

// Write to a particular PPU register by its id, with open bus behavior
void picture_reg_write(Picture_regs *regs, Picture_reg_id id, uint8_t data);

#endif // LIBRE_NES_PICTURE_REGISTERS_H
