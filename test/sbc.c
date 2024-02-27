#include <stdint.h>
#include <assert.h>

#include "debug.h"
#include "processor.h"
#include "utils.h"

int main() {
    uint8_t code[] = {
        0x38,       // SEC       ; set carry
        0xA9, 0x40, // LDA #$40  ; acc = $40
        0xA2, 0x01, // LDX #$01  ; x = $01

        0xE9, 0x0A, // SBC #10   ; acc = $36
        0xE5, 0x06, // SBC V0    ; acc = $E6, CARRY clear and NEG set
        0x38,       // SEC       ; set carry
        0xF5, 0x19, // SBC V1    ; acc = $7F, OVERFLOW set

        // 16-bit subtraction test; calculate B - A, store in C
        0x38,       // SEC       ; set carry
        0xA5, 0x02, // SBC Bl    ; lsb of B in acc
        0xE5, 0x00, // SBC Al    ; subtract lsb of A
        0x85, 0x04, // STA Cl    ; store result in lsb of C
        0xA5, 0x03, // SBC Bh    ; msb of B in acc
        0xE5, 0x01, // SBC Ah    ; subtract msb of A
        0x85, 0x05, // STA Ch    ; store result in msb of C
    };

    Fake f = {0};
    load_code(&f, code, sizeof(code));
    disassemble(stdout, &f, read, CODE_START, sizeof(code));
    write(&f, 0x06, 0x50); // V0 = 80  ($50)
    write(&f, 0x1A, 0x67); // V1 = 103 ($67)
    // 16-bit A = 7,601 ($1DB1)
    write(&f, 0x00, 0xB1);
    write(&f, 0x01, 0x1D);
    // 16-bit B = 50,890 ($C6CA)
    write(&f, 0x02, 0xCA);
    write(&f, 0x03, 0xC6);
    // 16-bit C will be at addresses 0x04 and 0x05, it must have the final
    // value of B - A = 43,289 ($A919)

    Processor proc;
    processor_init(&proc, read, write, &f);
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

    // 16-bit subtraction test
    REPEAT(7) processor_step(&proc);
    uint16_t c = read(&f, 0x04);
    c |= read(&f, 0x05) << 8;
    assert(c == 0xA919);

    return TEST_OK;
}
