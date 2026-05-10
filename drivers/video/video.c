#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <font.h>

int x = 0;
int y = 0;
int ncount = 0;
int current_mode = 0;
int show_crt_window = 0;
int is_window_crt = 0;
int is_button_apps = 0;
int is_button_files = 1;

void set_palette_color(unsigned char index, unsigned char r, unsigned char g, unsigned char b) {
    outb(0x03C8, index);
    outb(0x03C9, r);
    outb(0x03C9, g);
    outb(0x03C9, b);
}

void init_palette() {
    set_palette_color(0, 8, 10, 12);
    set_palette_color(1, 45, 50, 58);
}


void screen_clear() {
	for (int i = 0; i < 64000; i = i + 1) {
		VIDEO_MEMORY[i] = 0;
	}

	x = 0;
	y = 0;
	ncount = 1;
}

void put_char(char s, unsigned char color) {
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
            unsigned char mask = 128 >> (pos % 8);

            if (bits > 127) {
                if (color > 0) {
                    row[pos / 8] = row[pos / 8] | mask;
                } else {
                    row[pos / 8] = row[pos / 8] & ~mask;
                }
            }
            bits = bits << 1;
        }
    }
    x = x + 8;
}

void print(char *msg, unsigned char color) {
	for (int i = 0; msg[i] != 0; i++) {
		put_char(msg[i], color);
	}
}

void draw_gray_rect(int x, int y, int width, int height) {
    for (int i = 0; i < height; i = i + 1) {
        for (int j = 0; j < width; j = j + 1) {
            int curr_x = x + j;
            int curr_y = y + i;

            if (curr_x >= 0 && curr_x < 640 && curr_y >= 0 && curr_y < 480) {
                int is_white = (curr_x + curr_y) % 2 == 0;
                
                unsigned char *row = &VIDEO_MEMORY[curr_y * 80];
                unsigned char mask = 128 >> (curr_x % 8);

                if (is_white) {
                    row[curr_x / 8] = row[curr_x / 8] | mask;
                } else {
                    row[curr_x / 8] = row[curr_x / 8] & ~mask;
                }
            }
        }
    }
}

void draw_rect(int x, int y, int width, int height, unsigned char color) {
    if (color == 2) {
        draw_gray_rect(x, y, width, height);
        return;
    }

    for (int i = 0; i < height; i = i + 1) {
        for (int j = 0; j < width; j = j + 1) {
            int curr_x = x + j;
            int curr_y = y + i;

            if (curr_x >= 0 && curr_x < 640 && curr_y >= 0 && curr_y < 480) {
                unsigned char *ptr = &VIDEO_MEMORY[curr_y * 80 + (curr_x / 8)];
                unsigned char mask = 128 >> (curr_x % 8);

                if (color > 0) {
                    *ptr = *ptr | mask;   
                } else {
                    *ptr = *ptr & ~mask;   
                }
            }
        }
    }
}