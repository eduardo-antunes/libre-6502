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

#include "addressing.h"
#include "definitions.h"
#include "processor.h"

// Text representation table for the 6502 instructions
static const char *op_text[] = {
    // Load and store operations
    [LDA] = "lda", [LDX] = "ldx", [LDY] = "ldy",
    [STA] = "sta", [STX] = "stx", [STY] = "sty",
    // Register transfer operations
    [TAX] = "tax", [TAY] = "tay", [TXA] = "txa",
    [TYA] = "tya", [TSX] = "tsx", [TXS] = "txs",
    // Stack operations
    [PHA] = "pha", [PHP] = "php", [PLA] = "pla",
    [PLP] = "plp",
    // Logic operations
    [AND] = "and", [EOR] = "eor", [ORA] = "ora",
    [BIT] = "bit",
    // Arithmetic operations
    [ADC] = "adc", [SBC] = "sbc", [CMP] = "cmp",
    [CPX] = "cpx", [CPY] = "cpy",
    // Increment operations
    [INC] = "inc", [INX] = "inx", [INY] = "iny",
    // Decrement operations
    [DEC] = "dec", [DEX] = "dex", [DEY] = "dey",
    // Shift operations
    [ASL] = "asl", [LSR] = "lsr", [ROL] = "rol",
    [ROR] = "ror",
    // Jump (unconditional) operations
    [JMP] = "jmp", [JSR] = "jsr", [RTS] = "rts",
    // Branch (conditional) operations
    [BEQ] = "beq", [BNE] = "bne", [BCS] = "bcs",
    [BCC] = "bcc", [BMI] = "bmi", [BPL] = "bpl",
    [BVS] = "bvs", [BVC] = "bvc",
    // Flag operations
    [SEC] = "sec", [SEI] = "sei", [SED] = "sed",
    [CLC] = "clc", [CLI] = "cli", [CLD] = "cld",
    [CLV] = "clv",
    // System operations
    [BRK] = "brk", [NOP] = "nop", [RTI] = "rti",
    [ERR] = "<decoder error>",
};

// Format string table for addressing modes
static const char *arg_format[] = {
    [MODE_IMMEDIATE]  = " #%d",
    [MODE_ZEROPAGE]   = " $%02X",
    [MODE_ZEROPAGE_X] = " $%02X,X",
    [MODE_ZEROPAGE_Y] = " $%02X,Y",
    [MODE_RELATIVE]   = " $%02X",
    [MODE_ABSOLUTE]   = " $%04X",
    [MODE_ABSOLUTE_X] = " $%04X",
    [MODE_ABSOLUTE_Y] = " $%04X",
    [MODE_INDIRECT]   = " ($%04X)",
    [MODE_INDIRECT_X] = " ($%02X,X)",
    [MODE_INDIRECT_Y] = " ($%02X),Y",
};

// Disassemble a single instruction; requires a reference to the processor
// to read the instruction's argument from memory
int disassemble_step(const Processor *proc) {
    uint8_t inc = 0;
    printf("%s", op_text[proc->inst.op]);
    if(proc->inst.mode != MODE_IMPLIED && proc->inst.mode != MODE_ACCUMULATOR) {
        // If not in those modes, there is some argument to be printed
        uint16_t arg = (proc->inst.mode == MODE_IMMEDIATE) ?
            get_data_static(proc, NULL, &inc) : get_address_static(proc, &inc);
        printf(arg_format[proc->inst.mode], arg);
    }
    printf("\n");
    return inc;
}
