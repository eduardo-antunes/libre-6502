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
#include "disassembler.h"

// Print an operation in readable form, showing the opcode for invalid operations
void print_operation(FILE *fp, Operation op, uint8_t opcode) {
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

// Print the operand for an instruction given its addressing mode; returns the
// amount of bytes following the opcode that correspond to its operand
static uint8_t print_operand(FILE *fp, const Emulator *nes, uint16_t ptr, Addressing mode) {
    uint8_t data;
    uint16_t addr;
    switch(mode) {
        case MODE_IMMEDIATE:
            // Operand is the following byte, it is printed in decimal
            data = emulator_read(nes, ptr);
            fprintf(fp, "#%d", data);
            return 1;
        case MODE_ZEROPAGE:
            // Operand is the following byte, which is a zero-page address
            addr = emulator_read(nes, ptr);
            fprintf(fp, "$%02X", addr);
            return 1;
        case MODE_ZEROPAGE_X:
            // Operand is the following byte, which is a zero-page address
            addr = emulator_read(nes, ptr);
            fprintf(fp, "$%02X,X", addr);
            return 1;
        case MODE_ZEROPAGE_Y:
            // Operand is the following byte, which is a zero-page address
            addr = emulator_read(nes, ptr++);
            fprintf(fp, "$%02X,Y", addr);
            return 1;
        case MODE_RELATIVE:
            // Now these ones are tricky. Ideally, we would like to use labels
            // when disassembling, in order to make the code clearer. That's
            // going to require some preprocessing though. I'll leave it for
            // the future. In the meantime, the raw absolute address will do
            addr = emulator_read(nes, ptr);
            if(addr & 0x80) addr |= 0xFF00; // sign extension
            addr += ptr + addr - 2; // get absolute address
            fprintf(fp, "$%04X", addr);
            return 1;
        case MODE_ABSOLUTE:
            // Operand is the following two bytes, which is an absolute address
            addr = emulator_read(nes, ptr);
            addr |= emulator_read(nes, ptr + 1) << 8;
            fprintf(fp, "$%04X", addr);
            return 2;
        case MODE_ABSOLUTE_X:
            // Operand is the following two bytes, which is an absolute address
            addr = emulator_read(nes, ptr);
            addr |= emulator_read(nes, ptr + 1) << 8;
            fprintf(fp, "$%04X,X", addr);
            return 2;
        case MODE_ABSOLUTE_Y:
            // Operand is the following two bytes, which is an absolute address
            addr = emulator_read(nes, ptr);
            addr |= emulator_read(nes, ptr + 1) << 8;
            fprintf(fp, "$%04X,Y", addr);
            return 2;
        case MODE_INDIRECT:
            // Operand is the following two bytes, which is an absolute address
            addr = emulator_read(nes, ptr);
            addr |= emulator_read(nes, ptr + 1) << 8;
            fprintf(fp, "($%04X)", addr);
            return 2;
        case MODE_INDIRECT_X:
            // Operand is the following byte, which is a zero-page address
            addr = emulator_read(nes, ptr);
            fprintf(fp, "($%02X,X)", addr);
            return 1;
        case MODE_INDIRECT_Y:
            // Operand is the following byte, which is a zero-page address
            addr = emulator_read(nes, ptr);
            fprintf(fp, "($%02X),Y", addr);
            return 1;
        default:
            // Other modes don't have operands to be shown
            return 0;
    }
}

// Print an instruction in a readable form, reading it from main memory
static uint16_t print_instruction(FILE *fp, const Emulator *nes, uint16_t pc) {
    uint8_t opcode = emulator_read(nes, pc++);
    Instruction inst = decode(opcode);
    print_operation(fp, inst.op, opcode);
    if(inst.mode != MODE_IMPLIED && inst.mode != MODE_ACCUMULATOR)
        fprintf(fp, " ");
    pc += print_operand(fp, nes, pc, inst.mode);
    fprintf(fp, "\n");
    return pc;
}

// Disassemble a program stored in main memory
void disassemble(FILE *fp, const Emulator *nes, int nr_instructions) {
    uint16_t pc = processor_init_pc(nes);
    for(int i = 0; i < nr_instructions; ++i)
        pc = print_instruction(fp, nes, pc);
}
