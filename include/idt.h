#ifndef IDT_H
#define IDT_H
#include <stdint.h>

struct idt_entry {
    uint16_t base_low;
    uint16_t selector;      
    uint8_t  always0;  
    uint8_t  flags;  
    uint16_t base_high; 
} __attribute__((packed));

struct idt_pointer {
    uint16_t idt_size;
    uint32_t idt_ptr;
} __attribute__((packed));

struct registers {
    uint32_t ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t int_no, err_code; 
    uint32_t eip, cs, eflags, useresp, ss;
};

void init_idt();
void set_idt_gate(uint8_t number, uint32_t base, uint16_t selector, uint8_t flags);
void pic_remap();
void exception_handler(struct registers regs);

extern void timer_wrapper();
extern void keyboard_wrapper();
extern void mouse_wrapper();
extern void ata_wrapper();

extern void isr0();
extern void isr8();
extern void isr13();
extern void isr14();
void delay_ticks(uint32_t ticks);

extern unsigned int timer_ticks;
extern uint8_t *timer_str[16];

extern volatile int ata_interrupt_received;

#endif