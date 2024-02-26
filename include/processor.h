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

#ifndef LIBRE_6502_PROCESSOR_H
#define LIBRE_6502_PROCESSOR_H

// Address for the so-called reset vectors. They should themselves point to
// other address to which the execution should jump to in case of a non-
// maskable interrupt, an interrupt request or a reset, respectively
#define NMI_VECTOR   0xFFFA
#define IRQ_VECTOR   0xFFFE
#define RESET_VECTOR 0xFFFC

// The core 6502 implementation. It can be connected to any arbitrary address
// space via two user-defined functions which read and write to it. In addition,
// a userdata pointer can be provided to keep track of the state of the space.
// This pointer is taken by the read and write functions. I think this will
// probably always be needed.

#include <stdint.h>
#include <stdbool.h>
#include "definitions.h"

// Signatures for address readers and writers
typedef uint8_t (*AddrReader)(void *userdata, uint16_t address);
typedef void    (*AddrWriter)(void *userdata, uint16_t address, uint8_t data);

// The various CPU flags, stored in the status register. Their purpose is
// twofold: first, they communicate to the running program information on the
// instruction previously executed. Second, they can be set to affect future
// instructions and hardware behavior.
typedef enum : uint8_t {
    FLAG_CARRY    = (1 << 0), // indicates carry in adition, borrow in subtraction
    FLAG_ZERO     = (1 << 1), // indicates that the last value dealt with was 0
    FLAG_IRQ_DIS  = (1 << 2), // determines whether maskable interrupts (IRQ) are disabled
    FLAG_DECIMAL  = (1 << 3), // determines if the CPU is in decimal mode (BCD)
    FLAG_BREAK    = (1 << 4), // for internal usage by the hardware only
    FLAG_NIL      = (1 << 5), // unused
    FLAG_OVERFLOW = (1 << 6), // indicates an overflow happened in the last arithmetic operation
    FLAG_NEGATIVE = (1 << 7), // indicates the last value dealt with was negative
} Processor_flag;

// Structure representing the CPU's state and metadata
typedef struct {
    // Hardware registers
    uint16_t pc;      // program counter, to control the flow of execution
    uint8_t x, y;     // index registers, to hold counters and offsets
    uint8_t acc;      // accumulator register, for arithmetic and logic
    uint8_t status;   // status register, to store the set of CPU flags
    uint8_t sp;       // stack pointer, to point to the top of the stack in RAM

    // Metadata used by the library
    void *u;          // custom userdata, passed to read and write functions
    AddrReader read;  // read from addresses (user-provided)
    AddrWriter write; // write to addresses (user-provided)
    Instruction inst; // representation of the current instruction
} Processor;

// Initializes a new processor instance, connecting it to its address space
void processor_init(Processor *proc, AddrReader read,
        AddrWriter write, void *userdata);

// Reset the CPU, reinitializing its state
void processor_reset(Processor *proc);

// Request a CPU interruption (IRQ)
void processor_request(Processor *proc);

// Generate a non-maskable CPU interruption (NMI)
void processor_interrupt(Processor *proc);

// Run a single clock cycle of execution
void processor_step(Processor *proc);

#endif // LIBRE_6502_PROCESSOR_H
