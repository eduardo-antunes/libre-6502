#include "processor.h"
#include "test.h"

boilerplate(read, write);

int main() {
    uint8_t code[1024] = {
        0x18,            // CLC       ; clear carry
        0x29, 0x00,      // AND #0    ; acc = 0
        0xA2, 0xE0,      // LDX #$E0  ; x = $E0

        0x69, 0x80,      // ADC #$80  ; neg
        0x65, 0xE0,      // ADC $E0
        0x75, 0x01,      // ADC $01,X ; overflow!
        0x6D, 0x01, 0x03 // ADC $0301 ; carry
    };
    code[0xE0] = 0x30;   // ZEROPAGE
    code[0xE1] = 0x80;   // ZEROPAGE,X
    code[0x0301] = 0xD2; // ABSOLUTE

    Processor proc;
    processor_init(&proc, read, write, code);
    REPEAT(3) processor_step(&proc);

    // Should set negative flag
    processor_step_debug(&proc);
    assert_fset(proc, FLAG_NEGATIVE);
    processor_step(&proc);

    // CRITICAL should set overflow flag
    processor_step_debug(&proc);
    assert_fset(proc, FLAG_OVERFLOW);

    // CRITICAL should set carry flag
    processor_step_debug(&proc);
    assert_fset(proc, FLAG_CARRY);

    return TEST_OK;
}
