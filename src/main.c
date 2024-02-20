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

#include "computer.h"
#include <stdint.h>

int main() {
    Computer c;
    computer_init(&c);
    uint8_t code[] = {
        0xA9, 0x05, // lda #5
        0xA2, 0x10, // lxd #16
        0x18,       // clc
        0x69, 0x11, // adc #17
    };
    computer_load_prog(&c, code, 7);
    computer_start(&c);
    return 0;
}
