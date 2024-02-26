#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x38,       // SEC ; set carry
        0xF8,       // SED ; enable decimal mode
        0xA9, 0x15, // LDA #21 ; acc = 0x15

        0xE9, 0x06, // SBC #06 ; acc = 0x09 (BCD arithmetic)
        0xE5, 0x00, // SBC $00 ; acc = 0x99, CARRY clear
    };
    Machine d = {0};
    load_code(&d, code, sizeof(code));
    write(&d, 0x00, 0x10); // ZEROPAGE
    disassemble(stdout, &d, read, CODE_START, sizeof(code));

    Processor proc;
    processor_init(&proc, read, write, &d);
    REPEAT(3) processor_step(&proc); // skip setup

    // Basic BCD arithmetic correctness
    processor_step(&proc);
    assert(proc.acc == 0x09);

    // BCD results wrap around at 0x100, carry is cleared on borrow. Also,
    // values in the 0x80-0x99 range set the negative flag
    processor_step(&proc);
    assert(proc.acc == 0x99);
    assert_flag_clear(proc, FLAG_CARRY);
    assert_flag_set(proc, FLAG_NEGATIVE);

    printf("Acc: 0x%02X\n", proc.acc);
    return TEST_OK;
}
