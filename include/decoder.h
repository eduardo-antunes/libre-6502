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

#ifndef LIBRE_NES_DECODER_H
#define LIBRE_NES_DECODER_H

// Decoding logic for the 6502-like processor. It is best to decouple it from the
// actual flow of execution of the CPU, as it is also useful for other stuff.

#include <stdint.h>

// Enumeration representing the CPU addressing modes
typedef enum : uint8_t {
    MODE_IMPLIED     ,
    MODE_ACCUMULATOR ,
    MODE_IMMEDIATE   ,
    MODE_ZEROPAGE    ,
    MODE_ZEROPAGE_X  ,
    MODE_ZEROPAGE_Y  ,
    MODE_RELATIVE    ,
    MODE_ABSOLUTE    ,
    MODE_ABSOLUTE_X  ,
    MODE_ABSOLUTE_Y  ,
    MODE_INDIRECT    ,
    MODE_INDIRECT_X  ,
    MODE_INDIRECT_Y  ,
} Addressing;

// Enumeration representing the available CPU operations
typedef enum : uint8_t {
    // Load and store operations
    LDA, LDX, LDY, STA, STX, STY,
    // Register transfer operations
    TAX, TAY, TXA, TYA, TSX, TXS,
    // Stack operations
    PHA, PHP, PLA, PLP,
    // Logic operations
    AND, EOR, ORA, BIT,
    // Arithmetic operations
    ADC, SBC, CMP, CPX, CPY,
    // Increment operations
    INC, INX, INY,
    // Decrement operations
    DEC, DEX, DEY,
    // Shift operations
    ASL, LSR, ROL, ROR,
    // Jump (unconditional) operations
    JMP, JSR, RTS,
    // Branch (conditional) operations
    BEQ, BNE, BCS, BCC, BMI, BPL, BVS, BVC,
    // Flag operations
    SEC, SEI, SED, CLC, CLI, CLD, CLV,
    // System/symbolic operations
    BRK, NOP, RTI, ERR,
} Operation;

// Richer representation of a CPU instruction
typedef struct {
    Operation op;
    Addressing mode;
} Instruction;

// Decode an 8-bit opcode, translating it into its correspoding CPU operation
// and addressing mode. Key component of execution
Instruction decode(uint8_t opcode);

#endif // LIBRE_NES_DECODER_H
