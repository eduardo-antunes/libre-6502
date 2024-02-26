#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x18,       // CLC ; clear carry
        0xF8,       // SED ; enable decimal mode
        0xA9, 0x09, // LDA #09 ; acc = 0x09

        0x69, 0x01, // ADC #01 ; acc = 0x10 (BCD arithmetic)
        0x65, 0x01, // ADC $01 ; acc = 0x85, NEG flag set
        0x69, 0x17, // ADC #23 ; acc = 0x02, CARRY flag set
        0x18,       // CLC ; clear carry
        0x69, 0x98, // ADC #$98 ; acc = 0, CARRY and ZERO set
    };
    Machine d = {0};
    load_code(&d, code, sizeof(code));
    write(&d, 0x01, 0x75); // ZEROPAGE
    disassemble(stdout, &d, read, CODE_START, sizeof(code));

    Processor proc;
    processor_init(&proc, read, write, &d);
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

    printf("Acc: 0x%02X\n", proc.acc);
    return TEST_OK;
}
