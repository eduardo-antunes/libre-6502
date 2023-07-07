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

#include <stdio.h>
#include <stdint.h>
#include "decoder.h"
#include "emulator.h"
#include "processor.h"

// Display the operation name in a readable manner
void display_operation(FILE *fp, uint16_t opcode, Operation op) {
    switch(op) {
        case NOP: fprintf(fp, "nop"); break;
        case LDA: fprintf(fp, "lda"); break;
        case LDX: fprintf(fp, "ldx"); break;
        case LDY: fprintf(fp, "ldy"); break;
        case STA: fprintf(fp, "sta"); break;
        case STX: fprintf(fp, "stx"); break;
        case STY: fprintf(fp, "sty"); break;
        case TAX: fprintf(fp, "tax"); break;
        case TAY: fprintf(fp, "tay"); break;
        case TXA: fprintf(fp, "txa"); break;
        case TYA: fprintf(fp, "tya"); break;
        case TSX: fprintf(fp, "tsx"); break;
        case TXS: fprintf(fp, "txs"); break;
        case PHA: fprintf(fp, "pha"); break;
        case PHP: fprintf(fp, "php"); break;
        case PLA: fprintf(fp, "pla"); break;
        case PLP: fprintf(fp, "plp"); break;
        case AND: fprintf(fp, "and"); break;
        case EOR: fprintf(fp, "eor"); break;
        case ORA: fprintf(fp, "ora"); break;
        case BIT: fprintf(fp, "bit"); break;
        case ADC: fprintf(fp, "adc"); break;
        case SBC: fprintf(fp, "sbc"); break;
        case CMP: fprintf(fp, "cmp"); break;
        case CPX: fprintf(fp, "cpx"); break;
        case CPY: fprintf(fp, "cpy"); break;
        case INC: fprintf(fp, "inc"); break;
        case INX: fprintf(fp, "inx"); break;
        case INY: fprintf(fp, "iny"); break;
        case DEC: fprintf(fp, "dec"); break;
        case DEX: fprintf(fp, "dex"); break;
        case DEY: fprintf(fp, "dey"); break;
        case ASL: fprintf(fp, "asl"); break;
        case LSR: fprintf(fp, "lsr"); break;
        case ROL: fprintf(fp, "rol"); break;
        case ROR: fprintf(fp, "ror"); break;
        case JMP: fprintf(fp, "jmp"); break;
        case JSR: fprintf(fp, "jsr"); break;
        case RTS: fprintf(fp, "rts"); break;
        case BEQ: fprintf(fp, "beq"); break;
        case BNE: fprintf(fp, "bne"); break;
        case BCS: fprintf(fp, "bcs"); break;
        case BCC: fprintf(fp, "bcc"); break;
        case BMI: fprintf(fp, "bmi"); break;
        case BPL: fprintf(fp, "bpl"); break;
        case BVS: fprintf(fp, "bvs"); break;
        case BVC: fprintf(fp, "bvc"); break;
        case SEC: fprintf(fp, "sec"); break;
        case SEI: fprintf(fp, "sei"); break;
        case SED: fprintf(fp, "sed"); break;
        case CLC: fprintf(fp, "clc"); break;
        case CLI: fprintf(fp, "cli"); break;
        case CLD: fprintf(fp, "cld"); break;
        case CLV: fprintf(fp, "clv"); break;
        default: fprintf(fp, "<invalid opcode %02X>", opcode);
    }
}

// Display the instruction's argument based on the addressing mode, returning
// the new appropriate value for the PC
static uint16_t display_arg(FILE *fp, const Emulator *nes, uint16_t pc, Addressing mode) {
    uint8_t data;
    uint16_t addr;
    switch(mode) {
        case MODE_IMPLIED:
            // No argument to be displayed
            break;
        case MODE_ACCUMULATOR:
            // No argument to be displayed
            break;
        case MODE_IMMEDIATE:
            // Argument is the following byte
            data = emulator_read(nes, pc++);
            fprintf(fp, "#%d", data);
            break;
        case MODE_ZEROPAGE:
            // Argument is the following byte
            addr = emulator_read(nes, pc++);
            fprintf(fp, "$%02X", addr);
            break;
        case MODE_ZEROPAGE_X:
            // Argument is the following byte
            addr = emulator_read(nes, pc++);
            fprintf(fp, "$%02X,X", addr);
            break;
        case MODE_ZEROPAGE_Y:
            // Argument is the following byte
            addr = emulator_read(nes, pc++);
            fprintf(fp, "$%02X,Y", addr);
            break;
        case MODE_RELATIVE:
            // Now these ones are tricky. Ideally, we would like to use labels
            // when disassembling, in order to make the code clearer. That's
            // going to require some preprocessing though. I'll leave it for
            // the future. In the meantime, the raw absolute address will do
            addr = emulator_read(nes, pc++);
            if(addr & 0x80) addr |= 0xFF00; // sign extension
            addr += pc - 2; // get absolute address
            fprintf(fp, "$%04X", addr);
            break;
        case MODE_ABSOLUTE:
            // Argument is the following two bytes
            addr = emulator_read(nes, pc++);
            addr |= emulator_read(nes, pc++) << 8;
            fprintf(fp, "$%04X", addr);
            break;
        case MODE_ABSOLUTE_X:
            // Argument is the following two bytes
            addr = emulator_read(nes, pc++);
            addr |= emulator_read(nes, pc++) << 8;
            fprintf(fp, "$%04X,X", addr);
            break;
        case MODE_ABSOLUTE_Y:
            // Argument is the following two bytes
            addr = emulator_read(nes, pc++);
            addr |= emulator_read(nes, pc++) << 8;
            fprintf(fp, "$%04X,Y", addr);
            break;
        case MODE_INDIRECT:
            // Argument is the following two bytes
            addr = emulator_read(nes, pc++);
            addr |= emulator_read(nes, pc++) << 8;
            fprintf(fp, "($%04X)", addr);
            break;
        case MODE_INDIRECT_X:
            // Argument is the following byte
            addr = emulator_read(nes, pc++);
            fprintf(fp, "($%02X,X)", addr);
            break;
        case MODE_INDIRECT_Y:
            // Argument is the following byte
            addr = emulator_read(nes, pc++);
            fprintf(fp, "($%02X),Y", addr);
            break;
    }
    return pc;
}

// Disassemble a program stored in main memory
void disassemble(FILE *fp, const Emulator *nes, int nr_instructions) {
    // Reproduction of the PC for disassembling
    uint16_t pc = processor_init_pc(nes);
    for(int i = 0; i < nr_instructions; ++i) {
        uint8_t opcode = emulator_read(nes, pc++);
        Instruction inst = decode(opcode);
        display_operation(fp, opcode, inst.op);
        if(inst.mode != MODE_IMPLIED && inst.mode != MODE_ACCUMULATOR)
            fprintf(fp, " ");
        pc = display_arg(fp, nes, pc, inst.mode);
        fprintf(fp, "\n");
    }
}
