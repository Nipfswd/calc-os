#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>

struct idt_entry   idt_entry[256];
struct idt_pointer idtp;

unsigned int timer_ticks = 0;
unsigned char *timer_str[16];

void set_idt_gate(unsigned char number, unsigned int base, unsigned short selector, unsigned char flags) {
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
    
    outb(0x21, 0xFC);
    outb(0xA1, 0xFF);
}

unsigned int task2_stack[1024]; 

void task2_main() {
    int hours, minutes;
    char h_str[3], m_str[3];

    while(1) {
        if (current_mode != 0) {
            get_time(&hours, &minutes);

            x = 600; y = 15;
            
            itoa(hours, h_str);
            print(h_str, 1);
            print(":", 1);
            
            itoa(minutes, m_str);
            if (minutes < 10) print("0", 1); 
            print(m_str, 1);

            draw_rect(500, 0, 140, 40, 0); 
        }
    }
}

void prepare_task2() {
    unsigned int* st = &task2_stack[1024];

    *(--st) = 0x202;       
    *(--st) = 0x08;    
    *(--st) = (unsigned int)task2_main; 

    for (int i = 0; i < 8; i++) {
        *(--st) = 0;
    }

    task_list[1].esp = (unsigned int)st;
}

unsigned int timer_handler(unsigned int current_esp) {
    task_list[current_task].esp = current_esp;

    current_task = (current_task + 1) % 2;

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

void exception_handler(struct registers regs) {
    draw_rect(0, 0, 640, 480, 1); 

    x = 10;
    y = 10;
    
    print("KERNEL FATAL: ", 0);
    
    if (regs.int_no == 0) print("DIVISION BY ZERO", 0);
    else if (regs.int_no == 1) print("DEBUG", 0);
    else if (regs.int_no == 2) print("NON MASKABLE INTERRUPT", 0);
    else if (regs.int_no == 3) print("BREAKPOINT", 0);
    else if (regs.int_no == 4) print("INTO DETECTED OVERFLOW", 0);
    else if (regs.int_no == 5) print("OUT OF BOUNDS", 0);
    else if (regs.int_no == 6) print("INVALID OPCODE", 0);
    else if (regs.int_no == 7) print("NO COPROCESSOR", 0);
    else if (regs.int_no == 8) print("DOUBLE FAULT", 0);
    else if (regs.int_no == 9) print("COPROCESSOR SEGMENT OVERRUN", 0);
    else if (regs.int_no == 10) print("BAD TSS", 0);
    else if (regs.int_no == 11) print("SEGMENT NOT PRESENT", 0);
    else if (regs.int_no == 12) print("STACK FAULT", 0);
    else if (regs.int_no == 13) print("GENERAL PROTECTION FAULT", 0);
    else if (regs.int_no == 14) print("PAGE FAULT", 0);
    else if (regs.int_no == 15) print("UNKNOWN INTERRUPT (RESERVED)", 0);
    else if (regs.int_no == 16) print("FLOATING POINT ERROR", 0);
    else if (regs.int_no == 17) print("ALIGNMENT CHECK", 0);
    else if (regs.int_no == 18) print("MACHINE CHECK", 0);
    else if (regs.int_no == 19) print("SIMD FLOATING POINT EXCEPTION", 0);
    else if (regs.int_no == 20) print("VIRTUALIZATION EXCEPTION", 0);
    else if (regs.int_no == 21) print("CONTROL PROTECTION EXCEPTION", 0);
    else if (regs.int_no >= 22 && regs.int_no <= 27) print("RESERVED EXCEPTION", 0);
    else if (regs.int_no == 28) print("HYPERVISOR INJECTION EXCEPTION", 0);
    else if (regs.int_no == 29) print("VMM COMMUNICATION EXCEPTION", 0);
    else if (regs.int_no == 30) print("SECURITY EXCEPTION", 0);
    else if (regs.int_no == 31) print("RESERVED EXCEPTION", 0);
    else print("UNKNOWN EXCEPTION\n", 0);

    x = 10;
    y = 70;
    print("TECHICAL INFORMATION: ", 0);
    char buf[3];
    itoa(regs.int_no, buf);
    print("\nINTERRUPT NO: ", 0); 
    print(buf, 0); 


    print("\nEIP: ", 0);
    char eip_buf[10];
    htoa(regs.eip, eip_buf);
    print(eip_buf, 0);

    print("\nCS: ", 0);
    htoa(regs.cs, buf); 
    print(buf, 0);

    print("\nERR CODE: ", 0);
    itoa(regs.err_code, buf); 
    print(buf, 0);
    
    while(1); 
}

void init_timer() {
    outb(0x43, 0x36); 

    outb(0x40, 0xFF); 
    outb(0x40, 0xFF); 
}

void init_idt() {
    idtp.idt_ptr = (unsigned int)&idt_entry;
    idtp.idt_size = (sizeof(struct idt_entry) * 256) - 1;

    for (int i = 0; i < 256; i++) {
        set_idt_gate(i, 0, 0, 0);
    }

    pic_remap();
    init_timer();

    set_idt_gate(0,  (unsigned int)isr0,  0x08, 0x8E);
    set_idt_gate(8,  (unsigned int)isr8,  0x08, 0x8E);
    set_idt_gate(13, (unsigned int)isr13, 0x08, 0x8E);
    set_idt_gate(14, (unsigned int)isr14, 0x08, 0x8E);

    set_idt_gate(32, (unsigned int)timer_wrapper, 0x08, 0x8E);
    set_idt_gate(33, (unsigned int)keyboard_wrapper, 0x08, 0x8E);
    set_idt_gate(44, (unsigned int)mouse_wrapper, 0x08, 0x8E);

    prepare_task2();

    current_task = 0; 
    task_list[0].id = 0;

    __asm__ __volatile__("lidt (%0)" : : "r" (&idtp));
    __asm__ __volatile__("sti");
}