#ifndef LIBRE_6502_TEST_UTILS_H
#define LIBRE_6502_TEST_UTILS_H

#include <assert.h>
#include <stdint.h>
#include <stddef.h>

// Exit codes recognized by meson's testing system; can be used to signal the
// status of any particular test to the tester

#define TEST_OK    0
#define TEST_FAIL  1
#define TEST_SKIP  77
#define TEST_ERROR 99

#define REPEAT(n) for(int i = 0; i < (n); ++i)
#define assert_flag_set(proc, flag) assert((proc).status & (flag))
#define assert_flag_clear(proc, flag) assert(!((proc).status & (flag)))

// Simple addressing space for tests, consists solely of 1KiB of RAM mirrored
// throughout all addresses. Loads code at the CODE_START address, which is set
// to leave the zero page freely available to any variables

#define CODE_START  0x0100

typedef struct { uint8_t ram[1024]; } Machine; // simple addressing space

void load_code(Machine *d, uint8_t code[], size_t code_length); // loads code

uint8_t read(void *ptr, uint16_t addr); // reads from given address

void write(void *ptr, uint16_t addr, uint8_t data); // writes to given address

#endif // LIBRE_6502_TEST_UTILS_H
