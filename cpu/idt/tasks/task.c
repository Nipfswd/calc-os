#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>

Task task_list[2];
int current_task = 0;
unsigned int task2_stack[1024]; 

void task2_main() {
    int hours, minutes;
    int old_hours = -1, old_minutes = -1;
    char h_str[3], m_str[3];

    while(1) {
        if (current_mode != 0 && is_window_crt == 0) {
            get_time(&hours, &minutes);

            if (hours != old_hours || minutes != old_minutes) {
                draw_rect(970, 5, 46, 30, 0); 

                x = 972; y = 15;
                
                itoa(hours, h_str);
                if (hours < 10) print("0", 15); 
                print(h_str, 15);
                
                print(":", 15);
                
                itoa(minutes, m_str);
                if (minutes < 10) print("0", 15); 
                print(m_str, 15);

                old_hours = hours;
                old_minutes = minutes;
            }
        }
        __asm__ __volatile__("hlt");
    }
}

void prepare_task2() {
    uint32_t* st = &task2_stack[1024];

    *(--st) = 0x202;       
    *(--st) = 0x08;    
    *(--st) = (uint32_t)task2_main; 

    *(--st) = 0;
    *(--st) = 0;

    for (int i = 0; i < 8; i++) {
        *(--st) = 0;
    }

    *(--st) = 0x10;

    task_list[1].esp = (uint32_t)st;
}