#define SYS_EXIT  1
#define SYS_READ  3
#define SYS_WRITE 4

typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

void print(char *msg, uint8_t color) {
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_WRITE), "b"(1), "c"((uint32_t)msg), "d"((uint32_t)color)
        : "memory"
    );
}

uint8_t get_char() {
    uint32_t scancode;
    __asm__ __volatile__(
        "int $0x80"
        : "=a"(scancode)
        : "a"(SYS_READ), "b"(0)
        : "memory"
    );
    return (uint8_t)scancode;
}

void exit() {
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(SYS_EXIT)
    );
    while(1); 
}

void _start() {
    print("SYS_WRITE: Enter any key...\n", 14); 
    
    uint8_t code = get_char(); 
    
    print("\nKey received! Exiting...\n", 10); 
    
    exit();
}