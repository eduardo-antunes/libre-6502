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

// Software reproduction of the 6502-like processor used by the original NES
// console. It differs from the standard 6502 primarily in its lack of support
// for decimal mode, as far as I understand.

#include <stdint.h>
#include "decoder.h"

// Base address of the stack in RAM
#define STACK_BASE 0x0100

// Enumeration representing the CPU flags
typedef enum : uint8_t {
    FLAG_CARRY    = 0, // indicates carry in adition, borrow in subtraction
    FLAG_ZERO     = 1, // indicates the last value dealt with was 0
    FLAG_ID       = 2, // determines whether maskable interrupts are enabled
    FLAG_DEC      = 3, // determines if the CPU is decimal mode, no effect in the NES
    FLAG_BRK      = 4, // for internal usage by the hardware only
    FLAG_NIL      = 5, // unused
    FLAG_OVERFLOW = 6, // indicates an overflow happened in the last arithmetic operation
    FLAG_NEGATIVE = 7, // indicates the last value dealt with was negative
} Processor_flag;


typedef struct nes Emulator; // forward declaration

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

// Run a single step of execution, reading code from the main memory
void processor_step(Processor *proc);

#endif // LIBRE_NES_PROCESSOR_H
