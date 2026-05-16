#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <ata.h>
#include <idt.h>
#include <stdint.h>
#include <fat.h>

void ata_wait_bsy() {
    while (inb(0x1F7) & 0x80);
}

void ata_wait_drq() {
    while (!(inb(0x1F7) & 0x08));
}

void ata_read_sector(uint32_t lba, uint16_t* buffer) {
    asm volatile("sti");
    ata_wait_bsy(); 

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1); 
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));

    ata_interrupt_received = 0;

    outb(0x1F7, 0x20);

    while (!ata_interrupt_received) {
    }

    for (int i = 0; i < 256; i++) {
        buffer[i] = inw(0x1F0); 
    }
}

void ata_write_sector(uint32_t lba, uint16_t* buffer) {
    asm volatile("sti");
    ata_wait_bsy();

    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F));
    outb(0x1F2, 1);
    outb(0x1F3, (uint8_t)lba);
    outb(0x1F4, (uint8_t)(lba >> 8));
    outb(0x1F5, (uint8_t)(lba >> 16));

    outb(0x1F7, 0x30); 

    ata_wait_bsy();
    ata_wait_drq(); 

    for (int i = 0; i < 256; i++) {
        outw(0x1F0, buffer[i]); 
    }

    ata_interrupt_received = 0;
    while (!ata_interrupt_received);
}