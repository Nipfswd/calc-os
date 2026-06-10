#include <pci.h>
#include <stdint.h>

typedef struct {
    volatile uint32_t* port_base;
    __attribute__((aligned(1024))) uint32_t clb[256];
    __attribute__((aligned(256))) uint32_t fis[64];
    __attribute__((aligned(128))) uint8_t ct[256];
} ahci_port_t;

ahci_port_t ports[32];

int ahci_find() {
    for (int i = 0; i < device_count; i++) {
        if (devices[i].class_id == 0x01 && devices[i].subclass == 0x06 && devices[i].prog_if == 0x01) {
            return i;
        }
    }
    return -1;
}

uint32_t ahci_get_mmio() {
    int index = ahci_find();
    if (index < 0) return -1;

    uint32_t mmio = pci_read_config_dword(devices[index].bus, 
        devices[index].slot, 
        devices[index].func,
        0x24
    ) & 0xFFFFFFF0; 
    return mmio;
}

void ahci_init() {
    uint32_t mmio = ahci_get_mmio();
    if ( mmio == 0 || mmio == 0xFFFFFFFF) return;

    volatile uint32_t* hba = (volatile uint32_t*)mmio;
    uint32_t cap = hba[0];   
    uint32_t pi = hba[3];   

    for (int port = 0; port < 32; port++) {
        if (!(pi & (1 << port))) {
            continue;
        }
        volatile uint32_t* port_base = (volatile uint32_t*)(mmio + 0x100 + port * 0x80);
        
        uint16_t ssts = port_base[0x28 / 4];
        
        uint8_t det = ssts & 0x0F;
        uint8_t ipm = (ssts >> 8) & 0x0F;

        if (det == 3 && ipm == 1) {
            volatile uint32_t cmd = port_base[0x18 / 4];
            ports[port].port_base = port_base;

            cmd = cmd & ~1;
            cmd = cmd & ~(1 << 4);
            port_base[0x18 / 4] = cmd;
            volatile uint32_t cmd2 = port_base[0x18 / 4];
            
            while ((cmd2 & (1 << 15)) || (cmd2 & (1 << 14))) {
                cmd2 = port_base[0x18 / 4];
            }

            uint32_t clb_addr = (uint32_t)ports[port].clb;
            ports[port].port_base[0] = clb_addr;
            ports[port].port_base[1] = 0;

            uint32_t fis_addr = (uint32_t)ports[port].fis;
            ports[port].port_base[2] = fis_addr;
            ports[port].port_base[3] = 0;

            uint32_t ct_addr = (uint32_t)ports[port].ct;
            ports[port].clb[4] = ct_addr;
            ports[port].clb[5] = 0;

            ports[port].clb[1] = 1;

            cmd = cmd | 1;
            cmd = cmd | (1 << 4);
            port_base[0x18 / 4] = cmd;
        }
    }
}

void ahci_identify(int port, uint8_t* ident_buf) {
    uint32_t ident_buf_addr = (uint32_t)ident_buf;
    ports[port].ct[128] = (ident_buf_addr >> 0) & 0xFF;
    ports[port].ct[129] = (ident_buf_addr >> 8) & 0xFF;
    ports[port].ct[130] = (ident_buf_addr >> 16) & 0xFF;
    ports[port].ct[131] = (ident_buf_addr >> 24) & 0xFF;

    ports[port].ct[140] = (511 >> 0) & 0xFF;
    ports[port].ct[141] = (511 >> 8) & 0xFF;
    ports[port].ct[142] = (511 >> 16) & 0xFF;
    ports[port].ct[143] = (511 >> 24) & 0xFF;

    ports[port].ct[1] = 1 << 7;
    ports[port].ct[2] = 0xEC;
    ports[port].ct[3] = 0;

    for (int i = 4; i < 15; i++) {
        ports[port].ct[i] = 0;
    }

    uint32_t ci = ports[port].port_base[0x38 / 4];
    ci = ci | (1 << 0);

    ports[port].port_base[0x38 / 4] = ci;

    while ((ports[port].port_base[0x38 / 4]) & 1);
}

void ahci_read_sector(uint64_t lba, uint8_t* buffer) {
    
}