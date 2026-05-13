#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>
#include <ata.h>
#include <fat.h>

static uint8_t cached_fat[512 * 9];
static struct fat12_bpb cached_bpb;
static int disk_initialized = 0;

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

        print(name_buf, 1);
        if (ext_len > 0) {
            print(".", 1);
            print(ext_buf, 1);
        }
        print("\n", 0); 
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

void read_file_basic(uint16_t cluster, uint8_t* destination, uint8_t* fat_table, struct fat12_bpb* bpb) {
    uint32_t root_dir_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;
    uint32_t fat_size = bpb->num_fats * bpb->fat_size_sectors;
    uint32_t data_lba = bpb->reserved_sectors + fat_size + root_dir_sectors;

    uint16_t current_cluster = cluster;

    while (current_cluster < 0xFF8) {
        uint32_t cluster_lba = ((current_cluster - 2) * bpb->sectors_per_cluster) + data_lba;

        for (uint8_t i = 0; i < bpb->sectors_per_cluster; i++) {
            ata_read_sector(cluster_lba + i, (uint16_t*)destination);
            destination = destination + 512;
        }

        current_cluster = get_fat_entry(current_cluster, fat_table);
        
        if (current_cluster == 0) break;
    }
}

void read_file(const char* filename_11, uint8_t* buffer) {
    if (!disk_initialized) {
        uint16_t bpb_buf[256];
        ata_read_sector(0, bpb_buf);
        cached_bpb = *(struct fat12_bpb*)bpb_buf;

        for (uint32_t i = 0; i < cached_bpb.fat_size_sectors; i++) {
            ata_read_sector(cached_bpb.reserved_sectors + i, 
                            (uint16_t*)(cached_fat + (i * 512)));
        }
        disk_initialized = 1;
    }

    uint16_t start_cluster = find_file_in_root(filename_11);

    if (start_cluster != 0) {
        read_file_basic(start_cluster, buffer, cached_fat, &cached_bpb);
    } else {
        print("File not found!\n", 0);
    }
}

void list_files() {
    uint16_t buffer[256]; 
    
    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, buffer);
        struct fat12_entry* entries = (struct fat12_entry*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) return;
            
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue; 

            char name_buf[9];
            char ext_buf[4];
            int name_len = 0;
            int ext_len = 0;

            for(int j = 0; j < 8; j++) {
                if(entries[i].name[j] != ' ') {
                    name_buf[name_len++] = entries[i].name[j]; 
                }
            }
            name_buf[name_len] = '\0';

            for(int j = 0; j < 3; j++) {
                if(entries[i].ext[j] != ' ') {
                    ext_buf[ext_len++] = entries[i].ext[j];
                }
            }
            ext_buf[ext_len] = '\0';

            print(name_buf, 1);
            if(ext_len > 0) {
                print(".", 1);
                print(ext_buf, 1);
            }

            print("\n", 1);
        }
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