#ifndef PCI_H
#define PCI_H
#include <stdint.h>

#define NULL ((void*)0)

#define mmio_write8(addr, val)  (*(volatile uint8_t*)(addr) = (val))
#define mmio_read8(addr)        (*(volatile uint8_t*)(addr))
#define mmio_write32(addr, val) (*(volatile uint32_t*)(addr) = (val))
#define mmio_read32(addr)       (*(volatile uint32_t*)(addr))

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_id;   
    uint8_t subclass;    
} pci_device_t;

void pci_print_devices();
void pci_scan();
uint32_t pci_read_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset);
void pci_write_config_dword(uint8_t bus, uint8_t slot, uint8_t func, uint8_t offset, uint32_t value);

extern pci_device_t devices[32];
extern int device_count;

struct rtl8111_rx_desc {
    uint32_t status;   
    uint32_t vlan;   
    uint32_t buf_addr_low;  
    uint32_t buf_addr_high; 
} __attribute__((packed));

struct rtl8111_tx_desc {
    uint32_t status;       
    uint32_t vlan;       
    uint32_t buf_addr_low; 
    uint32_t buf_addr_high;
} __attribute__((packed));

void rtl8111_init();
void rtl8111_recv();
void rtl8111_send(uint8_t byte_to_send, uint8_t* dest_mac);

#endif