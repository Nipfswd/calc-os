#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <ata.h>
#include <idt.h>
#include <stdint.h>
#include <fat.h>

uint32_t section_offset = 0;

void ata_wait_bsy() {
    while (inb(0x1F7) & 0x80);
}

void ata_wait_drq() {
    while (!(inb(0x1F7) & 0x08));
}

void ata_read_sector_real(uint32_t lba, uint16_t* buffer) {
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

void ata_write_sector_real(uint32_t lba, uint16_t* buffer) {
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

void ata_read_sector(uint32_t lba, uint16_t* buffer) {
    ata_read_sector_real(lba + section_offset, buffer);
}

void ata_write_sector(uint32_t lba, uint16_t* buffer) {
    ata_write_sector_real(lba + section_offset, buffer);
}

void test_disk() {
    ata_wait_bsy();
    outb(0x1F6, 0xA0);
    outb(0x1F7, 0xEC); 
    
    uint8_t status = inb(0x1F7);
    if (status == 0) {
        screen_clear();
        print("Disk not found. Controller might be in AHCI mode\n", 1); 
    }
}

void ata_set_offset(uint32_t offset) {
    section_offset = offset;
}

int test_disk_at_offset(uint32_t offset) {
    ata_wait_bsy();
    outb(0x1F6, 0xA0); 
    
    uint16_t buffer[256];
    ata_read_sector_real(0 + offset, buffer);
    
    if (buffer[255] == 0xAA55) {
        return 1;
    }
    
    return 0; 
}