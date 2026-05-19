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

void init_identity_paging() {
    for(int i = 0; i < 1024; i++) {
        page_table[i] = (i * 4096) | 0x3;
    }

    page_directory[0] = ((unsigned int)page_table) | 0x3;

    for(int i = 1; i < 1024; i++) {
        page_directory[i] = 0x0;
    }
}