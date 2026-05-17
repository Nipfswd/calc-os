#ifndef STDIO_H
#define STDIO_H
#include <stdint.h>

#define VIDEO_MEMORY ((uint8_t*)0xA0000)

extern int x;
extern int y;
extern int ncount;
extern int current_mode;
extern int show_crt_window;
extern int is_window_crt;
extern int mouse_x;
extern int mouse_y;
extern int mouse_left_button;
extern int is_button_apps;
extern int is_button_files;
extern int is_button_calc;

void screen_clear();
void put_char(char s, uint8_t color);
void print(char *msg, uint8_t color);
void draw_rect(int x, int y, int width, int height, uint8_t color);

void update_system();
void handle_hotkeys(int code);

void set_palette_color(uint8_t index, uint8_t r, uint8_t g, uint8_t b);
void init_palette(); 

extern int get_scancode();
extern void outb( uint16_t port, uint8_t val);
extern volatile uint8_t inb(uint16_t port);
extern void outw(uint16_t port, uint16_t val);
extern volatile uint16_t inw(uint16_t port);

#endif