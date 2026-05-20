#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <ata.h>
#include <idt.h>
#include <stdint.h>
#include <fat.h>
#include <ahci.h>

uint32_t* ahci_base_ptr = (uint32_t*)0x8000;
struct ahci_cmd_header* cmd_header = (struct ahci_cmd_header*)0x200000;
uint8_t* fis = (uint8_t*)0x202000;

void ahci_stop_port(struct ahci_port* port) {
    port->cmd = port->cmd & ~1; 
    port->cmd = port->cmd & ~16; 

    while (port->cmd & ((1 << 14) | (1 << 15)));
}

void ahci_init_port(struct ahci_port* port) {
    ahci_stop_port(port);

    port->clb = 0x200000;
    port->fb  = 0x201000;

    port->cmd = port->cmd | 16;
    delay_ticks(5);
    port->cmd = port->cmd | 1;
}

void ahci_read_sector(struct ahci_port* port, uint32_t lba_low, uint32_t buffer_addr) {
    ahci_init_port(port);

    struct ahci_cmd_header* cmd_header = (struct ahci_cmd_header*)port->clb;
    cmd_header->cfl = 5;          
    cmd_header->prdtl = 1;      
    cmd_header->ctba = 0x202000;  

    struct ahci_prd* prd = (struct ahci_prd*)(cmd_header->ctba + 0x80);
    prd->dba = buffer_addr;      
    prd->dbc = 0x800001FF;       

    uint8_t* fis = (uint8_t*)cmd_header->ctba;
    fis[0] = 0x27;                
    fis[1] = 0x80;               
    fis[2] = 0x25;               

    fis[4] = 0; 
    fis[5] = 0; 
    fis[6] = 0;
    fis[7] = 0;
    fis[8] = 0;
    fis[9] = 0;

    port->ci = port->ci | 1;
    while (port->ci & 1);
}