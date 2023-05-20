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

#ifndef LIBRE_NES_PROCESSOR_H
#define LIBRE_NES_PROCESSOR_H

#include <stdint.h>
#include "decoder.h"

// Base address of the stack in RAM
#define STACK_BASE 0x0100

// Enumeration representing the CPU flags
typedef enum : uint8_t {
    FLAG_CARRY    = 0,
    FLAG_ZERO     = 1,
    FLAG_ID       = 2,
    FLAG_DEC      = 3,
    FLAG_BRK      = 4,
    FLAG_NIL      = 5,
    FLAG_OVERFLOW = 6,
    FLAG_NEGATIVE = 7,
} Processor_flag;


// Quick and stupid forward declaration
typedef struct nes Emulator;

// Structure representing the CPU's current state
typedef struct {
    Emulator *nes;    // reference to the outside world
    Instruction inst; // richer representation of the current instruction
    uint16_t pc;      // program counter, to control the flow of execution
    uint8_t x, y;     // index registers, to hold counters and offsets
    uint8_t acc;      // accumulator register, for arithmetic and logic
    uint8_t status;   // status register, to store a set of CPU flags
    uint8_t sp;       // stack pointer, to point to the top of the stack in RAM
} Processor;

// Initialize/reset the state of the CPU
void processor_init(Processor *proc, Emulator *nes);

// Run a single step of execution, reading code from memory
void processor_step(Processor *proc);

#endif // LIBRE_NES_PROCESSOR_H
