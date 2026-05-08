#include <cmos.h>
#include <stdio.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>

void write(unsigned char addr, unsigned char value) {
    outb(0x70, addr);  
    outb(0x71, value);
}

unsigned char read(unsigned char addr) {
    outb(0x70, addr);  
    return inb(0x71);  
}