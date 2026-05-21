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

uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset << 2);
    outl(0xCF8, address);
    return inl(0xCFC);
}

int check_device(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vendor_id = pci_read_config_dword(bus, slot, func, 0) & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return 0; 
    }
    return 1;
}

void pci_scan() {
    for (uint16_t bus = 0; bus < 256; bus++) {
        for (uint8_t slot = 0; slot < 32; slot++) {
            if (check_device(bus, slot, 0)) {
                
                uint32_t reg3 = pci_read_config_dword(bus, slot, 0, 3);
                uint8_t header_type = (reg3 >> 16) & 0xFF;
                
                uint8_t max_functions = 1;
                if (header_type & 0x80) {
                    max_functions = 8; 
                }
                
                for (uint8_t func = 0; func < max_functions; func++) {
                    if (check_device(bus, slot, func)) {
                        uint32_t data = pci_read_config_dword(bus, slot, func, 0);
                        uint16_t vendor_id = data & 0xFFFF;
                        uint16_t device_id = (data >> 16) & 0xFFFF;
                        
                        char vendor_str[9];
                        char device_str[9];
                        htoa(vendor_id, vendor_str);
                        print("\nPCI Device Found: Vendor ID = ", 1);
                        print(vendor_str, 1);
                        itoa(device_id, device_str);
                        print(", Device ID = ", 1);
                        print(device_str, 1);
                        print("\n", 1);
                    }
                }
                
            }
        }
    }
}