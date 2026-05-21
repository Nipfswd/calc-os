#ifndef PCI_H
#define PCI_H
#include <stdint.h>

typedef struct {
    uint8_t bus;
    uint8_t slot;
    uint8_t func;
    uint16_t vendor_id;
    uint16_t device_id;
    uint8_t class_code;   
    uint8_t subclass;    
} pci_device_t;

void pci_print_devices();
void pci_scan();

#endif