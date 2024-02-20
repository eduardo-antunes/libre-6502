#ifndef LIBRE_6502_TEST_H
#define LIBRE_6502_TEST_H

#include <assert.h>

// Standard return values
#define TEST_OK   0
#define TEST_FAIL 1

// Defines standard read and write functions to setup the address space of
// unit tests. It expects an array of 1024 bytes to be provided as userdata.
#define boilerplate(read, write) \
    uint8_t read(void *p, uint16_t addr) {                             \
        if(addr == RESET_VECTOR || addr == RESET_VECTOR + 1) return 0; \
        uint8_t *code = p;                                             \
        return code[addr & 0x03FF];                                    \
    }                                                                  \
    void write(void *p, uint16_t addr, uint8_t data) {                 \
        uint8_t *code = p;                                             \
        code[addr & 0x03FF] = data;                                    \
    }


// Very useful for "skipping" setup instructions in the tests
#define REPEAT(n) for(int i = 0; i < (n); ++i)

// "Pretty" print value of a register in the processor; poor replacement for
// actual pretty printing, but such feature will come eventually
#define prettyreg(proc, reg) printf(#reg ": 0x%02X\n", (proc).reg)

// Assert that a flag is set
#define assert_fset(proc, flag) assert((proc).status & (flag))

// Assert that a flag is clear
#define assert_fclear(proc, flag) assert(!((proc).status & (flag)))

#endif // LIBRE_6502_TEST_H
