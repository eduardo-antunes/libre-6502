#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x18,       // CLC      ; clear carry
        0xF8,       // SED      ; enable decimal mode
        0xA9, 0x09, // LDA #09  ; acc = $09

        0x69, 0x01, // ADC #01  ; acc = $10 (BCD arithmetic)
        0x65, 0x00, // ADC V0   ; acc = $85, NEG flag set
        0x69, 0x17, // ADC #23  ; acc = $02, CARRY flag set
        0x18,       // CLC      ; clear carry
        0x69, 0x98, // ADC #$98 ; acc = 0, CARRY and ZERO set
    };

    Fake f = {0};
    load_code(&f, code, sizeof(code));
    disassemble(stdout, &f, read, CODE_START, sizeof(code));
    write(&f, 0x00, 0x75); // V0 = 117 ($75)

    Processor proc;
    processor_init(&proc, read, write, &f);
    REPEAT(3) processor_step(&proc); // skip setup

    // Basic BCD arithmetic correctness
    processor_step(&proc);
    assert(proc.acc == 0x10);

    // BCD results from 0x80 to 0x99 set the negative flag
    processor_step(&proc);
    assert(proc.acc == 0x85);
    assert_flag_set(proc, FLAG_NEGATIVE);

    // BCD results wrap around at 0x100, setting carry
    processor_step(&proc);
    assert(proc.acc == 0x02);
    assert_flag_set(proc, FLAG_CARRY);

    // Should also set the zero flag when appropriate
    REPEAT(2) processor_step(&proc);
    assert(proc.acc == 0x00);
    assert_flag_set(proc, FLAG_ZERO);

    return TEST_OK;
}
