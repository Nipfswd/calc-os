#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>

static int is_safe_cmos_addr(uint8_t addr) {
    return addr >= 0x50 && addr <= 0x54;
}

void write(uint8_t addr, uint8_t value) {
    if (!is_safe_cmos_addr(addr)) return;

    __asm__ volatile ("cli");
    outb(0x70, addr | 0x80);  
    outb(0x71, value);
    __asm__ volatile ("sti");
}

uint8_t read(uint8_t addr) {
    uint8_t res;
    __asm__ volatile ("cli");
    
    outb(0x70, addr | 0x80);  
    res = inb(0x71);  
    
    __asm__ volatile ("sti"); 
    return res;
}