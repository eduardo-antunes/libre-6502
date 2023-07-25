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
#include "emulator.h"

int main(int argc, char *argv[]) {
    if(argc < 2) {
        fprintf(stderr, "usage: %s <file.nes>\n", argv[0]);
        return 1;
    }
    Emulator libre_nes;
    emulator_init(&libre_nes, argv[1]);
    emulator_start(&libre_nes);
    emulator_free(&libre_nes);
    return 0;
}
