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

static inline int sys_exec(const char* filename_11) {
    return (int)_syscall3(SYS_EXEC, (uint32_t)filename_11, 0, 0);
}