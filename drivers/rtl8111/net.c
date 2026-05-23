#include <video.h>
#include <utils.h>
#include <idt.h>
#include <stdint.h>
#include <pci.h>
#include <mm.h>

volatile uint8_t* rtl_mmio;

int rtl8111_find() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].vendor_id == 0x10EC && devices[i].device_id == 0x8168) {
            return i; 
        }
    }
    return -1;
}

int rtl8111_get_mmio() {
    uint32_t mmio_base;

    int index = rtl8111_find();
    if (index < 0) return 0;

    mmio_base = pci_read_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x20);
    mmio_base = mmio_base & ~0xF;

    return mmio_base;
}

void rtl8111_init() {
    int index = rtl8111_find();
    if (index < 0) return;

    uint16_t cmd = pci_read_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x04);

    cmd = cmd | 0x06; 

    pci_write_config_dword(devices[index].bus, devices[index].slot, devices[index].func, 0x04, cmd);

    uint32_t mmio_base = rtl8111_get_mmio();
    if (mmio_base == 0) return;
    rtl_mmio = (volatile uint8_t*)mmio_base;

    rtl_mmio[0x50] = 0xC0; 

    rtl_mmio[0x37] = 0x10;

    uint32_t timeout = 10000;
    while ((rtl_mmio[0x37] & 0x10) && timeout > 0) {
        timeout--;
    }

    if (timeout == 0) {
        print("RTL8111 initialization timed out\n", 1);
        return;
    }
}
