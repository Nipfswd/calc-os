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
        case SYS_EXIT:
            delete_task(current_task);
            ncount = 1;
            break;

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
                print((char*)arg2, (uint8_t)arg3);
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

        case SYS_EXEC: {  
            uint16_t exec_cluster = find_file_in_root((const char*)arg1);
            
            if (exec_cluster == 0) {
                return_value = 0;
                break;
            }

            uint8_t* prog_buffer = (uint8_t*)kmalloc(65536);
            
            if (!prog_buffer) {
                return_value = 0;
                break;
            }

            read_file((const char*)arg1, prog_buffer);

            __asm__ __volatile__("cli");
            uint32_t* st = &task2_stack[1024];
            *(--st) = 0x202;
            *(--st) = 0x08;
            *(--st) = (uint32_t)prog_buffer;
            *(--st) = 0;
            *(--st) = 0;
            for (int i = 0; i < 8; i++) {
                *(--st) = 0;
            }
            *(--st) = 0x10;

            task_list[1].esp = (uint32_t)st;
            task_list[1].id = 1;
            task_list[1].is_active = 1;
            __asm__ __volatile__("sti");

            return_value = 1;
            break;
        }

        default:
            return_value = 0;
            break;
    }

    return return_value;
}