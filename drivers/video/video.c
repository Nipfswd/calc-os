#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <font.h>
#include <stdint.h>

int x = 0;
int y = 0;
int ncount = 0;
int current_mode = 0;
int show_crt_window = 0;
int is_window_crt = 0;
int is_button_apps = 0;
int is_button_files = 1;
int is_button_calc = 0;
int draw_0 = 1;
int draw_1 = 1;
int is_scaled = 0;

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x03C8, index);
    outb(0x03C9, r);
    outb(0x03C9, g);
    outb(0x03C9, b);
}

void init_palette() {
    inb(0x03DA); 

    
    outb(0x03C0, 0); 
    outb(0x03C0, 0);
    
    for (uint8_t i = 1; i < 16; i++) {
        outb(0x03C0, i); 
        outb(0x03C0, 1);
    }

    outb(0x03C0, 0x20);

    set_palette_color(0, 10, 11, 10);

    for (uint8_t i = 1; i < 16; i++) {
        set_palette_color(i, 62, 48, 26);
    }
}


void screen_clear() {
	for (int i = 0; i < 38400; i = i + 1) {
		VIDEO_MEMORY[i] = 0;
	}

	x = 0;
	y = 0;
	ncount = 1;
}

void put_char(char s, uint8_t color) {
    if (is_scaled == 1) {
        asm volatile("cli");
    
        int scale = 3; 

        if (s == '\n') {
            x = 0;
            y = y + (8 * scale);
            asm volatile("sti");
            return;
        }

        if (x + (8 * scale) > 640) {
            x = 0;
            y = y + (8 * scale);
        }

        if (y + (8 * scale) > 480) {
            screen_clear();
            y = 0;
        }

        for (int i = 0; i < 8; i++) {
            
            for (int v_scale = 0; v_scale < scale; v_scale++) {
                
                unsigned char bits = font[(int)s][i];
                unsigned char *row = &VIDEO_MEMORY[(y + (i * scale) + v_scale) * 80];

                for (int j = 0; j < 8; j++) {
                    for (int h_scale = 0; h_scale < scale; h_scale++) {
                        
                        int pos = x + (j * scale) + h_scale;
                        unsigned char mask = 128 >> (pos & 7);

                        if (bits > 127) { 
                            if (color > 0) {
                                row[pos >> 3] |= mask;
                            } else {
                                row[pos >> 3] &= ~mask;
                            }
                        }
                    }
                    bits = bits << 1; 
                }
            }
        }
        
        x = x + (8 * scale);
        
        asm volatile("sti");
    } else {
        asm volatile("cli");
        if (s == '\n') {
            x = 0;
            y = y + 8;
            return;
        }

        if (x >= 640) {
            x = 0;
            y = y + 8;
        }

        if (y >= 480) {
            screen_clear();
            y = 0;
        }

        for (int i = 0; i < 8; i = i + 1) {
            unsigned char bits = font[(int)s][i];
            unsigned char *row = &VIDEO_MEMORY[(y + i) * 80];

            for (int j = 0; j < 8; j = j + 1) {
                int pos = x + j;
                unsigned char mask = 128 >> (pos & 7);

                if (bits > 127) {
                    if (color > 0) {
                        row[pos >> 3] = row[pos >> 3] | mask;
                    } else {
                        row[pos >> 3] = row[pos >> 3] & ~mask;
                    }
                }
                bits = bits << 1;
            }
        }
        x = x + 8;
        asm volatile("sti");
    }
}

void print(char *msg, uint8_t color) {
	for (int i = 0; msg[i] != 0; i++) {
		put_char(msg[i], color);
	}
}

void draw_gray_rect(int x, int y, int width, int height) {
    asm volatile("cli");
    for (int i = 0; i < height; i = i + 1) {
        for (int j = 0; j < width; j = j + 1) {
            int curr_x = x + j;
            int curr_y = y + i;

            if (curr_x >= 0 && curr_x < 640 && curr_y >= 0 && curr_y < 480) {
                int is_white = (curr_x + curr_y) % 2 == 0;
                
                uint8_t *row = &VIDEO_MEMORY[curr_y * 80];
                uint8_t mask = 128 >> (curr_x & 7);

                if (is_white) {
                    row[curr_x >> 3] = row[curr_x >> 3] | mask;
                } else {
                    row[curr_x >> 3] = row[curr_x >> 3] & ~mask;
                }
            }
        }
    }
    asm volatile("sti");
}

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    asm volatile("cli");
    if (color == 2) {
        draw_gray_rect(x, y, width, height);
        return;
    }

    for (int i = 0; i < height; i = i + 1) {
        for (int j = 0; j < width; j = j + 1) {
            int curr_x = x + j;
            int curr_y = y + i;

            if (curr_x >= 0 && curr_x < 640 && curr_y >= 0 && curr_y < 480) {
                uint8_t *ptr = &VIDEO_MEMORY[curr_y * 80 + (curr_x >> 3)];
                uint8_t mask = 128 >> (curr_x & 7);

                if (color > 0) {
                    *ptr = *ptr | mask;   
                } else {
                    *ptr = *ptr & ~mask;   
                }
            }
        }
    }
    asm volatile("sti");
}