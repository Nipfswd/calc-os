#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>

int disk_initialized = 0;
uint8_t cached_fat[512 * 9];
struct fat12_bpb cached_bpb;

uint16_t get_fat_entry(uint16_t cluster_n, uint8_t* fat_table) {
    uint32_t offset = cluster_n + (cluster_n / 2);
    
    uint16_t value = *(uint16_t*)(fat_table + offset);

    if (cluster_n % 2 == 0) {
        return value & 0x0FFF;
    } else {
        return value >> 4;
    }
}

void load_root_directory() {
    uint16_t buffer[256]; 

    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_dir_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_dir_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    ata_read_sector(root_dir_lba, buffer);

    struct fat12_entry* entries = (struct fat12_entry*)buffer;

    for (int i = 0; i < 16; i++) {
        if (entries[i].name[0] == 0x00) break;

        if ((uint8_t)entries[i].name[0] == 0xE5) continue;

        char name_buf[9];
        char ext_buf[4];
        int name_len = 0;
        int ext_len = 0;

        for (int j = 0; j < 8; j++) {
            if (entries[i].name[j] != ' ') {
                name_buf[name_len++] = entries[i].name[j];
            }
        }
        name_buf[name_len] = '\0';

        for (int j = 0; j < 3; j++) {
            if (entries[i].ext[j] != ' ') {
                ext_buf[ext_len++] = entries[i].ext[j];
            }
        }
        ext_buf[ext_len] = '\0';

        printk(name_buf, 1);
        if (ext_len > 0) {
            printk(".", 1);
            printk(ext_buf, 1);
        }
        printk("\n", 0); 
    }
}

uint16_t find_file_in_root(const char* target_name_11) {
    uint16_t buffer[256]; 
    
    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, buffer);
        struct fat12_entry* entries = (struct fat12_entry*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) return 0;
            
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;

            if (memcmp(entries[i].name, target_name_11, 11)) {
                return entries[i].first_cluster;
            }
        }
    }

    return 0; 
}

void read_cluster(uint16_t cluster, uint16_t* target_buffer, uint32_t data_region_lba, uint8_t sectors_per_cluster) {
    uint32_t lba = ((cluster - 2) * sectors_per_cluster) + data_region_lba;
    
    for (int i = 0; i < sectors_per_cluster; i++) {
        ata_read_sector(lba + i, target_buffer + (i * 256)); 
    }
}

uint16_t find_free_cluster(uint8_t* fat_table) {
    for (uint16_t i = 2; i < 4084; i++) {
        if (get_fat_entry(i, fat_table) == 0x000) {
            return i;
        }
    }
    return 0; 
}

void set_fat_entry(uint16_t cluster_n, uint16_t value, uint8_t* fat_table) {
    uint32_t offset = cluster_n + (cluster_n / 2);
    uint16_t* ptr = (uint16_t*)(fat_table + offset);

    if (cluster_n % 2 == 0) {
        *ptr = (*ptr & 0xF000) | (value & 0x0FFF);
    } else {
        *ptr = (*ptr & 0x000F) | (value << 4);
    }
}