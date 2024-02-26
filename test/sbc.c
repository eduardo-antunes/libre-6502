#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x38,       // SEC ; set carry
        0xA9, 0x40, // LDA #$40 ; acc = 0x40
        0xA2, 0x01, // LDX #$01 ; x = 1

        0xE9, 0x0A, // SBC #10 ; intended: acc = 0x36
        0xE5, 0x00, // SBC $00 ; acc = 0xE6, CARRY clear, NEG set
        0x38,       // SEC ; set carry
        0xF5, 0x19, // SBC $19,X ; acc = 0x7F, OVERFLOW set
    };
    Machine d = {0};
    load_code(&d, code, sizeof(code));
    write(&d, 0x00, 0x50); // ZEROPAGE
    write(&d, 0x1A, 0x67); // ZEROPAGE
    disassemble(stdout, &d, read, CODE_START, sizeof(code));

    Processor proc;
    processor_init(&proc, read, write, &d);
    REPEAT(3) processor_step(&proc); // skip setup

    // Basic arithmetic correctness
    processor_step(&proc);
    assert(proc.acc == 0x36);

    // CRITICAL Clear the carry flag on borrow, and set the NEG flag
    processor_step(&proc);
    assert(proc.acc == 0xE6);
    assert_flag_clear(proc, FLAG_CARRY);
    assert_flag_set(proc, FLAG_NEGATIVE);

    // CRITICAL Set the overflow flag on incorrect sign
    REPEAT(2) processor_step(&proc);
    assert(proc.acc == 0x7F);
    assert_flag_set(proc, FLAG_OVERFLOW);

    printf("Acc: 0x%02X\n", proc.acc);
    return TEST_OK;
}
