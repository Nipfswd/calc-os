#define SYS_EXIT  0
#define SYS_WRITE 4

static inline int _syscall(int num, unsigned int arg1, unsigned int arg2, unsigned int arg3) {
    int ret;
    __asm__ __volatile__ (
        "int $0x80"
        : "=a" (ret)
        : "a" (num), "b" (arg1), "c" (arg2), "d" (arg3)
        : "memory"
    );
    return ret;
}

int sys_write(int fd, const char* str, unsigned char color) {
    return (int)_syscall(SYS_WRITE, (unsigned int)fd, (unsigned int)str, (unsigned int)color);
}

void sys_exit(void) {
    _syscall(SYS_EXIT, 0, 0, 0);
}

void print(char *msg, unsigned char color) {
    sys_write(1, msg, color);
}

void _start() {
    print("Hello, World!\n", 15);
    sys_exit();
}