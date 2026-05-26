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

void screen_clear() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VIDEO_MEMORY[i] = 0;
    }
    x = 0;
    y = 0;
}

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    outb(0x03C8, index);    
    outb(0x03C9, r >> 2);   
    outb(0x03C9, g >> 2);    
    outb(0x03C9, b >> 2);   
}

void init_palette() {
    set_palette_color(0, 0, 0, 0);
    
    set_palette_color(1, 0, 0, 170);
    
    set_palette_color(2, 0, 170, 0);
    
    set_palette_color(4, 170, 0, 0);

    set_palette_color(7, 170, 170, 170);
    
    set_palette_color(8, 85, 85, 85);

    set_palette_color(14, 255, 255, 85);

    set_palette_color(15, 255, 255, 255);
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

        if (x + (8 * scale) > SCREEN_WIDTH) {
            x = 0;
            y = y + (8 * scale);
        }

        if (y + (8 * scale) > SCREEN_HEIGHT) {
            screen_clear();
            y = 0;
        }

        for (int i = 0; i < 8; i++) {
            for (int v_scale = 0; v_scale < scale; v_scale++) {
                unsigned char bits = font[(int)s][i];
                
                int current_y = y + (i * scale) + v_scale;
                uint8_t *row = &VIDEO_MEMORY[current_y * SCREEN_WIDTH];

                for (int j = 0; j < 8; j++) {
                    if (bits > 127) { 
                        for (int h_scale = 0; h_scale < scale; h_scale++) {
                            int current_x = x + (j * scale) + h_scale;
                            if (current_x < SCREEN_WIDTH) {
                                row[current_x] = color; 
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
            asm volatile("sti"); 
            return;
        }

        if (x + 8 > SCREEN_WIDTH) {
            x = 0;
            y = y + 8;
        }

        if (y + 8 > SCREEN_HEIGHT) {
            screen_clear();
            y = 0;
        }

        for (int i = 0; i < 8; i++) {
            unsigned char bits = font[(int)s][i];
            
            uint8_t *row = &VIDEO_MEMORY[(y + i) * SCREEN_WIDTH];

            for (int j = 0; j < 8; j++) {
                if (bits > 127) {
                    row[x + j] = color; 
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

void draw_rect(int x, int y, int width, int height, uint8_t color) {
    asm volatile("cli");
    
    for (int i = 0; i < height; i = i + 1) {
        for (int j = 0; j < width; j = j + 1) {
            int curr_x = x + j;
            int curr_y = y + i;

            if (curr_x >= 0 && curr_x < SCREEN_WIDTH && curr_y >= 0 && curr_y < SCREEN_HEIGHT) {
                VIDEO_MEMORY[curr_y * SCREEN_WIDTH + curr_x] = color;
            }
        }
    }
    
    asm volatile("sti");
}