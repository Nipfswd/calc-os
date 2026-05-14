#ifndef ATA_H
#define ATA_H
#include <stdint.h>

void ata_read_sector(uint32_t lba, uint16_t* buffer);
void ata_write_sector(uint32_t lba, uint16_t* buffer);

extern uint32_t section_offset;
int test_disk_at_offset(uint32_t offset);
void test_disk();
void ata_set_offset(uint32_t offset);

#endif
