#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>

void create_file(const char* name_11, uint8_t* data, int size) {
    uint16_t bpb_buf[256];
    uint8_t fat_table[512 * 9];
    uint16_t root_buf[256];

    ata_read_sector(0, bpb_buf);
    struct fat12_bpb* bpb = (struct fat12_bpb*)bpb_buf;
    for (int i = 0; i < bpb->fat_size_sectors; i++)
        ata_read_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));

    uint16_t free_cluster = 0;
    for (uint16_t i = 2; i < 4084; i++) {
        if (get_fat_entry(i, fat_table) == 0) {
            free_cluster = i;
            break;
        }
    }
    if (free_cluster == 0) return; 

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + 511) / 512;
    
    int found_entry = 0;
    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, root_buf);
        struct fat12_entry* entries = (struct fat12_entry*)root_buf;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00 || (uint8_t)entries[i].name[0] == 0xE5) {
                memcpy(entries[i].name, name_11, 11);
                entries[i].attributes = 0x20; 
                entries[i].first_cluster = free_cluster;
                entries[i].file_size = size; 

                ata_write_sector(root_lba + s, root_buf);
                found_entry = 1;
                break;
            }
        }
        if (found_entry) break;
    }

    set_fat_entry(free_cluster, 0xFFF, fat_table);
    for (int i = 0; i < bpb->fat_size_sectors; i++)
        ata_write_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));

    uint32_t data_lba = root_lba + root_sectors;
    ata_write_sector(data_lba + (free_cluster - 2), (uint16_t*)data);

    if (disk_initialized) {
        disk_initialized = 0;
    }
}

void delete_file(const char* name_11) {
    uint16_t bpb_buf[256];
    uint8_t fat_table[512 * 9];
    uint16_t root_buf[256];

    ata_read_sector(0, bpb_buf);
    struct fat12_bpb* bpb = (struct fat12_bpb*)bpb_buf;
    for (int i = 0; i < bpb->fat_size_sectors; i++) {
        ata_read_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));
    }

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + 511) / 512;
    
    uint16_t cluster_to_free = 0;
    int file_found = 0;

    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, root_buf);
        struct fat12_entry* entries = (struct fat12_entry*)root_buf;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] != 0x00 && (uint8_t)entries[i].name[0] != 0xE5) {
                if (memcmp(entries[i].name, name_11, 11) == 1) {
                    cluster_to_free = entries[i].first_cluster;
                    entries[i].name[0] = 0xE5; 

                    ata_write_sector(root_lba + s, root_buf);
                    file_found = 1;
                    break;
                }
            }
        }
        if (file_found) break;
    }

    if (!file_found) return;

    uint16_t current_cluster = cluster_to_free;
    while (current_cluster != 0 && current_cluster < 0xFF8) {
        uint16_t next_cluster = get_fat_entry(current_cluster, fat_table);
        set_fat_entry(current_cluster, 0x000, fat_table);
        current_cluster = next_cluster;
    }

    for (int i = 0; i < bpb->fat_size_sectors; i++) {
        ata_write_sector(bpb->reserved_sectors + i, (uint16_t*)(fat_table + (i * 512)));
    }

    if (disk_initialized) {
        disk_initialized = 0;
    }
}