#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>
#include <forth.h>
#include <fat.h>

file_descriptor_t fd_table[10];

uint32_t syscall_handler(struct registers *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;

    switch (syscall_num) {
        case SYS_EXIT:
            for (int i = 3; i < 10; i++) {
                fd_table[i].cluster = 0;
                fd_table[i].offset = 0;
                fd_table[i].used = 0;
            }
            delete_task(current_task);
            return 0;

        case SYS_OPEN: {
            const char* path = (const char*)arg1;
            if (path < (const char*)(13 * 4096)) return -EFAULT; 

            uint16_t cluster = find_file_in_root(path);
            if (cluster == 0) return -ENOENT;

            for (int i = 3; i < 10; i++) {
                if (!fd_table[i].used) {
                    fd_table[i].cluster = cluster;
                    fd_table[i].offset = 0;
                    fd_table[i].used = 1;
                    return i; 
                }
            }
            return -EMFILE;
        }

        case SYS_READ: {
            int fd = (int)arg1;
            char* buf = (char*)arg2;
            size_t count = (size_t)arg3;

            if (buf < (char*)(13 * 4096)) return -EFAULT;

            if (fd == 0) {
                size_t read_bytes = 0;
                while (read_bytes < count) {
                    uint8_t scancode = get_scancode();
                    if (scancode != 0) {
                        buf[read_bytes++] = scancode;
                    }
                    if (read_bytes == count) break;
                    __asm__ __volatile__("hlt"); 
                }
                return read_bytes;
            } 
            
            if (fd < 3 || fd >= 10 || !fd_table[fd].used) return -EBADF; 
            if (fd < 3 || fd >= 10 || !fd_table[fd].used) return -EBADF; 

            uint16_t cluster = fd_table[fd].cluster;
            uint32_t offset = fd_table[fd].offset;

            static uint8_t sector_buf[512];

            uint32_t root_lba = cached_bpb.reserved_sectors + (cached_bpb.num_fats * cached_bpb.fat_size_sectors);
            uint32_t root_sectors = ((cached_bpb.root_entries * 32) + 511) / 512;
            uint32_t data_lba = root_lba + root_sectors;

            ata_read_sector(data_lba + (cluster - 2), sector_buf);

            size_t bytes_to_copy = count;
            if (offset + bytes_to_copy > 512) { 
                bytes_to_copy = 512 - offset;
            }

            memcpy(buf, sector_buf + offset, bytes_to_copy);

            fd_table[fd].offset = fd_table[fd].offset + bytes_to_copy;

            return bytes_to_copy;
            
            return 0; 
        }

        case SYS_WRITE: {
            if (arg1 == 1 || arg1 == 2) {
                printk((char*)arg2, (uint8_t)arg3);
                return 0;
            }
            return -EBADF;
        }

        case SYS_CLOSE: {
            int fd = (int)arg1;
            if (fd < 3 || fd >= 10 || !fd_table[fd].used) return -EBADF;

            fd_table[fd].cluster = 0;
            fd_table[fd].offset = 0;
            fd_table[fd].used = 0; 
            return 0; 
        }
        

        default:
            return -ENOSYS; 
    }
}