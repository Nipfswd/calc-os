#ifndef IDT_H
#define IDT_H

struct idt_entry {
    unsigned short base_low;
    unsigned short selector;      
    unsigned char  always0;  
    unsigned char  flags;  
    unsigned short base_high; 
} __attribute__((packed));

struct idt_pointer {
    unsigned short idt_size;
    unsigned int   idt_ptr;
} __attribute__((packed));

struct registers {
    unsigned int ds;
    unsigned int edi, esi, ebp, esp, ebx, edx, ecx, eax;
    unsigned int int_no, err_code; 
    unsigned int eip, cs, eflags, useresp, ss;
};

void init_idt();
void set_idt_gate(unsigned char number, unsigned int base, unsigned short selector, unsigned char flags);
void pic_remap();
void exception_handler(struct registers regs);

extern void timer_wrapper();
extern void keyboard_wrapper();
extern void mouse_wrapper();

extern void isr0();
extern void isr8();
extern void isr13();
extern void isr14();

extern unsigned int timer_ticks;
extern unsigned char *timer_str[16];

#endif