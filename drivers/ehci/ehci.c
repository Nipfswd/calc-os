#include <pci.h>
#include <stdint.h>

int find_ehci() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].class_id == 0x0C &&
            devices[i].subclass == 0x03) {

            uint8_t progif = pci_read_config_byte(devices[i].bus, devices[i].slot, devices[i].func, 0x09);

            if (progif == 0x20) {
                return i; 
            }
        }
    }
    return -1;
}


void ehci_init() {
    int ehci_index = find_ehci();
    if (ehci_index == -1) {
        printk("\nEHCI device not found", 15);
        while(1);
    }

    uint32_t bar0 = pci_read_config_dword(devices[ehci_index].bus, devices[ehci_index].slot, devices[ehci_index].func, 0x10);
    uint32_t mmio = bar0 & ~0xF;

    uint8_t caplength = *(volatile uint8_t*)(mmio + 0x00);
    uint32_t hcsparams = *(volatile uint32_t*)(mmio + 0x04);
    uint32_t hccparams = *(volatile uint32_t*)(mmio + 0x08);

    uint8_t eecp = (hccparams >> 8) & 0xFF;

    if (eecp != 0) {
        volatile uint32_t* legsup = (volatile uint32_t*)(mmio + eecp);
        
        *legsup = *legsup | (1 << 24);

        while (*legsup & (1 << 16));
    }

    uint32_t op_base = mmio + caplength;

    volatile uint32_t* USBCMD  = (volatile uint32_t*)(op_base + 0x00);
    volatile uint32_t* USBSTS  = (volatile uint32_t*)(op_base + 0x04);
    volatile uint32_t* CONFIG  = (volatile uint32_t*)(op_base + 0x40);
    volatile uint32_t* PORTSC0 = (volatile uint32_t*)(op_base + 0x44);

    *USBCMD = *USBCMD | (1 << 1);   

    while (*USBCMD & (1 << 1));

    *CONFIG = 1;

    *PORTSC0 = *PORTSC0 | (1 << 8);
    while (*PORTSC0 & (1 << 8));

    *PORTSC0 = *PORTSC0 | (1 << 2);
    while (!(*PORTSC0 & (1 << 2)));
    
    uint32_t portsc = *PORTSC0;
    uint32_t speed = (portsc >> 26) & 0x3;

    static uint32_t qh[12];

    for (int i = 0; i < 12; i++) {
        qh[i] = 0;
    }

    qh[0] = (uint32_t)qh | 0x2;
    qh[1] = (64 << 16) | (1 << 27);
    qh[2] = (1 << 15);
    qh[5] = 1; 
    qh[6] = 1; 
    qh[7] = 0; 

    volatile uint32_t* ASYNCLISTADDR = (volatile uint32_t*)(op_base + 0x18);
    *ASYNCLISTADDR = (uint32_t)qh;
    *USBCMD = *USBCMD | (1 << 5);

    static uint32_t qtd[8];

    for (int i = 0; i < 8; i++) {
        qtd[i] = 0;
    }

    qtd[2] = (8 << 16) | (1 << 7);

    static uint8_t setup_packet[8];
    qtd[3] = (uint32_t)setup_packet;
    
    setup_packet[0] = 0x80; 
    setup_packet[1] = 0x06; 
    setup_packet[2] = 0x01; 
    setup_packet[3] = 0x00;
    setup_packet[4] = 0x00; 
    setup_packet[5] = 0x00; 
    setup_packet[6] = 0x12;
    setup_packet[7] = 0x00;

    if (qtd[2] & (1 << 6)) {
        printk("\nFATAL USB ERROR: HALTED", 15);
        while(1);
    }

    if (qtd[2] & 0xF8) {
        printk("\nFATAL USB ERROR", 15);
        while(1);
    }

    static uint32_t qtd_data[8];

    for (int i = 0; i < 8; i++) {
        qtd_data[i] = 0;
    }

    qtd_data[2] = 
        (18 << 16) |  
        (1 << 7)    |
        (1 << 31);   

    static uint8_t device_desc[18];
    qtd_data[3] = (uint32_t)device_desc;
    
    qtd[0] = (uint32_t)qtd_data;

    qh[5] = (uint32_t)qtd; 
    *USBCMD = *USBCMD | (1 << 6);     
    while (qtd[2] & (1 << 7)); 

    while (qtd_data[2] & (1 << 7)); 


    if (qtd_data[2] & (1 << 6)) {
        printk("\nFATAL USB ERROR (DATA): HALTED", 15);
        while (1);
    }

    if (qtd_data[2] & 0xF8) {
        printk("\nFATAL USB ERROR (DATA)", 15);
        while (1);
    }

    static uint32_t qtd_status[8];

    for (int i = 0; i < 8; i++) {
        qtd_status[i] = 0;
    }

    qtd_status[2] =
    (1 << 31) | 
    (1 << 7);   


    qtd_data[0] = (uint32_t)qtd_status;
    *USBCMD = *USBCMD | (1 << 6);
    while (qtd_status[2] & (1 << 7));

    if (qtd_status[2] & (1 << 6)) {
        printk("\nFATAL USB ERROR (STATUS): HALTED", 15);
        while (1);
    }

    if (qtd_status[2] & 0xF8) {
        printk("\nFATAL USB ERROR (STATUS)", 15);
        while (1);
    }

    if (device_desc[1] != 1) {
        printk("\nInvalid Device Descriptor", 15);
        while (1);
    }
}