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

struct idt_entry   idt_entry[256];
struct idt_pointer idtp;

unsigned int timer_ticks = 0;
unsigned char *timer_str[16];

volatile int ata_interrupt_received = 0;

void set_idt_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags) {
    idt_entry[number].base_low = (base & 0xFFFF);
    idt_entry[number].base_high = (base >> 16) & 0xFFFF;
    idt_entry[number].selector = selector;
    idt_entry[number].always0 = 0;
    idt_entry[number].flags = flags;
}

void pic_remap() {
    outb(0x20, 0x11); 
    outb(0xA0, 0x11);
    
    outb(0x21, 0x20);
    outb(0xA1, 0x28); 
    
    outb(0x21, 0x04); 
    outb(0xA1, 0x02); 
    
    outb(0x21, 0x01);
    outb(0xA1, 0x01);
    
    outb(0x21, 0xF8); 
    outb(0xA1, 0x3F); 
}

uint32_t timer_handler(struct registers *regs) {
    task_list[current_task].esp = (uint32_t)regs; 

    int next_task = current_task;
    while (1) {
        next_task = (next_task + 1) % 3;
        
        if (next_task == 0 || task_list[next_task].is_active == 1) {
            break;
        }
    }

    current_task = next_task;
    timer_ticks++;
    outb(0x20, 0x20); 

    return task_list[current_task].esp;
}

void keyboard_handler() {
    outb(0x20, 0x20); 
}

void stub_mouse_handler() {
    outb(0xA0, 0x20); 
    outb(0x20, 0x20); 
}

void delay_ticks(uint32_t ticks) {
    uint32_t target_ticks = timer_ticks + ticks;
    while (timer_ticks < target_ticks) {
        __asm__ __volatile__("hlt");
    }
}

void play_error_sound() {
    beep(150, 10); 
    delay_ticks(2);     
    beep(150, 10);
}

void exception_handler(struct registers *regs) {
    draw_rect(0, 0, 1024, 768, 1); 

    play_error_sound();

    x = 10;
    y = 10;
    
    print("KERNEL FATAL: ", 15);
    
    if (regs->int_no == 0) print("DIVISION BY ZERO", 15);
    else if (regs->int_no == 1) print("DEBUG", 15);
    else if (regs->int_no == 2) print("NON MASKABLE INTERRUPT", 15);
    else if (regs->int_no == 3) print("BREAKPOINT", 15);
    else if (regs->int_no == 4) print("INTO DETECTED OVERFLOW", 15);
    else if (regs->int_no == 5) print("OUT OF BOUNDS", 15);
    else if (regs->int_no == 6) print("INVALID OPCODE", 15);
    else if (regs->int_no == 7) print("NO COPROCESSOR", 15);
    else if (regs->int_no == 8) print("DOUBLE FAULT", 15);
    else if (regs->int_no == 9) print("COPROCESSOR SEGMENT OVERRUN", 15);
    else if (regs->int_no == 10) print("BAD TSS", 15);
    else if (regs->int_no == 11) print("SEGMENT NOT PRESENT", 15);
    else if (regs->int_no == 12) print("STACK FAULT", 15);
    else if (regs->int_no == 13) print("GENERAL PROTECTION FAULT", 15);
    else if (regs->int_no == 14) print("PAGE FAULT", 15);
    else if (regs->int_no == 15) print("UNKNOWN INTERRUPT (RESERVED)", 15);
    else if (regs->int_no == 16) print("FLOATING POINT ERROR", 15);
    else if (regs->int_no == 17) print("ALIGNMENT CHECK", 15);
    else if (regs->int_no == 18) print("MACHINE CHECK", 15);
    else if (regs->int_no == 19) print("SIMD FLOATING POINT EXCEPTION", 15);
    else if (regs->int_no == 20) print("VIRTUALIZATION EXCEPTION", 15);
    else if (regs->int_no == 21) print("CONTROL PROTECTION EXCEPTION", 15);
    else if (regs->int_no >= 22 && regs->int_no <= 27) print("RESERVED EXCEPTION", 15);
    else if (regs->int_no == 28) print("HYPERVISOR INJECTION EXCEPTION", 15);
    else if (regs->int_no == 29) print("VMM COMMUNICATION EXCEPTION", 15);
    else if (regs->int_no == 30) print("SECURITY EXCEPTION", 15);
    else if (regs->int_no == 31) print("RESERVED EXCEPTION", 15);
    else print("UNKNOWN EXCEPTION\n", 15);

    x = 10;
    y = 70;
    print("TECHICAL INFORMATION: ", 15);
    char buf[16];
    itoa(regs->int_no, buf);
    print("\n  INTERRUPT NO: ", 15);
    print(buf, 15); 


    print("\n  EIP: ", 15);
    char eip_buf[32];
    htoa(regs->eip, eip_buf);
    print(eip_buf, 15);

    print("\n  CS: ", 15);
    htoa(regs->cs, buf); 
    print(buf, 15);

    print("\n  ERR CODE: ", 15);
    itoa(regs->err_code, buf); 
    print(buf, 15);
    
    while(1); 
}

void init_timer() {
    outb(0x43, 0x36); 

    outb(0x40, 0x52); 
    outb(0x40, 0x09);
}

void ata_handler() {
    ata_interrupt_received = 1;

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
}

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

unsigned int task3_stack[1024]; 

void task3_main() {
    while(1) {
        interpret(stack_init(1024), content);
    }
}

void prepare_task3() {
    uint32_t* st = &task3_stack[1024];

    *(--st) = 0x202;    
    *(--st) = 0x08;     
    *(--st) = (uint32_t)task3_main; 

    *(--st) = 0;          
    *(--st) = 0;           

    for (int i = 0; i < 8; i++) {
        *(--st) = 0;
    }

    *(--st) = 0x10;   

    task_list[2].esp = (uint32_t)st;
}

void create_task(int task_id) {
    if (task_id < 1 || task_id > 2) return; 

    __asm__ __volatile__("cli"); 

    if (task_id == 1) {
        prepare_task2();
    } else if (task_id == 2) {
        prepare_task3();
    }

    task_list[task_id].id = task_id;
    task_list[task_id].is_active = 1; 

    __asm__ __volatile__("sti");
}

void delete_task(int task_id) {
    if (task_id < 1 || task_id > 2) return;

    __asm__ __volatile__("cli");

    task_list[task_id].is_active = 0;

    if (current_task == task_id) {
        __asm__ __volatile__("sti");
        while(1) {
            __asm__ __volatile__("hlt"); 
        }
    }

    __asm__ __volatile__("sti");
}

void init_idt() {
    idtp.idt_ptr = (uint32_t)&idt_entry;
    idtp.idt_size = (sizeof(struct idt_entry) * 256) - 1;

    for (int i = 0; i < 256; i++) {
        set_idt_gate(i, 0, 0, 0);
    }

    pic_remap();
    init_timer();

    set_idt_gate(0,  (uint32_t)isr0,  0x08, 0x8E);
    set_idt_gate(8,  (uint32_t)isr8,  0x08, 0x8E);
    set_idt_gate(13, (uint32_t)isr13, 0x08, 0x8E);
    set_idt_gate(14, (uint32_t)isr14, 0x08, 0x8E);

    set_idt_gate(32, (uint32_t)timer_wrapper, 0x08, 0x8E);
    set_idt_gate(33, (uint32_t)keyboard_wrapper, 0x08, 0x8E);
    set_idt_gate(44, (uint32_t)mouse_wrapper, 0x08, 0x8E);
    set_idt_gate(46, (uint32_t)ata_wrapper, 0x08, 0x8E);

    prepare_task2();
    prepare_task3();

    current_task = 0; 
    task_list[0].id = 0;

    task_list[0].is_active = 1; 

    task_list[1].is_active = 1;
    task_list[2].is_active = 0;

    __asm__ __volatile__("lidt (%0)" : : "r" (&idtp));
    __asm__ __volatile__("sti");
}