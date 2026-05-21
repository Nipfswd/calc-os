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

pci_device_t devices[32];
int device_count = 0;

uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset << 2);
    outl(0xCF8, address);
    return inl(0xCFC);
}

void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value) {
    uint32_t address = (1 << 31) | (bus << 16) | (slot << 11) | (func << 8) | (offset << 2);
    outl(0xCF8, address);
    outl(0xCFC, value);
}

int check_device(uint8_t bus, uint8_t slot, uint8_t func) {
    uint32_t vendor_id = pci_read_config_dword(bus, slot, func, 0) & 0xFFFF;
    if (vendor_id == 0xFFFF) {
        return 0; 
    }
    return 1;
}

void pci_scan() {
    device_count = 0; 

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
                        if (device_count >= 32) {
                            return; 
                        }

                        uint32_t data0 = pci_read_config_dword(bus, slot, func, 0);
                        uint32_t data2 = pci_read_config_dword(bus, slot, func, 2);

                        devices[device_count].bus = (uint8_t)bus;
                        devices[device_count].slot = slot;
                        devices[device_count].func = func;
                        devices[device_count].vendor_id = data0 & 0xFFFF;
                        devices[device_count].device_id = (data0 >> 16) & 0xFFFF;

                        devices[device_count].class_code = (data2 >> 24) & 0xFF;
                        devices[device_count].subclass   = (data2 >> 16) & 0xFF;

                        device_count++;
                    }
                }
                
            }
        }
    }
}

void pci_print_devices() {
    if (device_count == 0) {
        print("No PCI devices found.\n", 1);
        return;
    }

    for (int i = 0; i < device_count; i++) {
        print("PCI Device found: ", 1);
        
        char buf[16];
        
        print("Bus: ", 1);
        itoa(devices[i].bus, buf);
        print(buf, 1);
        
        print(" | Vendor: ", 1);
        htoa(devices[i].vendor_id, buf);
        print(buf, 1);
        
        print(" | Class: ", 1);
        htoa(devices[i].class_code, buf);
        print(buf, 1);
        
        print("\n", 1);
    }
}