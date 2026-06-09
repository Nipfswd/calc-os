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

uint32_t syscall_handler(struct registers *regs) {
    uint32_t syscall_num = regs->eax;
    uint32_t arg1 = regs->ebx;
    uint32_t arg2 = regs->ecx;
    uint32_t arg3 = regs->edx;
    uint32_t return_value = 0;

    switch (syscall_num) {
        case SYS_EXIT: {
            delete_task(current_task);
            ncount = 1;
            break;
        }

        case SYS_READ:
            if (arg1 == 0) {
                while (1) {
                    uint8_t scancode = get_scancode();
                    if (scancode != 0) {
                        return_value = (uint32_t)scancode;
                        break;
                    }
                    __asm__ __volatile__("hlt");
                }
            }
            break;

        case SYS_WRITE:
            if (arg1 == 1 || arg1 == 2) {
                printk((char*)arg2, (uint8_t)arg3);
                return_value = 1;
            }
            break;

        case SYS_OPEN: { 
            uint16_t open_cluster = find_file_in_root((const char*)arg1);
            if (open_cluster != 0) {
                return_value = (uint32_t)open_cluster;
            } else {
                return_value = 0;
            }
            break;
        }

        case SYS_TIME: {
            int hours = 0;
            int minutes = 0;
            get_time(&hours, &minutes); 

            return_value = (hours << 16) | minutes;
            break;
        }
        case SYS_GETPID: {
            return_value = (uint32_t)current_task;
            break;
        }

        case SYS_UNAME: {
            char* user_buffer = (char*)arg1;
            
            copy_string(user_buffer, "CalcOS 9.2");
            
            return_value = 1; 
            break;
        }
        
        default:
            return_value = 0;
            break;
    }
    return return_value;
}