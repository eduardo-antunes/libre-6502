#include "utils.h"
#include <stdint.h>

// Read from the machine address space
uint8_t read(void *ptr, uint16_t addr) {
    Fake *f = ptr;
    return f->ram[addr & 0x3FF];
}

// Write to the machine address space
void write(void *ptr, uint16_t addr, uint8_t data) {
    Fake *f = ptr;
    f->ram[addr & 0x3FF] = data;
}

// Load code into the machine's RAM
void load_code(Fake *f, uint8_t code[], size_t code_length) {
    for(size_t i = 0; i < code_length && i < 1024; ++i)
        // Leave zero page available for variables
        f->ram[CODE_START + i] = code[i];
    write(f, 0xFFFC, CODE_START & 0xFF);
    write(f, 0xFFFD, CODE_START >> 8);
}
