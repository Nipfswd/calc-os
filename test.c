typedef unsigned int uint32_t;
typedef unsigned char uint8_t;

void print(char *msg, uint8_t color) {
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(4), "b"(1), "c"((uint32_t)msg), "d"((uint32_t)color)
        : "memory"
    );
}

void exit() {
    __asm__ __volatile__(
        "int $0x80"
        :
        : "a"(1)
    );
    while(1); 
}

void _start() {
    print("SYS_WRITE OK\n", 15); 
    
    exit();
}