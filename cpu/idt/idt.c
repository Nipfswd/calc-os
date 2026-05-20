#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>

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
    outb(0xA1, 0x00);
}

unsigned int task2_stack[1024]; 

void task2_main() {
    int hours, minutes;
    char h_str[3], m_str[3];

    while(1) {
        if (current_mode != 0 && is_window_crt == 0) {
            get_time(&hours, &minutes);

            x = 600; y = 15;
            
            itoa(hours, h_str);
            print(h_str, 1);
            print(":", 1);
            
            itoa(minutes, m_str);
            if (minutes < 10) print("0", 1); 
            print(m_str, 1);

            draw_rect(602, 5, 38, 30, 0); 
        }
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

uint32_t timer_handler(uint32_t current_esp) {
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
    draw_rect(0, 0, 640, 480, 1); 

    play_error_sound();

    x = 10;
    y = 10;
    
    print("KERNEL FATAL: ", 0);
    
    if (regs->int_no == 0) print("DIVISION BY ZERO", 0);
    else if (regs->int_no == 1) print("DEBUG", 0);
    else if (regs->int_no == 2) print("NON MASKABLE INTERRUPT", 0);
    else if (regs->int_no == 3) print("BREAKPOINT", 0);
    else if (regs->int_no == 4) print("INTO DETECTED OVERFLOW", 0);
    else if (regs->int_no == 5) print("OUT OF BOUNDS", 0);
    else if (regs->int_no == 6) print("INVALID OPCODE", 0);
    else if (regs->int_no == 7) print("NO COPROCESSOR", 0);
    else if (regs->int_no == 8) print("DOUBLE FAULT", 0);
    else if (regs->int_no == 9) print("COPROCESSOR SEGMENT OVERRUN", 0);
    else if (regs->int_no == 10) print("BAD TSS", 0);
    else if (regs->int_no == 11) print("SEGMENT NOT PRESENT", 0);
    else if (regs->int_no == 12) print("STACK FAULT", 0);
    else if (regs->int_no == 13) print("GENERAL PROTECTION FAULT", 0);
    else if (regs->int_no == 14) print("PAGE FAULT", 0);
    else if (regs->int_no == 15) print("UNKNOWN INTERRUPT (RESERVED)", 0);
    else if (regs->int_no == 16) print("FLOATING POINT ERROR", 0);
    else if (regs->int_no == 17) print("ALIGNMENT CHECK", 0);
    else if (regs->int_no == 18) print("MACHINE CHECK", 0);
    else if (regs->int_no == 19) print("SIMD FLOATING POINT EXCEPTION", 0);
    else if (regs->int_no == 20) print("VIRTUALIZATION EXCEPTION", 0);
    else if (regs->int_no == 21) print("CONTROL PROTECTION EXCEPTION", 0);
    else if (regs->int_no >= 22 && regs->int_no <= 27) print("RESERVED EXCEPTION", 0);
    else if (regs->int_no == 28) print("HYPERVISOR INJECTION EXCEPTION", 0);
    else if (regs->int_no == 29) print("VMM COMMUNICATION EXCEPTION", 0);
    else if (regs->int_no == 30) print("SECURITY EXCEPTION", 0);
    else if (regs->int_no == 31) print("RESERVED EXCEPTION", 0);
    else print("UNKNOWN EXCEPTION\n", 0);

    x = 10;
    y = 70;
    print("TECHICAL INFORMATION: ", 0);
    char buf[16];
    itoa(regs->int_no, buf);
    print("\n  INTERRUPT NO: ", 0); 
    print(buf, 0); 


    print("\n  EIP: ", 0);
    char eip_buf[32];
    htoa(regs->eip, eip_buf);
    print(eip_buf, 0);

    print("\n  CS: ", 0);
    htoa(regs->cs, buf); 
    print(buf, 0);

    print("\n  ERR CODE: ", 0);
    itoa(regs->err_code, buf); 
    print(buf, 0);
    
    while(1); 
}

void init_timer() {
    outb(0x43, 0x36); 

    outb(0x40, 0xFF); 
    outb(0x40, 0xFF); 
}

void ata_handler() {
    ata_interrupt_received = 1;

    outb(0xA0, 0x20);
    outb(0x20, 0x20);
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

    current_task = 0; 
    task_list[0].id = 0;

    __asm__ __volatile__("lidt (%0)" : : "r" (&idtp));
    __asm__ __volatile__("sti");
}