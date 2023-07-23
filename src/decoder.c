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
#include <stdbool.h>
#include "decoder.h"

// Lookup tables for operations and modes

static const Operation op_table[][8] = {
    // Group 1 operations
    [0] = { [0] = ORA, [1] = AND, [2] = EOR, [3] = ADC,
            [4] = STA, [5] = LDA, [6] = CMP, [7] = SBC, },
    // Group 2 operations
    [1] = { [0] = ASL, [1] = ROL, [2] = LSR, [3] = ROR,
            [4] = STX, [5] = LDX, [6] = DEC, [7] = INC, },
    // Group 3 operations (no valid entry for 0)
    [2] = { [1] = BIT, [2] = JMP, [3] = JMP, [4] = STY,
            [5] = LDY, [6] = CPY, [7] = CPX, },
};

static const Addressing mode_table[][8] = {
    // Group 1 modes
    [0] = { [0] = MODE_INDIRECT_X,  [1] = MODE_ZEROPAGE,
            [2] = MODE_IMMEDIATE,   [3] = MODE_ABSOLUTE,
            [4] = MODE_INDIRECT_Y,  [5] = MODE_ZEROPAGE_X,
            [6] = MODE_ABSOLUTE_Y,  [7] = MODE_ABSOLUTE_X, },
    // Group 2 and 3 modes (no valid entries for 4 and 6)
    [1] = { [0] = MODE_IMMEDIATE,   [1] = MODE_ZEROPAGE,
            [2] = MODE_ACCUMULATOR, [3] = MODE_ABSOLUTE,
            [5] = MODE_ZEROPAGE_X,  [7] = MODE_ABSOLUTE_X, },
};

// Report decoding error
static void error(Instruction *inst, uint8_t opcode, const char *context) {
    fprintf(stderr, "[!] Nonsensical opcode: %02X\n", opcode);
    if(context != NULL) fprintf(stderr, "[?] %s\n", context);
    inst->op = ERR;
}

// Decode an 7-bit opcode, translating it into its correspoding CPU operation
// and addressing mode. Key component of execution
Instruction decode(uint8_t opcode) {
    bool basic = true;
    Instruction inst = { .op = NOP, .mode = MODE_IMPLIED };
    // A select group of instructions are best decoded via a simple switch-case.
    // Most of these use the implied addressing mode. The one exception is the
    // JSR instruction, which uses absolute addressing.
    switch(opcode) {
        case 0x08: inst.op = PHP; break;
        case 0x18: inst.op = CLC; break;
        case 0x20: inst.op = JSR; inst.mode = MODE_ABSOLUTE; break;
        case 0x38: inst.op = PLP; break;
        case 0x48: inst.op = PHA; break;
        case 0x58: inst.op = CLI; break;
        case 0x60: inst.op = RTS; break;
        case 0x68: inst.op = PLA; break;
        case 0x78: inst.op = SEI; break;
        case 0x88: inst.op = DEY; break;
        case 0x98: inst.op = TYA; break;
        case 0x9A: inst.op = TXS; break;
        case 0xA8: inst.op = TAY; break;
        case 0xB8: inst.op = CLV; break;
        case 0xBA: inst.op = TSX; break;
        case 0xC8: inst.op = INY; break;
        case 0xD8: inst.op = CLD; break;
        case 0xE8: inst.op = INX; break;
        case 0xF8: inst.op = SED; break;
        case 0x00:
            // TODO this is the BRK instruction
            // inst.op = BRK;
            break;
        case 0x40:
            // TODO this is the RTI instruction
            // inst.op = RTI;
            break;
        default:
            basic = false;
    }
    if (basic) return inst;
    // The remaining opcodes can mostly be cleanly mapped to their corresponding
    // operations and addressing modes by inspecting bit patterns in them. Most
    // conform to the pattern 0bAAABBBCC, where AAA determines the operation, BBB
    // determines the addressing mode, and CC determines the instruction group.
    uint8_t op = (opcode & 0xE0) >> 5;
    uint8_t mode = (opcode & 0x1C) >> 2;
    uint8_t group = opcode & 0x03;
    switch(group) {
        case 1:
            // Group 1 instructions: the most regular ones
            inst.op = op_table[0][op];
            inst.mode = mode_table[0][mode];
            if(inst.op == STA && inst.mode == MODE_IMMEDIATE)
                error(&inst, opcode, "LDA does not support immediate addressing");
            break;
        case 2:
            // Group 2 instructions: the second most regular ones, still not
            // terrible. Irregularities arise in immediate, accumulator and
            // absolute + index addressing modes
            if(mode == 4 || mode == 6) {
                error(&inst, opcode, "Invalid mode specifier for group 2 instruction");
                return inst;
            }
            inst.op = op_table[1][op];
            inst.mode = mode_table[1][mode];
            // Irregularities in individual instructions
            switch(inst.op) {
                case STX:
                    // Irregularities in the STX instruction
                    if(inst.mode == MODE_ABSOLUTE_X)
                        error(&inst, opcode, "STX does not support absolute_x addressing");
                    else if(inst.mode == MODE_ZEROPAGE_X)
                        // Zero page x becomes zero page y
                        inst.mode = MODE_ZEROPAGE_Y;
                    else if(inst.mode == MODE_ACCUMULATOR)
                        // With accumulator addressing, STX becomes TXA
                        inst.op = TXA;
                    break;
                case LDX:
                    // Irregularities in the LDX instruction
                    if(inst.mode == MODE_ZEROPAGE_X)
                        // Zero page x becomes zero page y
                        inst.mode = MODE_ZEROPAGE_Y;
                    else if(inst.mode == MODE_ABSOLUTE_X)
                        // Absolute x becomes absolute y
                        inst.mode = MODE_ABSOLUTE_Y;
                    else if(inst.mode == MODE_ACCUMULATOR)
                        // With accumulator addressing, LDX becomes TAX
                        inst.op = TAX;
                    break;
                case DEC:
                    // Irregularities in the DEC instruction
                    if(inst.mode == MODE_ACCUMULATOR)
                        // With accumulator addressing, DEC becomes DEX
                        inst.op = DEX;
                    break;
                case INC:
                    // Irregularities in the DEC instruction
                    if(inst.mode == MODE_ACCUMULATOR)
                        // With accumulator addressing, INC becomes NOP
                        inst.op = NOP;
                    break;
                default:
                    // Other instructions in this group are regular
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
                uint8_t flag = (op & 6) >> 1;
                uint8_t status = op & 1;
                switch(flag) {
                    case 0:
                        // NEGATIVE flag
                        inst.op = status ? BMI : BPL;
                        break;
                    case 1:
                        // OVERFLOW flag
                        inst.op = status ? BVS : BVC;
                        break;
                    case 2:
                        // CARRY flag
                        inst.op = status ? BCS : BCC;
                        break;
                    case 3:
                        // ZERO flag
                        inst.op = status ? BEQ : BNE;
                        break;
                }
                return inst;
            }
            // Addressing modes are looked up in the same table as group 2,
            // except accumulator mode is not supported in group 3
            inst.mode = mode_table[1][mode];
            if(inst.mode == MODE_ACCUMULATOR)
                error(&inst, opcode, "Group 3 instructions don't support accumulator addressing");
            if(op == 0)
                error(&inst, opcode, NULL);
            inst.op = op_table[2][op];
            // Irregularities in individual instructions (almost all of them)
            switch(inst.op) {
                case BIT:
                    // Irregularities in the BIT instruction
                    if(inst.mode != MODE_ZEROPAGE && inst.mode != MODE_ABSOLUTE)
                        error(&inst, opcode, "BIT only supports zero page and absolute addressing");
                    break;
                case JMP:
                    // Irregularities in the JMP instruction (this is a weird one)
                    inst.mode = (op == 2) ? MODE_INDIRECT : MODE_ABSOLUTE;
                    break;
                case STY:
                    // Irregularities in the STY instruction
                    if(inst.mode == MODE_IMMEDIATE || inst.mode == MODE_ABSOLUTE_X)
                        error(&inst, opcode, "STY does not support immediate nor absolute_x addressing");
                    break;
                case CPY:
                    // Irregularities in the CPY instruction
                    if(inst.mode == MODE_ZEROPAGE_X || inst.mode == MODE_ABSOLUTE_X)
                        error(&inst, opcode, "CPY does not support zero page x nor absolute_x addressing");
                    break;
                case CPX:
                    // Irregularities in the CPX instruction
                    if(inst.mode == MODE_ZEROPAGE_X || inst.mode == MODE_ABSOLUTE_X)
                        error(&inst, opcode, "CPX does not support zero page x nor absolute_x addressing");
                    break;
                default:
                    // LDY is regular lol
                    break;
            }
            break;
        default:
            error(&inst, opcode, "Opcode doesn't fit an instruction group");
    }
    return inst;
}
