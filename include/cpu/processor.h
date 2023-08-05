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

#ifndef LIBRE_NES_CPU_PROCESSOR_H
#define LIBRE_NES_CPU_PROCESSOR_H

// Software reproduction of the 6502-like processor used by the original NES
// console. It differs from the standard 6502 primarily in its lack of support
// for decimal mode, as far as I understand.

#include <stdint.h>
#include <stdbool.h>
#include "cpu/definitions.h"

// Enumeration representing the CPU flags
typedef enum : uint8_t {
    FLAG_CARRY    = (1 << 0), // indicates carry in adition, borrow in subtraction
    FLAG_ZERO     = (1 << 1), // indicates the last value dealt with was 0
    FLAG_ID       = (1 << 2), // determines whether maskable interrupts (IRQ) are disabled
    FLAG_DEC      = (1 << 3), // determines if the CPU is in decimal mode, no effect in the NES
    FLAG_BRK      = (1 << 4), // for internal usage by the hardware only
    FLAG_NIL      = (1 << 5), // unused
    FLAG_OVERFLOW = (1 << 6), // indicates an overflow happened in the last arithmetic operation
    FLAG_NEGATIVE = (1 << 7), // indicates the last value dealt with was negative
} Processor_flag;

// Enumeration representing types of interrupts
typedef enum : uint8_t { INT_IRQ, INT_NMI } Processor_int;

typedef struct nes Emulator; // forward declaration

// Structure representing the CPU's current state
typedef struct {
    Emulator *nes;    // reference to the outside world
    uint16_t pc;      // program counter, to control the flow of execution
    uint8_t x, y;     // index registers, to hold counters and offsets
    uint8_t acc;      // accumulator register, for arithmetic and logic
    uint8_t status;   // status register, to store a set of CPU flags
    uint8_t sp;       // stack pointer, to point to the top of the stack in RAM
    Instruction inst; // richer representation of the current instruction
} Processor;

// Initialize/reset the state of the CPU
void processor_reset(Processor *proc);

// Connect the processor to the rest of the console
void processor_connect(Processor *proc, Emulator *nes);

// Generate a CPU interruption (IRQ or NMI)
void processor_interrupt(Processor *proc, Processor_int type);

// Run a single clock cycle of execution
void processor_step(Processor *proc);

#endif // LIBRE_NES_CPU_PROCESSOR_H
