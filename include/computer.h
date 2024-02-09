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

#ifndef LIBRE_6502_COMPUTER_H
#define LIBRE_6502_COMPUTER_H

#include <stdint.h>
#include "processor.h"

// Structure representing the simulated computer
typedef struct c {
    Processor proc;    // 6502 CPU
    uint8_t ram[2048]; // 2KiB of main memory
} Computer;

// Initialize the state of the computer
void computer_init(Computer *c);

// Start the computer, running the CPU at a consistent frequency
void computer_start(Computer *c);

// Read data from a particular address in the main bus
uint8_t address_read(Computer *c, uint16_t addr);

// Write data to a particular address in the main bus
void address_write(Computer *c, uint16_t addr, uint8_t data);

#endif // LIBRE_6502_COMPUTER_H
