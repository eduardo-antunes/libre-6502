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

#include <stdint.h>
#include <stdbool.h>

#include "definitions.h"
#include "decoder.h"

// The decoding logic found here was heavily guided by the following resource:
// https://llx.com/Neil/a2/opcodes.html; I sincerely thank the author for
// sparing me of writing a huge opcode lookup table.

// Syntax sugar for decoder errors; I think it's pretty readable
#define error_if(cond) if(cond) return (Instruction){.op = ERR};

// Set of lookup tables for operations; it is indexed by group (-1) and its
// inner tables are index by the 'op' bits of each opcode
static Operation op_table[][8] = {
    [0] = { [0] = ORA, [1] = AND, [2] = EOR, [3] = ADC,
            [4] = STA, [5] = LDA, [6] = CMP, [7] = SBC, },
    [1] = { [0] = ASL, [1] = ROL, [2] = LSR, [3] = ROR,
            [4] = STX, [5] = LDX, [6] = DEC, [7] = INC, },
    [2] = { [1] = BIT, [2] = JMP, [3] = JMP, [4] = STY,
            [5] = LDY, [6] = CPY, [7] = CPX, }, // no 0th entry
};

// Set of lookup tables for operations; the first is for group 1 instructions
// and the second is used for groups 2 and 3. Each of them is indexed by the
// 'mode' bits of each opcode
static Mode mode_table[][8] = {
    [0] = { [0] = MODE_INDIRECT_X,  [1] = MODE_ZEROPAGE,
            [2] = MODE_IMMEDIATE,   [3] = MODE_ABSOLUTE,
            [4] = MODE_INDIRECT_Y,  [5] = MODE_ZEROPAGE_X,
            [6] = MODE_ABSOLUTE_Y,  [7] = MODE_ABSOLUTE_X, },
    [1] = { [0] = MODE_IMMEDIATE,   [1] = MODE_ZEROPAGE,
            [2] = MODE_ACCUMULATOR, [3] = MODE_ABSOLUTE,
            [5] = MODE_ZEROPAGE_X,  [7] = MODE_ABSOLUTE_X, }, // no 4th or 6th
};

// Decode an 8-bit opcode, translating it into an operation-addressing mode
// pair that can be more easily processed by the CPU
Instruction decode(uint8_t opcode) {
    bool basic = true;
    Instruction inst = { .op = NOP, .mode = MODE_IMPLIED };
    // A select group of instructions are best decoded via a simple switch-case.
    // Most of these use the implied addressing mode. The one exception is the
    // JSR instruction, which uses absolute addressing.
    switch(opcode) {
        case 0x00: inst.op = BRK; break;
        case 0x08: inst.op = PHP; break;
        case 0x18: inst.op = CLC; break;
        case 0x20: inst.op = JSR; inst.mode = MODE_ABSOLUTE; break;
        case 0x28: inst.op = PLP; break;
        case 0x38: inst.op = SEC; break;
        case 0x40: inst.op = RTI; break;
        case 0x48: inst.op = PHA; break;
        case 0x58: inst.op = CLI; break;
        case 0x60: inst.op = RTS; break;
        case 0x68: inst.op = PLA; break;
        case 0x78: inst.op = SEI; break;
        case 0x88: inst.op = DEY; break;
        case 0x8A: inst.op = TXA; break;
        case 0x98: inst.op = TYA; break;
        case 0x9A: inst.op = TXS; break;
        case 0xA8: inst.op = TAY; break;
        case 0xAA: inst.op = TAX; break;
        case 0xB8: inst.op = CLV; break;
        case 0xBA: inst.op = TSX; break;
        case 0xC8: inst.op = INY; break;
        case 0xCA: inst.op = DEX; break;
        case 0xD8: inst.op = CLD; break;
        case 0xE8: inst.op = INX; break;
        case 0xEA: inst.op = NOP; break;
        case 0xF8: inst.op = SED; break;
        default:
            basic = false;
    }
    if (basic) return inst;
    // The remaining opcodes can mostly be cleanly mapped to their corresponding
    // operations and addressing modes by inspecting bit patterns in them. Most
    // conform to the pattern 0bAAABBBCC, where AAA determines the operation, BBB
    // determines the addressing mode, and CC determines the instruction group.
    uint8_t op = (opcode & 0xE0) >> 5;   // AAA, 'op' bits
    uint8_t mode = (opcode & 0x1C) >> 2; // BBB, 'mode' bits
    uint8_t group = opcode & 0x03;       // CC, 'group' bits
    switch(group) {
        case 1:
            // Group 1 instructions: the most regular ones
            inst.op = op_table[0][op];
            inst.mode = mode_table[0][mode];
            error_if(inst.op == STA && inst.mode == MODE_IMMEDIATE);
            break;
        case 2:
            // Group 2 instructions: the second most regular ones, still not
            // terrible. Irregularities arise in immediate, accumulator and
            // absolute,* addressing modes
            error_if(mode == 4 || mode == 6); // invalid 'mode' bits
            inst.op = op_table[1][op];
            inst.mode = mode_table[1][mode];
            error_if(inst.op != LDX && inst.mode == MODE_IMMEDIATE);
            // Irregularities in individual instructions
            switch(inst.op) {
                case STX:
                    // No support for absolute,X mode
                    error_if(inst.mode == MODE_ABSOLUTE_X);
                    // Zeropage,X becomes zeropage,Y (+1)
                    if(inst.mode == MODE_ZEROPAGE_X) ++inst.mode;
                    break;
                case LDX:
                    // LDX does not support indexing by X; those modes
                    // become indexed by Y in this instruction. Changing this
                    // is as simple as incrementing the mode
                    if(inst.mode == MODE_ZEROPAGE_X
                        || inst.mode == MODE_ABSOLUTE_X) ++inst.mode;
                    break;
                default:
                    // All other instructions in this group are regular
                    break;
            }
            break;
        case 0:
            // Group 3 instructions: they are very similar to the group 2
            // instructions in the way they are organized, but are a lot less
            // regular. The branch instructions form a subgroup of group 3 and
            // have their own, particular way of being interpreted
            if(mode == 4) {
                // All branch instructions fall here. They fit the bit pattern
                // 0bXXY10000, where XX determines the flag to be checked and Y,
                // whether it must be set or clear in order for the branch to
                // actually take place
                inst.mode = MODE_RELATIVE;
                uint8_t flag = (op & 0x6) >> 1; // XX
                uint8_t status = op & 0x1;      // Y
                switch(flag) {
                    case 0: inst.op = status ? BMI : BPL; break; // NEGATIVE
                    case 1: inst.op = status ? BVS : BVC; break; // OVERFLOW
                    case 2: inst.op = status ? BCS : BCC; break; // CARRY
                    case 3: inst.op = status ? BEQ : BNE; break; // ZERO
                }
                return inst;
            }
            // Addressing modes are looked up in the same table as group 2,
            // except accumulator mode is not supported in group 3
            error_if(op == 0 || mode == 2 || mode == 6);
            inst.mode = mode_table[1][mode];
            inst.op = op_table[2][op];
            // Irregularities in individual instructions (almost all of them)
            switch(inst.op) {
                case BIT:
                    // No support for anything but zeropage and absolute
                    error_if(inst.mode != MODE_ZEROPAGE
                        && inst.mode != MODE_ABSOLUTE);
                    break;
                case JMP:
                    // JMP might be absolute or indirect, depending on the
                    // actual opcode in question (4C or 6C, respectively)
                    error_if(inst.mode != MODE_ABSOLUTE);
                    if(opcode == 0x6C) inst.mode = MODE_INDIRECT;
                    break;
                case STY:
                    // No support for immediate or absolute,X
                    error_if(inst.mode == MODE_IMMEDIATE
                        || inst.mode == MODE_ABSOLUTE_X);
                    break;
                case CPY:
                    // No support for zeropage,X or absolute,X
                    error_if(inst.mode == MODE_ZEROPAGE_X
                        || inst.mode == MODE_ABSOLUTE_X);
                    break;
                case CPX:
                    // No support for zeropage,X or absolute,X
                    error_if(inst.mode == MODE_ZEROPAGE_X
                        || inst.mode == MODE_ABSOLUTE_X);
                    break;
                default:
                    // LDY is the only truly regular instruction here
                    break;
            }
            break;
        default:
            inst.op = ERR; // weird unrecognized opcode
    }
    return inst;
}

#undef error_if
