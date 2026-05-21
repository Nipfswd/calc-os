#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <ata.h>
#include <idt.h>
#include <stdint.h>
#include <fat.h>
#include <pci.h>

uint32_t mmio_base = 0;

struct rtl8111_rx_desc rx_ring __attribute__((aligned(256)));
uint8_t rx_buffer[1536] __attribute__((aligned(8)));

struct rtl8111_tx_desc tx_ring __attribute__((aligned(256)));
uint8_t tx_buffer[1536] __attribute__((aligned(8)));

void rtl8111_init() {
    pci_device_t* dev = NULL; 

    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == 0x10EC && devices[i].device_id == 0x8168) {
            dev = &devices[i];
            break;
        }
    }

    if (dev == NULL) {
        return;
    }

    uint32_t pci_command = pci_read_config_dword(dev->bus, dev->slot, dev->func, 1);
    pci_command |= 0x0006; 
    pci_write_config_dword(dev->bus, dev->slot, dev->func, 1, pci_command);

    uint32_t bar2 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 6);
    
    mmio_base = bar2 & 0xFFFFFFF0; 

    print("MMIO Base: ", 1);
    char buf[16];
    htoa(mmio_base, buf);
    print(buf, 1);
    print("\n", 1);

    mmio_write8(mmio_base + 0x37, 0x10); 

    while ((mmio_read8(mmio_base + 0x37) & 0x10) != 0) {
        delay_ticks(25);
    }

    rx_ring.status = 0xC0000600; 
    rx_ring.vlan = 0;
    rx_ring.buf_addr_low = (uint32_t)rx_buffer; 
    rx_ring.buf_addr_high = 0;                 
    *(volatile uint32_t*)(mmio_base + 0xE4) = (uint32_t)&rx_ring;
    *(volatile uint32_t*)(mmio_base + 0xE8) = 0; 

    *(volatile uint32_t*)(mmio_base + 0x44) = 0xF; 

    mmio_write8(mmio_base + 0x37, 0x08);

    tx_ring.buf_addr_low = (uint32_t)tx_buffer;
    tx_ring.buf_addr_high = 0;

    *(volatile uint32_t*)(mmio_base + 0x20) = (uint32_t)&tx_ring;
    *(volatile uint32_t*)(mmio_base + 0x24) = 0;

    mmio_write8(mmio_base + 0x37, 0x08 | 0x04);
}

void rtl8111_recv() {
    while ((rx_ring.status & 0x80000000) != 0) {
        delay_ticks(25);
    }

    uint32_t packet_len = rx_ring.status & 0x3FFF;

    print("First byte: ", 1);
    char buf[16];
    htoa(rx_buffer[0], buf);
    print(buf, 1);
    print("\n", 1);

    rx_ring.status |= 0x80000000;
}

void rtl8111_send(uint8_t byte_to_send) {
    tx_buffer[0] = byte_to_send;
    
    for(int i = 1; i < 60; i++) {
        tx_buffer[i] = 0;
    }

    tx_ring.status = 0xF000003C;

    *(volatile uint8_t*)(mmio_base + 0x38) = 0x01;

    while ((tx_ring.status & 0x80000000) != 0) {
        delay_ticks(25);
    }
}