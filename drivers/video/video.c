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
int is_button_ethernet = 0;
int is_window_send = 0;

void screen_clear() {
    for (int i = 0; i < SCREEN_WIDTH * SCREEN_HEIGHT; i++) {
        VIDEO_MEMORY[i] = 0;
    }
    x = 0;
    y = 0;

    ncount = 1;
    is_scaled = 0;
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

    set_palette_color(3, 0, 170, 170);     
    set_palette_color(5, 170, 0, 170);     
    set_palette_color(6, 170, 85, 0);       
    set_palette_color(9, 85, 85, 255);     
    set_palette_color(10, 85, 255, 85);    
    set_palette_color(11, 85, 255, 255);   
    set_palette_color(12, 255, 85, 85);     
    set_palette_color(13, 255, 85, 255);    

    set_palette_color(16, 212, 208, 200); 
    set_palette_color(17, 10, 24, 80);      
    set_palette_color(18, 128, 128, 128); 
    set_palette_color(19, 230, 230, 230);
    
    set_palette_color(20, 0, 120, 215);    
    set_palette_color(21, 26, 26, 26);     
    set_palette_color(22, 255, 165, 0);    
}

void put_char(char s, uint8_t color) {
    int scale = 1;
    if (is_scaled == 1) scale = 3;
    else if (is_scaled == 2) scale = 2;

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

static const uint8_t fade_palette[24] = {
    15, 15, 19, 19, 19, 19, 7,  7, 
    7,  7,  18, 18, 18, 8,  8,  8, 
    20, 20, 20, 20, 20, 20, 20, 20
};

void draw_desktop() {
    draw_rect(0, 40, 1024, 728, 20);

    int glass_x = 60;
    int glass_y = 180;
    int glass_w = 500;
    int glass_h = 320;

    draw_rect(glass_x, glass_y, glass_w, 2, 15);
    draw_rect(glass_x, glass_y, 2, glass_h, 15);
    draw_rect(glass_x, glass_y + glass_h, glass_w, 2, 18);
    draw_rect(glass_x + glass_w, glass_y, 2, glass_h, 18);

    draw_rect(glass_x + 6, glass_y + glass_h + 6, glass_w, 4, 0); 
    draw_rect(glass_x + glass_w + 6, glass_y + 6, 4, glass_h, 0);

    char *text = "CalcOS";
    int text_x = glass_x + 40;
    int text_y = glass_y + 80;
    int scale = 3;

    is_scaled = 1;
    x = text_x + 3; y = text_y + 3;
    print(text, 0);

    x = text_x; y = text_y;
    print(text, 15);
    is_scaled = 0;

    int reflect_y = text_y + 24 + 2; 

    asm volatile("cli");

    for (int char_idx = 0; text[char_idx] != 0; char_idx++) {
        unsigned char s = (unsigned char)text[char_idx];
        int start_x = text_x + (char_idx * 8 * scale);

        for (int i = 0; i < 8; i++) {
            unsigned char bits = font[s][7 - i];

            for (int v_scale = 0; v_scale < scale; v_scale++) {
                int current_pixel_y = reflect_y + (i * scale) + v_scale;
                
                if (current_pixel_y >= SCREEN_HEIGHT || current_pixel_y >= (glass_y + glass_h)) {
                    continue;
                }

                int fade_index = (i * scale) + v_scale;
                uint8_t reflect_color = (fade_index < 24) ? fade_palette[fade_index] : 20;

                uint8_t *row = &VIDEO_MEMORY[current_pixel_y * SCREEN_WIDTH];
                unsigned char bits_copy = bits;

                for (int j = 0; j < 8; j++) {
                    if (bits_copy & 0x80) {
                        for (int h_scale = 0; h_scale < scale; h_scale++) {
                            int current_pixel_x = start_x + (j * scale) + h_scale;
                            
                            if (current_pixel_x >= 0 && current_pixel_x < SCREEN_WIDTH) {
                                row[current_pixel_x] = reflect_color;
                            }
                        }
                    }
                    bits_copy <<= 1;
                }
            }
        }
    }

    asm volatile("sti");
}