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

#include "cart.h"
#include "emulator.h"
#include "disassembler.h"

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "usage: %s <file.nes>\n", argv[0]);
        return 1;
    }
    Cartrige cart;
    cart_init(&cart);
    int err = cart_load(&cart, argv[1]);
    if(!err) {
        printf("File sucessfully loaded! PRG banks: %u, CHR banks: %u\n",
            cart.prg_banks, cart.chr_banks);
    }
    cart_free(&cart);
    return 0;
}
