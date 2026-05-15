#include <video.h>
#include <cmos.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>

unsigned char mouse_cursor[16][16] = {
        {1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,1,0,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,1,0,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,1,0,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,1,0,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,1,0,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,2,1,0,0,0,0,0,0},
        {1,2,2,2,2,2,2,2,2,2,1,0,0,0,0,0},
        {1,2,2,2,2,2,2,1,1,1,1,0,0,0,0,0},
        {1,2,2,2,1,2,2,1,0,0,0,0,0,0,0,0},
        {1,2,2,1,0,1,2,2,1,0,0,0,0,0,0,0},
        {1,2,1,0,0,1,2,2,1,0,0,0,0,0,0,0},
        {1,1,0,0,0,0,1,1,0,0,0,0,0,0,0,0},
};


void draw_cursor(int mx, int my) {
    for (int row = 0; row < 16; row = row + 1) {
        for (int col = 0; col < 16; col = col + 1) {
            uint8_t c = mouse_cursor[row][col];
            
            if (c == 1 || c == 2) {
                int curr_x = mx + col;
                int curr_y = my + row;

                if (curr_x >= 0 && curr_x < 640 && curr_y >= 0 && curr_y < 480) {
                    uint8_t *ptr = &VIDEO_MEMORY[curr_y * 80 + (curr_x / 8)];
                    uint8_t mask = 128 >> (curr_x % 8);

                    if (c == 1) {
                        *ptr = *ptr | mask;
                    } else {
                        *ptr = *ptr & ~mask;
                    }
                }
            }
        }
    }
}

int mouse_x = 320;
int mouse_y = 200;

int old_mouse_x = 320;
int old_mouse_y = 200;
int mouse_left_button = 0;
uint8_t cursor_back_buffer[256];

void mouse_handler() {
	if ((inb(0x64) & 1) == 0) {
		return;
	}

	uint8_t mouse_packet[3];
	uint8_t status = inb(0x64);

	if (!(status & 0x01) || !(status & 0x20)) {
		return;
	}

	uint8_t first_byte = inb(0x60);

	if (!(first_byte & 0x08)) {
		return;
	}

	mouse_packet[0] = first_byte;

	mouse_wait_input();
	mouse_packet[1] = inb(0x60);

	mouse_wait_input();
	mouse_packet[2] = inb(0x60);

	int x_offset = (int)((signed char)mouse_packet[1]);
	int y_offset = (int)((signed char)mouse_packet[2]);

	mouse_x += x_offset;
	mouse_y -= y_offset;

    if (mouse_x < 0) mouse_x = 0;
    if (mouse_y < 0) mouse_y = 0;

    if (mouse_x > 624) mouse_x = 624; 
    if (mouse_y > 464) mouse_y = 464; 

	mouse_left_button = first_byte & 0x01;
}

void save_background() {
    for (int i = 0; i < 16; i = i + 1) {
        for (int j = 0; j < 16; j = j + 1) {
            int cur_x = mouse_x + j;
            int cur_y = mouse_y + i;

            if (cur_x >= 0 && cur_x < 640 && cur_y >= 0 && cur_y < 480) {
                uint8_t p_byte = VIDEO_MEMORY[cur_y * 80 + (cur_x / 8)];
                uint8_t mask = 128 >> (cur_x % 8);

                if (p_byte & mask) {
                    cursor_back_buffer[i * 16 + j] = 1;
                } else {
                    cursor_back_buffer[i * 16 + j] = 0;
                }
            }
        }
    }
}

void restore_background() {
    for (int i = 0; i < 16; i = i + 1) {
        for (int j = 0; j < 16; j = j + 1) {
            int old_x = old_mouse_x + j;
            int old_y = old_mouse_y + i;

            if (old_x >= 0 && old_x < 640 && old_y >= 0 && old_y < 480) {
                uint8_t *ptr = &VIDEO_MEMORY[old_y * 80 + (old_x / 8)];
                uint8_t mask = 128 >> (old_x % 8);

                if (cursor_back_buffer[i * 16 + j] == 1) {
                    *ptr = *ptr | mask;    
                } else {
                    *ptr = *ptr & ~mask;   
                }
            }
        }
    }
}

int check_keyboard() {
	int code = get_scancode();
	if (code == 0) return 0;
	return code;
}

void check_ui_clicks() {
	if (mouse_left_button == 1) {
		if (mouse_x >= 190 && mouse_x <= 326 && mouse_y >= 48 && mouse_y <= 74) {
			if (current_mode != 1) {
				current_mode = 1;
				ncount = 1;
			}
		}
		
		if (mouse_x >= 20 && mouse_x <= 156 && mouse_y >= 48 && mouse_y <= 74) {
			if (current_mode != 0) {
				current_mode = 0;
				ncount = 1;
			}
		}

		if (current_mode == 1 && mouse_x >= 156 && mouse_x <= 316 && mouse_y >= 100 && mouse_y <= 118) {
            current_mode = 2;
			ncount = 1;
        }

		if (current_mode == 1 && mouse_x >= 156 && mouse_x <= 316 && mouse_y >= 200 && mouse_y <= 218) {
			current_mode = 3;
			ncount = 1;
		}

		if (current_mode == 2 && mouse_y >= 440 && mouse_y <= 480) {
			show_crt_window = 1;
			ncount = 1;
		}
	}
}

void update_system() {
    asm volatile("cli");
	mouse_handler();
	if (mouse_x != old_mouse_x || mouse_y != old_mouse_y) {
		restore_background();
		save_background();
		draw_cursor(mouse_x, mouse_y);
		old_mouse_x = mouse_x;
		old_mouse_y = mouse_y;
	}
	check_ui_clicks();
    asm volatile("sti");
}