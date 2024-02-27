#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x38,       // SEC     ; set carry
        0xF8,       // SED     ; enable decimal mode
        0xA9, 0x15, // LDA #21 ; acc = $15

        0xE9, 0x06, // SBC #06 ; acc = $09 (BCD arithmetic)
        0xE5, 0x00, // SBC V0  ; acc = $99, CARRY clear
    };

    Fake f = {0};
    load_code(&f, code, sizeof(code));
    disassemble(stdout, &f, read, CODE_START, sizeof(code));
    write(&f, 0x00, 0x10); // V0 = 16 ($10)

    Processor proc;
    processor_init(&proc, read, write, &f);
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

    return TEST_OK;
}
