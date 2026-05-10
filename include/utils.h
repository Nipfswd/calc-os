#ifndef UTILS_H
#define UTILS_H

int compare_strings(const char *str1, const char *str2);

void copy_string(char *dest, char *src);

int atoi(char *str);
void itoa(unsigned int n, char* s);
void htoa(int n, char str[]);

void draw_button(int _x, int _y, int _width, int _height, char *_msg, unsigned char color, unsigned char text_color);

unsigned char check_battery();

#endif