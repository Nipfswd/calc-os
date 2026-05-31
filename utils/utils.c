#include <fat.h>
#include <cmos.h>
#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <stdint.h>
#include <ata.h>
#include <sound.h>
#include <pci.h>

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

void reboot() {
    while (inb(0x64) & 0x02);
    outb(0x64, 0xFE);

    for (;;);
}

void name_clear() {
    for (int i = 0; i < 128; i++) {
        name[i] = 0;
    }
}

void content_clear() {
    for (int i = 0; i < 512; i++) {
        content[i] = 0;
    }
}

void format_fat_name(const char* src, char dest[11]) {
    for (int i = 0; i < 11; i++) {
        dest[i] = ' ';
    }

    int name_idx = 0;
    int ext_idx = 0;
    int i = 0;
    int in_ext = 0;

    while (src[i] != '\0' && (name_idx < 8 || ext_idx < 3)) {
        char c = src[i++];
        if (c == '.') {
            in_ext = 1;
            continue;
        }

        if (c >= 'a' && c <= 'z') {
            c -= 32;
        }

        if (!in_ext) {
            if (name_idx < 8) {
                dest[name_idx++] = c;
            }
        } else {
            if (ext_idx < 3) {
                dest[8 + ext_idx++] = c;
            }
        }
    }
}

void draw_file_icons() {
    uint16_t buffer[256];
    ata_read_sector(0, buffer);
    struct fat12_bpb* bpb = (struct fat12_bpb*)buffer;

    uint32_t root_lba = bpb->reserved_sectors + (bpb->num_fats * bpb->fat_size_sectors);
    uint32_t root_sectors = ((bpb->root_entries * 32) + (bpb->bytes_per_sector - 1)) / bpb->bytes_per_sector;

    int icon_index = 0;
    for (uint32_t s = 0; s < root_sectors; s++) {
        ata_read_sector(root_lba + s, buffer);
        struct fat12_entry* entries = (struct fat12_entry*)buffer;

        for (int i = 0; i < 16; i++) {
            if (entries[i].name[0] == 0x00) return;
            if ((uint8_t)entries[i].name[0] == 0xE5) continue;
            if (entries[i].attributes == 0x0F) continue;

            int col = icon_index % 6;
            int row = icon_index / 5;
            int icon_x = 20 + col * 180;
            int icon_y = 100 + row * 100;

            draw_rect(icon_x, icon_y, 130, 30, 15);

            char name_buf[9];
            char ext_buf[4];
            int name_len = 0;
            int ext_len = 0;

            for (int j = 0; j < 8; j++) {
                if (entries[i].name[j] != ' ') {
                    name_buf[name_len++] = entries[i].name[j];
                }
            }
            name_buf[name_len] = '\0';

            for (int j = 0; j < 3; j++) {
                if (entries[i].ext[j] != ' ') {
                    ext_buf[ext_len++] = entries[i].ext[j];
                }
            }
            ext_buf[ext_len] = '\0';

            x = icon_x + 8;
            y = icon_y + 8;
            print(name_buf, 0);
            if (ext_len > 0) {
                print(".", 0);
                print(ext_buf, 0);
            }

            icon_index++;
            if (icon_index >= 6) return;
        }
    }
}

void play_startup_sound() {
    beep(233, 80); 
    beep(349, 80); 
    beep(311, 80);
    beep(466, 80); 
    beep(523, 80);
    
    beep(698, 100); 
    beep(695, 70); 
    beep(690, 70);
}

int atoi_super(const char* str) {
    int res = 0;
    int i = 0;

    while (str[i] == ' ' || str[i] == '\t' || str[i] == '\n' || str[i] == '\r') {
        i++;
    }

    while (str[i] >= '0' && str[i] <= '9') {
        res = res * 10 + (str[i] - '0');
        i++;
    }

    return res;
}