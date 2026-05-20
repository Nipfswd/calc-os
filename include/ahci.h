#ifndef AHCI_H
#define AHCI_H
#include <stdint.h>

extern uint32_t* ahci_base_ptr;

struct ahci_port {
    uint64_t clb;   
    uint64_t fb;    
    uint32_t reserved[2];
    uint32_t cmd; 
    uint8_t reserved2[28];
    uint32_t ci;
} __attribute__((packed));

struct ahci_hba {
    uint32_t cap;
    uint32_t ghc;
    uint8_t reserved[248];
    struct ahci_port ports[32];
} __attribute__((packed));

struct ahci_cmd_header {
    uint8_t cfl : 5;
    uint8_t res1 : 3;
    uint8_t flags;
    uint16_t prdtl;
    uint32_t prdbc;
    uint64_t ctba; 
    uint32_t reserved[4];
} __attribute__((packed));

struct ahci_prd {
    uint64_t dba;
    uint32_t reserved;
    uint32_t dbc;
} __attribute__((packed));

#endif