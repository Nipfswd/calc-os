#include <cmos.h>
#include <stdio.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>

static int is_safe_cmos_addr(unsigned char addr) {
    return addr >= 0x60 && addr <= 0x65;
}

void write(unsigned char addr, unsigned char value) {
    if (!is_safe_cmos_addr(addr)) return;
    outb(0x70, addr);  
    outb(0x71, value);
}

unsigned char read(unsigned char addr) {
    if (!is_safe_cmos_addr(addr)) return 0;
    outb(0x70, addr);  
    return inb(0x71);  
}