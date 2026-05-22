#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <mm.h>

unsigned int page_directory[1024] __attribute__((aligned(4096)));
unsigned int page_table[1024] __attribute__((aligned(4096)));

unsigned int rtl_mmio_page_table[1024] __attribute__((aligned(4096)));

void init_identity_paging() {
    for(int i = 0; i < 1024; i++) {
        page_table[i] = (i * 4096) | 0x3; 
    }
    page_directory[0] = ((unsigned int)page_table) | 0x3;

    for(int i = 1; i < 1024; i++) {
        page_directory[i] = 0x0;
    }
}

void map_mmio_space(uint32_t physical_base) {
    uint32_t pde_idx = physical_base >> 22;
    uint32_t base_4mb = physical_base & 0xFFC00000;

    for(int i = 0; i < 1024; i++) {
        uint32_t physical_addr = base_4mb + (i * 4096);
        rtl_mmio_page_table[i] = physical_addr | 0x13; 
    }

    page_directory[pde_idx] = ((unsigned int)rtl_mmio_page_table) | 0x3;

    asm volatile("mov %cr3, %eax; mov %eax, %cr3");
}