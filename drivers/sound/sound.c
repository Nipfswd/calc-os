#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>

void play_sound(uint32_t frequency) {
    if (frequency == 0) return;

    uint32_t division = 1193182 / frequency;
    outb(0x43, 0xB6);
    outb(0x42, (uint8_t)(division & 0xFF));
    outb(0x42, (uint8_t)((division >> 8) & 0xFF));

    uint8_t speaker_state = inb(0x61);
    if (speaker_state != (speaker_state | 3)) {
        outb(0x61, speaker_state | 3);
    }
}

void stop_sound() {
    uint8_t speaker_state = inb(0x61) & 0xFC;
    outb(0x61, speaker_state);
}

void beep(uint32_t frequency, uint32_t ticks) {
    play_sound(frequency);
    delay_ticks(ticks);
    stop_sound();
}