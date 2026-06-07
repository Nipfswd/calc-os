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

static inline uint16_t sys_open(const char* filename_11) {
    return (uint16_t)_syscall3(SYS_OPEN, (uint32_t)filename_11, 0, 0);
}