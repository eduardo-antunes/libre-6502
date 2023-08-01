#include <stdint.h>
#include "picture/registers.h"

// Read a particular PPU register by its index, with open bus behavior
uint8_t picture_reg_read(Picture_regs *regs, Picture_reg_id id) {
    // Only certain registers are considered readable. They are the only ones
    // that trigger an update to the bus's contents when read. The others
    // simply respond with the current contents of the bus: this is the so
    // called "open bus" behavior. We use the open_bus register to implement it
    switch(id) {
        case PPU_STATUS:
            // NOTE reading the PPU status register mutates it :O
            regs->status.open_bus = regs->open_bus & 0x1F;
            regs->open_bus = regs->status.reg;
            regs->status.vblank = 0;
            break;
        case PPU_OAM_DATA:
            regs->open_bus = regs->oam_data;
            break;
        case PPU_DATA:
            regs->open_bus = regs->ppu_data;
            break;
        default:
            // No updates to the PPU bus's contents, open_bus is untouched
            break;
    }
    return regs->open_bus;
}

// Write to a particular PPU register by its index, with open bus behavior
void picture_reg_write(Picture_regs *regs, Picture_reg_id id, uint8_t data) {
    regs->open_bus = data;
    switch(id) {
        case PPU_CONTROL:
            regs->control.reg = data;
            break;
        case PPU_MASK:
            regs->mask.reg = data;
            break;
        case PPU_STATUS:
            // The PPU status register is read-only. Attempting to write to it
            // does not modify its value, but it does overwrite the current
            // contents of the bus. This is reflected in the open_bus register
            break;
        case PPU_OAM_ADDRESS:
            regs->oam_addr = data;
            break;
        case PPU_OAM_DATA:
            regs->oam_data = data;
            break;
        case PPU_SCROLL:
            regs->scroll = data;
            break;
        case PPU_ADDR:
            regs->ppu_addr = data;
            break;
        case PPU_DATA:
            regs->ppu_data = data;
            break;
    }
}
