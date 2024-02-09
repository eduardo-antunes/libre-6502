/*
   Copyright 2024 Eduardo Antunes S. Vieira <eduardoantunes986@gmail.com>

   This file is part of libre-6502.

   libre-6502 is free software: you can redistribute it and/or modify it under
   the terms of the GNU General Public License as published by the Free Software
   Foundation, either version 3 of the License, or (at your option) any later
   version.

   libre-6502 is distributed in the hope that it will be useful, but WITHOUT ANY
   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
   FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.

   You should have received a copy of the GNU General Public License along with
   libre-6502. If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdio.h>
#include <stdint.h>

#include "computer.h"
#include "processor.h"

// Initialize the state of the computer
void computer_init(Computer *c) {
    processor_connect(&c->proc, c);
}

// Start the computer, running the CPU at a consistent frequency
void computer_start(Computer *c) {
    char op;
    int quit = 0, i = 0;
    processor_reset(&c->proc);
    while(!quit) {
        processor_step_debug(&c->proc);
        printf("Clock cycle #%d done. Continue? [y/n] ", i);
        do op = getchar(); while(op != 'y' && op != 'n');
        if(op == 'n') quit = 1;
        ++i;
    }
}

// Read data from a particular address in the main bus
uint8_t address_read(Computer *c, uint16_t addr) {
    // For now, the main memory is just going to be mirrored throughout
    // the whole address space. In the future, this will be configurable
    return c->ram[addr & 0x07FF];
}

// Write data to a particular address in the main bus
void address_write(Computer *c, uint16_t addr, uint8_t data) {
    // For now, the main memory is just going to be mirrored throughout
    // the whole address space. In the future, this will be configurable
    c->ram[addr & 0x07FF] = data;
}
