#include <stdint.h>

#include "cartridge.h"
#include "picture/bus.h"

// Connect the game cartridge to the picture bus
void picture_connect(Picture_bus *bus, Cartridge *cart) {
    bus->cart = cart;
}

// Read data from a particular address in the picture bus
uint8_t picture_read(Picture_bus *bus, uint16_t addr) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // Pattern tables, stored in the game cartridge
        return cartridge_ppu_read(bus->cart, addr);
    else if(addr >= 0x2000 && addr <= 0x3EFF)
        // The 2KiB of video ram are mirrored throughout this range
        return bus->vram[addr & 0x0FFF];
    else if(addr >= 0x3F00 && addr <= 0x3FFF)
        // Pallete indexes, which are, in theory, part of the PPU
        return bus->pallete[addr & 31];
    return 0;
}

// Write data to a particular address in the picture bus
void picture_write(Picture_bus *bus, uint16_t addr, uint8_t data) {
    if(addr >= 0x0000 && addr <= 0x1FFF)
        // Pattern tables, stored in the game cartridge
        cartridge_ppu_write(bus->cart, addr, data);
    else if(addr >= 0x2000 && addr <= 0x3EFF)
        // The 2KiB of video ram are mirrored throughout this range
        bus->vram[addr & 0x0FFF] = data;
    else if(addr >= 0x3F00 && addr <= 0x3FFF)
        // Pallete indexes, which are, in theory, part of the PPU
        bus->pallete[addr & 31] = data;
}
