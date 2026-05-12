#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>

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

        print(entries[i].name, 1);
        
        print('.', 1);

        print(entries[i].ext, 1);

        print('\n', 0); 
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