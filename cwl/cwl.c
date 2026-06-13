#include <cmos.h>
#include <video.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>
#include <idt.h>
#include <task.h>
#include <stdint.h>
#include <sound.h>
#include <cwl.h>
#include <mm.h>
#include <pci.h>
#include <fat.h>

void parse_cwl(char *text, CWL_Doc *doc) {
    clear_string(doc->title, 128);
    clear_string(doc->main_text, 1024);

    int in_main = 0;
    int pos = 0;

    while (text[pos]) {

        char line[256];
        clear_string(line, 256);

        int li = 0;
        while (text[pos] && text[pos] != '\n' && li < 255) {
            line[li++] = text[pos++];
        }
        if (text[pos] == '\n') pos++;

        trim(line);

        if (starts_with(line, "TITLE:")) {
            char *p = line + 6;
            trim(p);
            copy_string(doc->title, p);
        }

        else if (starts_with(line, "MAIN:")) {
            in_main = 1;
        }

        else if (in_main && starts_with(line, "TEXT:")) {
            char *p = line + 5;
            trim(p);
            append(doc->main_text, p, 1024);
            append(doc->main_text, "\n", 1024);
        }
    }
}

void render_cwl(CWL_Doc *doc) {
    if (current_mode == 5) {
        x = 0;
        y = 50;
        print("Browser - ", 14);
        print(doc->title, 15);
        print('\n', 15);
        print('\n', 15);

        print(doc->main_text, 15);
    }
}

void open_page(const char* filename) {
    uint8_t buf[2048];
    char fat_name[11];
    CWL_Doc doc;

    format_fat_name(filename, fat_name);

    read_file(fat_name, buf);

    parse_cwl((char*)buf, &doc);

    render_cwl(&doc);
}


void browser_main() {
    x = 0;
    y = 90;

    print("Calc Browser\n\n", 15);

    open_page("welcome.cwl");
}


