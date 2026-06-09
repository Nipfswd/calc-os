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
void exception_handler(struct registers *regs);

void create_task(int task_id);
void delete_task(int task_id);

extern void timer_wrapper();
extern void keyboard_wrapper();
extern void mouse_wrapper();
extern void ata_wrapper();

extern void isr0();
extern void isr8();
extern void isr13();
extern void isr14();
void delay_ticks(uint32_t ticks);
extern void syscall_wrapper();

extern unsigned int timer_ticks;
extern uint8_t *timer_str[16];

extern volatile int ata_interrupt_received;

#define SYS_EXIT    1
#define SYS_READ    3
#define SYS_WRITE   4
#define SYS_OPEN    5
#define SYS_TIME    13
#define SYS_GETPID  20
#define SYS_UNAME   122

uint32_t syscall_handler(struct registers *regs);

extern unsigned int task2_stack[1024];
extern unsigned int task3_stack[1024];
extern unsigned int task4_stack[1024];

static inline uint32_t _syscall(uint32_t num, uint32_t a1, uint32_t a2, uint32_t a3) {
    uint32_t ret;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(ret)
        : "a"(num), "b"(a1), "c"(a2), "d"(a3)
        : "memory"
    );
    return ret;
}

int sys_exec(const char* filename_11);
void sys_exit(void);
uint8_t sys_read(void);
uint16_t sys_open(const char* filename_11);
int sys_write(int fd, const char* str, uint8_t color);


#endif