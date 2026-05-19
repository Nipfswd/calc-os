#ifndef SOUND_H
#define SOUND_H

#include <stdint.h>
void play_sound(uint32_t frequency);
void stop_sound();
void beep(uint32_t frequency, uint32_t ticks);
void play_note(uint32_t base_frequency, uint32_t duration_ms);

#endif 