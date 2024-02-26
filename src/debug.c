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

#include "decoder.h"
#include "addressing.h"
#include "definitions.h"
#include "processor.h"

// Text representation table for the 6502 instructions (mnemonics)
static const char *op_text[] = {
    [LDA] = "lda", [LDX] = "ldx", [LDY] = "ldy", [STA] = "sta", [STX] = "stx",
    [STY] = "sty", [TAX] = "tax", [TAY] = "tay", [TXA] = "txa", [TYA] = "tya",
    [TSX] = "tsx", [TXS] = "txs", [PHA] = "pha", [PHP] = "php", [PLA] = "pla",
    [PLP] = "plp", [AND] = "and", [EOR] = "eor", [ORA] = "ora", [BIT] = "bit",
    [ADC] = "adc", [SBC] = "sbc", [CMP] = "cmp", [CPX] = "cpx", [CPY] = "cpy",
    [INC] = "inc", [INX] = "inx", [INY] = "iny", [DEC] = "dec", [DEX] = "dex",
    [DEY] = "dey", [ASL] = "asl", [LSR] = "lsr", [ROL] = "rol", [ROR] = "ror",
    [JMP] = "jmp", [JSR] = "jsr", [RTS] = "rts", [BEQ] = "beq", [BNE] = "bne",
    [BCS] = "bcs", [BCC] = "bcc", [BMI] = "bmi", [BPL] = "bpl", [BVS] = "bvs",
    [BVC] = "bvc", [SEC] = "sec", [SEI] = "sei", [SED] = "sed", [CLC] = "clc",
    [CLI] = "cli", [CLD] = "cld", [CLV] = "clv", [BRK] = "brk", [NOP] = "nop",
    [RTI] = "rti", [ERR] = "<invalid opcode>",
};

// Format string table for addressing modes
static const char *arg_format[] = {
    [MODE_IMMEDIATE]  = " #%d"       ,
    [MODE_ZEROPAGE]   = " $%02X"     ,
    [MODE_ZEROPAGE_X] = " $%02X,X"   ,
    [MODE_ZEROPAGE_Y] = " $%02X,Y"   ,
    [MODE_RELATIVE]   = " $%02X"     ,
    [MODE_ABSOLUTE]   = " $%04X"     ,
    [MODE_ABSOLUTE_X] = " $%04X"     ,
    [MODE_ABSOLUTE_Y] = " $%04X"     ,
    [MODE_INDIRECT]   = " ($%04X)"   ,
    [MODE_INDIRECT_X] = " ($%02X,X)" ,
    [MODE_INDIRECT_Y] = " ($%02X),Y" ,
};

// Read code from the given addressing space (provided via the userdata and
// read parameters) at the given address and with the given length,
// decoding and disassembling it to the given file
void disassemble(FILE *out, void *userdata, AddrReader read,
        uint16_t addr, size_t code_length) {
    size_t i = 0;
    while(i < code_length) {
        uint8_t arg_len = 0;
        uint8_t opcode = read(userdata, addr + i);
        Instruction inst = decode(opcode);
        fprintf(out, "%s", op_text[inst.op]);
        if(inst.mode == MODE_ACCUMULATOR) {
            printf(" A");
        } else if(inst.mode != MODE_IMPLIED) {
            // If not in those modes, there is some real argument to be printed
            arg_len = get_inc(inst.mode);
            uint16_t arg = read(userdata, addr + i + 1);
            if(arg_len == 2) arg |= read(userdata, addr + i + 2) << 8;
            fprintf(out, arg_format[inst.mode], arg);
        }
        fprintf(out, "\n");
        i += arg_len + 1;
    }
}
