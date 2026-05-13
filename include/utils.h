#ifndef UTILS_H
#define UTILS_H
#include <stdint.h>

int compare_strings(const char *str1, const char *str2);

void copy_string(char *dest, char *src);

int atoi(char *str);
void itoa(unsigned int n, char* s);
void htoa(int n, char str[]);

void draw_button(int _x, int _y, int _width, int _height, char *_msg, uint8_t color, uint8_t text_color);

uint8_t check_battery();
uint8_t bcd_to_bin(uint8_t val);
void get_time(int *h, int *m);
int memcmp(const char *s1, const char *s2, int n);
void* memcpy(void* dest, const void* src, uint32_t n);

#endif