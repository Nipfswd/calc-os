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

static inline uint8_t sys_read(void) {
    return (uint8_t)_syscall3(SYS_READ, 0, 0, 0);
}