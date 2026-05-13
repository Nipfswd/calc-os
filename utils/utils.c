#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <stdint.h>

int compare_strings(const char *str1, const char *str2) {
	int i = 0;
	while (str1[i] == str2[i]) {
		if (str1[i] == '\0') {
			return 1;
		}
		i++;
	}
	return 0;
}

void copy_string(char *dest, char *src) {
	int i = 0;
	while (src[i] != '\0') {
		dest[i] = src[i];
		i++;
	}
	dest[i] = '\0';
}

int atoi(char *str) {
	int res = 0;
	for (int i = 0; str[i] != '\0'; ++i) {
		if (str[i] >= '0' && str[i] <= '9') {
			res = res * 10 + str[i] - '0';
		}
	}
	return res;
}

void draw_button(int _x, int _y, int _width, int _height, char *_msg, uint8_t color, uint8_t text_color) {
    draw_rect(_x, _y, _width, _height, color);
    x = _x + 4;
    y = _y + 4;

    print(_msg, text_color);
}

void itoa(uint32_t n, char* s) {
    int i = 0;
    if (n == 0) s[i++] = '0';
    while (n > 0) {
        s[i++] = (n % 10) + '0';
        n /= 10;
    }
    s[i] = '\0';
    
    for (int j = 0; j < i / 2; j++) {
        char temp = s[j];
        s[j] = s[i - j - 1];
        s[i - j - 1] = temp;
    }
}

void htoa(int n, char str[]) {
    str[0] = '0';
    str[1] = 'x';
    const char *hex_chars = "0123456789ABCDEF";
    
    for (int i = 7; i >= 0; i--) {
        str[i + 2] = hex_chars[n & 0xF];
        n >>= 4;
    }
    str[10] = '\0';
}

uint8_t check_battery() {
    uint8_t status = read(0x0D);
    if (!(status & 0x80)) {
        return 0; 
    }
    return 1; 
}

uint8_t bcd_to_bin(uint8_t val) {
    return (val & 0x0F) + ((val >> 4) * 10);
}

void get_time(int *h, int *m) {
    while (read(0x0A) & 0x80);

    uint8_t raw_h = read(0x04);
    uint8_t raw_m = read(0x02);

    *h = bcd_to_bin(raw_h);
    *m = bcd_to_bin(raw_m);
}

int memcmp(const char *s1, const char *s2, int n) {
    for (int i = 0; i < n; i++) {
        if (s1[i] != s2[i]) {
            return 0; 
        }
    }
    return 1; 
}

void* memcpy(void* dest, const void* src, uint32_t n) {
    uint8_t* d = (uint8_t*)dest;
    const uint8_t* s = (const uint8_t*)src;

    for (uint32_t i = 0; i < n; i++) {
        d[i] = s[i];
    }

    return dest;
}