#include <stdint.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x18,            // CLC       ; clear carry
        0x29, 0x00,      // AND #0    ; acc = 0
        0xA2, 0xE0,      // LDX #$E0  ; x = $E0

        0x69, 0x80,      // ADC #$80  ; neg
        0x65, 0xE0,      // ADC $E0
        0x75, 0x01,      // ADC $01,X ; overflow!
        0x18,            // CLC       ; clear carry
        0x6D, 0x01, 0x03 // ADC $0301 ; carry
    };
    Machine d = {0};
    load_code(&d, code, sizeof(code));
    write(&d, 0xE0, 0x30);   // ZEROPAGE
    write(&d, 0xE1, 0x80);   // ZEROPAGE,X
    write(&d, 0x0301, 0xD2); // ABSOLUTE
    disassemble(stdout, &d, read, CODE_START, sizeof(code));

    Processor proc;
    processor_init(&proc, read, write, &d);
    REPEAT(3) processor_step(&proc); // skip setup

    // Should set negative flag
    processor_step(&proc);
    assert_flag_set(proc, FLAG_NEGATIVE);
    processor_step(&proc);

    // CRITICAL should set overflow flag
    processor_step(&proc);
    assert_flag_set(proc, FLAG_OVERFLOW);

    // CRITICAL should set carry flag
    REPEAT(2) processor_step(&proc);
    assert_flag_set(proc, FLAG_CARRY);

    printf("Acc: 0x%02X\n", proc.acc);
    return TEST_OK;
}
