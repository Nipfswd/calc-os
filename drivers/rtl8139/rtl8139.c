#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>

uint16_t rtl_port_base = 0;

#define RX_BUFFER_SIZE 0x8000 

uint8_t rtl_rx_buffer[RX_BUFFER_SIZE + 2048] __attribute__((aligned(16)));
volatile uint8_t rtl_tx_buffer[4][2048] __attribute__((aligned(4)));

static uint8_t tx_counter = 0;
static uint8_t mac_addr[6]; 
static uint16_t rx_offset = 0; 

int rtl8139_find() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == 0x10EC && devices[i].device_id == 0x8139) {
            return i;
        }
    }
    return -1;
}

uint32_t rtl8139_get_port_base() {
    int index = rtl8139_find();
    if (index < 0) return 0;

    uint32_t io_base = pci_read_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x10);
    io_base = io_base & ~0x3;
    return io_base; 
}

void rtl8139_init() {
    uint32_t rx_phys = (uint32_t)rtl_rx_buffer;

    int index = rtl8139_find();
    if (index < 0) return;

    uint16_t cmd = pci_read_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04);
    cmd = cmd | 0x0007; 
    pci_write_config_word(devices[index].bus, devices[index].slot, devices[index].func, 0x04, cmd);

    rtl_port_base = rtl8139_get_port_base();
    if (rtl_port_base == 0) return;

    for (int i = 0; i < 6; i++) {
        mac_addr[i] = inb(rtl_port_base + i);
    }

    outb(rtl_port_base + 0x37, 0x10);
    uint32_t timeout = 500000;
    while ((inb(rtl_port_base + 0x37) & 0x10) && timeout > 0) {
        timeout--;
    }
    if (timeout == 0) return;

    outl(rtl_port_base + 0x44, 0x0000E70F);

    outl(rtl_port_base + 0x30, rx_phys);

    outw(rtl_port_base + 0x38, 0);
    outw(rtl_port_base + 0x3A, 0); 
    outw(rtl_port_base + 0x3C, 0x0005);

    outb(rtl_port_base + 0x37, 0x0C);

    tx_counter = 0;
    rx_offset = 0; 
}

void send_pack(uint8_t data, uint8_t dest_mac[6]) {
    if (rtl_port_base == 0) return;

    uint32_t phys_tx_buf = (uint32_t)rtl_tx_buffer[tx_counter]; 

    for (int i = 0; i < 6; i++) rtl_tx_buffer[tx_counter][i] = dest_mac[i];
    for (int i = 0; i < 6; i++) rtl_tx_buffer[tx_counter][i + 6] = mac_addr[i];

    rtl_tx_buffer[tx_counter][12] = 0x88;
    rtl_tx_buffer[tx_counter][13] = 0xB5;
    rtl_tx_buffer[tx_counter][14] = data;

    for (int i = 15; i < 60; i++) rtl_tx_buffer[tx_counter][i] = 0;
    
    uint32_t length = 60; 
    uint8_t tx_reg_offset = tx_counter * 4;

    outl(rtl_port_base + 0x20 + tx_reg_offset, phys_tx_buf);
    outl(rtl_port_base + 0x10 + tx_reg_offset, length);

    uint32_t tx_timeout = 200000;
    while (!(inl(rtl_port_base + 0x10 + tx_reg_offset) & (1 << 13)) && tx_timeout > 0) {
        tx_timeout--;
    }

    tx_counter = (tx_counter + 1) % 4;
}

uint8_t read_pack() {
    if (rtl_port_base == 0) return 0;

    if (inb(rtl_port_base + 0x37) & 0x01) {
        return 0;
    }
    
    uint8_t* header = rtl_rx_buffer + rx_offset;
    uint16_t status = header[0] | (header[1] << 8);
    if (!(status & 0x01)) {
        return 0;
    }

    uint16_t length = header[2] | (header[3] << 8);
    uint8_t* payload = header + 4;
    uint16_t ethertype = (payload[12] << 8) | payload[13];
    
    if (ethertype == 0x88B5) {
        char c = payload[14];
        put_char(c, 1);
    } 
    else if (ethertype == 0x0800 && length >= 42 && length < 1600) {
        uint16_t offset = 14 + 20 + 8;
        uint8_t* icmp_data = payload + offset;
        for (int i = 0; i < length - offset - 4; i++) {
            char c = icmp_data[i];
            put_char(c, 1);
        }
    }

    rx_offset = (rx_offset + length + 4 + 3) & ~3;
    rx_offset = rx_offset % RX_BUFFER_SIZE;

    int capr = (int)rx_offset - 16;
    if (capr < 0) {
        capr += RX_BUFFER_SIZE;
    }

    outw(rtl_port_base + 0x3A, (uint16_t)capr);

    return 1;
}