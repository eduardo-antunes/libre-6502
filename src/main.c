#include <stdio.h>
#include <stdint.h>

#include "emulator.h"
#include "debug.h"

int main() {
    Emulator em;
    emulator_init(&em);

    // Write some instructions to main memory
    uint8_t prog[] = {
        0xA9, 0x01, // LDA #$01
        0xC9, 0x02, // CMP #$02
        0xD0, 0x02, // BNE $0204 ; eu acho
        0x85, 0x22, // STA $22
    };
    int n = sizeof(prog) / sizeof(prog[0]);
    emulator_load_prog(&em, prog, n);
    disassemble(stdout, &em, 4);
    return 0;
}
