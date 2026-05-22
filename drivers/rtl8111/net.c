#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>

uint32_t mmio_base = 0;
static uint32_t current_rx = 0;

struct rtl8111_rx_desc rx_ring[4] __attribute__((aligned(256)));
uint8_t rx_buffer[4][1536] __attribute__((aligned(8)));

struct rtl8111_tx_desc tx_ring __attribute__((aligned(256)));
uint8_t tx_buffer[1536] __attribute__((aligned(8)));

void rtl8111_init() {
    pci_device_t* dev = NULL; 

    for (int i = 0; i < device_count; i++) {
        uint32_t clean_vendor = devices[i].vendor_id & 0xFFFF;
        uint32_t clean_class = devices[i].class_id & 0xFF; 

        if (clean_vendor == 0x10EC && clean_class == 0x02) {
            dev = &devices[i];
            break;
        }
    }

    if (dev == NULL) {
        return;
    }

    uint32_t pci_command = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x04);
    pci_command |= 0x0006; 
    pci_write_config_dword(dev->bus, dev->slot, dev->func, 0x04, pci_command);

    uint32_t bar2 = pci_read_config_dword(dev->bus, dev->slot, dev->func, 0x18);
    mmio_base = bar2 & 0xFFFFFFF0;

    if (mmio_base == 0) {
        return;
    }

    map_mmio_space(mmio_base);

    mmio_write8(mmio_base + 0x37, 0x10); 
    int timeout = 1000;
    while ((mmio_read8(mmio_base + 0x37) & 0x10) != 0) {
        delay_ticks(5);
        timeout--;
        if (timeout <= 0) {
            break; 
        }
    }

    for (int i = 0; i < 4; i++) {
        rx_ring[i].vlan = 0;
        rx_ring[i].buf_addr_low = (uint32_t)rx_buffer[i];
        rx_ring[i].buf_addr_high = 0;                 
        rx_ring[i].status = 0x80000000 | 1536; 
    }
    rx_ring[3].status |= 0x40000000;

    mmio_write32(mmio_base + 0xE4, (uint32_t)&rx_ring[0]);
    mmio_write32(mmio_base + 0xE8, 0); 

    mmio_write32(mmio_base + 0x44, 0x0F | (1 << 7)); 

    mmio_write8(mmio_base + 0x37, 0x08);

    tx_ring.buf_addr_low = (uint32_t)tx_buffer;
    tx_ring.buf_addr_high = 0;
    tx_ring.vlan = 0;
    tx_ring.status = 0x40000000; 

    mmio_write32(mmio_base + 0x20, (uint32_t)&tx_ring);
    mmio_write32(mmio_base + 0x24, 0);

    mmio_write8(mmio_base + 0x37, 0x08 | 0x04);
}

void rtl8111_recv() {
    if ((rx_ring[current_rx].status & 0x80000000) != 0) {
        return;
    }

    print("Byte: ", 1);
    char buf[16];
    htoa(rx_buffer[current_rx][0], buf);
    print(buf, 1);
    print("\n", 1);

    rx_ring[current_rx].status = 0x80000000 | 1536;
    if (current_rx == 3) {
        rx_ring[current_rx].status |= 0x40000000;
    }

    current_rx = (current_rx + 1) % 4;
}

void rtl8111_send(uint8_t byte_to_send, uint8_t* dest_mac) {
    tx_buffer[0] = dest_mac[0]; 
    tx_buffer[1] = dest_mac[1]; 
    tx_buffer[2] = dest_mac[2]; 
    tx_buffer[3] = dest_mac[3]; 
    tx_buffer[4] = dest_mac[4]; 
    tx_buffer[5] = dest_mac[5];

    tx_buffer[6] = 0x00; 
    tx_buffer[7] = 0xDE; 
    tx_buffer[8] = 0xAD; 
    tx_buffer[9] = 0xBE; 
    tx_buffer[10] = 0xEF; 
    tx_buffer[11] = 0x00;

    tx_buffer[12] = 0x88;
    tx_buffer[13] = 0xB5;

    tx_buffer[14] = byte_to_send;

    for(int i = 15; i < 60; i++) {
        tx_buffer[i] = 0;
    }

    tx_ring.status = 0x80000000 | 0x20000000 | 0x10000000 | 60;

    *(volatile uint8_t*)(mmio_base + 0xD9) = 0x02;

    int timeout = 500000;
    while ((tx_ring.status & 0x80000000) != 0) {
        delay_ticks(1);
        if (--timeout == 0) {
            return;
        }
    }
}