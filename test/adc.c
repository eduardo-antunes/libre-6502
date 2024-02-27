#include <stdint.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x18,             // CLC      ; clear carry
        0x29, 0x00,       // AND #0   ; acc = 0
        0xA2, 0xE0,       // LDX #$E0 ; x = $E0

        0x69, 0x80,       // ADC #$80 ; acc = $80, NEG flag set
        0x65, 0xE0,       // ADC V0   ; acc = $B0
        0x75, 0x01,       // ADC V1   ; acc = $30, OVERFLOW set
        0x18,             // CLC      ; clear carry
        0x6D, 0x01, 0x03, // ADC W0   ; acc = $02, CARRY set

        // 16-bit addition test; calculate A + B, store in C
        0x18,            // CLC       ; clear carry
        0xA5, 0x00,      // LDA Al    ; lsb of A in the acc
        0x65, 0x02,      // ADC Bl    ; add lsb of B
        0x85, 0x04,      // STA Cl    ; store result in lsb of C
        0xA5, 0x01,      // LDA Ah    ; msb of A in the acc
        0x65, 0x03,      // ADC Bh    ; add msb of B
        0x85, 0x05,      // STA Ch    ; store result in msb of C
    };

    Fake f = {0};
    load_code(&f, code, sizeof(code));
    disassemble(stdout, &f, read, CODE_START, sizeof(code));
    write(&f, 0xE0,   0x30); // V0 = 48  ($30)
    write(&f, 0xE1,   0x80); // V1 = 128 ($80)
    write(&f, 0x0301, 0xD2); // W0 = 210 ($D2)
    // 16-bit A = 7,601 ($1DB1)
    write(&f, 0x00, 0xB1);
    write(&f, 0x01, 0x1D);
    // 16-bit B = 50,890 ($C6CA)
    write(&f, 0x02, 0xCA);
    write(&f, 0x03, 0xC6);
    // 16-bit C will be at addresses 0x04 and 0x05, it must have the final
    // value of A + B = 58,491 ($E47B)

    Processor proc;
    processor_init(&proc, read, write, &f);
    REPEAT(3) processor_step(&proc); // skip setup

    // Should set negative flag
    processor_step(&proc);
    assert(proc.acc == 0x80);
    assert_flag_set(proc, FLAG_NEGATIVE);

    // Arithmetic correctness
    processor_step(&proc);
    assert(proc.acc == 0xB0);

    // CRITICAL should set overflow flag
    processor_step(&proc);
    assert_flag_set(proc, FLAG_OVERFLOW);

    // CRITICAL should set carry flag
    REPEAT(2) processor_step(&proc);
    assert(proc.acc == 0x02);
    assert_flag_set(proc, FLAG_CARRY);

    // 16-bit addition test
    REPEAT(7) processor_step(&proc);
    uint16_t c = read(&f, 0x04);
    c |= read(&f, 0x05) << 8;
    assert(c == 0xE47B);

    return TEST_OK;
}
