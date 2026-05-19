#ifndef MM_H
#define MM_H
#include <stdint.h>

void init_identity_paging();
extern void enable_paging();
extern unsigned int page_directory[1024] __attribute__((aligned(4096)));
extern unsigned int page_table[1024] __attribute__((aligned(4096)));

#endif 