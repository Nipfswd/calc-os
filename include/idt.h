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

typedef struct {
    uint16_t cluster;
    uint32_t offset;
    uint8_t used;
} file_descriptor_t;

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
#define SYS_CLOSE   6

#define EPERM            1
#define ENOENT           2
#define ESRCH            3
#define EINTR            4
#define EIO              5
#define ENXIO            6
#define E2BIG            7
#define ENOEXEC          8
#define EBADF            9
#define ECHILD          10
#define EAGAIN          11
#define ENOMEM          12
#define EACCES          13
#define EFAULT          14
#define ENOTBLK         15
#define EBUSY           16
#define EEXIST          17
#define EXDEV           18
#define ENODEV          19
#define ENOTDIR         20
#define EISDIR          21
#define EINVAL          22
#define ENFILE          23
#define EMFILE          24
#define ENOTTY          25
#define ETXTBSY         26
#define EFBIG           27
#define ENOSPC          28
#define ESPIPE          29
#define EROFS           30
#define EMLINK          31
#define EPIPE           32
#define EDOM            33
#define ERANGE          34

#define ENOTSOCK        88
#define EDESTADDRREQ    89
#define EMSGSIZE        90
#define EPROTOTYPE      91
#define ENOPROTOOPT     92
#define EPROTONOSUPPORT 93
#define ESOCKTNOSUPPORT 94
#define EOPNOTSUPP      95
#define EPFNOSUPPORT    96
#define EAFNOSUPPORT    97
#define EADDRINUSE      98
#define EADDRNOTAVAIL   99
#define ENETDOWN        100
#define ENETUNREACH     101
#define ECONNRESET      104
#define EISCONN         106
#define ENOTCONN        107
#define ETIMEDOUT       110
#define ECONNREFUSED    111

#define ENOSYS          38

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

int sys_close(int fd);
void sys_exit(void);
int sys_getpid(void);
int sys_open(const char* path);
int sys_read(int fd, char* buf, uint32_t count);
uint32_t sys_time(void);
void sys_uname(char *buffer);
int sys_write(int fd, const char* str, uint8_t color);

#endif