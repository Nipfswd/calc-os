#include <cmos.h>
#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <fat.h>
#include <stdint.h>

char command[256];
char value[128];
char content[512];

static void format_fat_name(const char* src, char dest[11]) {
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

void prompt() {
refresh:
    ncount = 0;

    draw_rect(0, 0, 640, 400, 1);
    draw_rect(0, 0, 640, 40, 0);

    x = 276;
    y = 10;
    print("CalcOS", 0xFFFFFF);

    if (current_mode == 0) {
        draw_rect(0, 80, 640, 480, 0);

        draw_button(20, 48, 136, 26, "Terminal", 0, 1);
        draw_button(190, 48, 136, 26, "Explorer", 2, 0);
        x = 0;
        y = 96;
    }
    else if (current_mode == 2) {
        draw_rect(0, 80, 640, 480, 0);

        draw_button(20, 48, 136, 26, "Terminal", 2, 0);
        draw_button(190, 48, 136, 26, "Explorer", 0, 1);

        draw_button(0, 440, 640, 40, "F2 - create a new file", 1, 0);

        int file_count = 0;


        int col = file_count % 3;
        int row = file_count / 3;

        int icon_x = (16 + col * 100) * 2;
        int icon_y = (50 + row * 34) * 2;

        draw_rect(icon_x, icon_y, 160, 48, 1);

        x = icon_x + 16;
        y = icon_y + 16;


        //print(, 0);

        file_count++;

        if (show_crt_window == 1) {
            is_window_crt = 1;
            draw_rect(104, 100, 432, 260, 0);
            draw_rect(100, 96, 432, 260, 1);
            draw_rect(104, 100, 424, 252, 2);

            draw_rect(104, 100, 424, 32, 0);

            x = 118;
            y = 104;
            print("Create a new value", 1);

            x = 118;
            y = 140;
            print("Value:", 0);
            draw_rect(118, 152, 404, 24, 1);
            draw_rect(120, 154, 400, 20, 0);
        }
    }
    else if (current_mode == 3) {
        draw_rect(0, 80, 640, 480, 0);
        
        draw_button(20, 48, 136, 26, "Terminal", 2, 0);
        draw_button(190, 48, 136, 26, "Explorer", 0, 1);

          x = 0;
          y = 96;
          print("System Information:\n", 1);
          print("Battery Status: ", 1);
          unsigned char battery_status = check_battery();
          if (battery_status) {
              print("OK\n", 1);
          } else {
              print("BAD. Please insert a new CMOS battery\n", 1);
          }
    }
    else {
        draw_rect(0, 80, 640, 480, 0);

        draw_button(20, 48, 136, 26, "Terminal", 2, 0);
        draw_button(190, 48, 136, 26, "Explorer", 0, 1);

        if (is_button_files == 1) {
            draw_button(156, 100, 320, 36, "Values", 1, 0);
            draw_button(156, 200, 320, 36, "System", 2, 0);
        }
        else if (is_button_apps == 1) {
            draw_button(156, 100, 320, 36, "Values", 2, 0);
            draw_button(156, 200, 320, 36, "System", 1, 0);
        }
    }

    while (1) {
        if (ncount == 1) goto refresh;

        update_system();

        if (ncount == 1) goto refresh;

        if (current_mode == 0) {
            print("> ", 0xFFFFFF);
            input_wait_string(command);

            if (ncount == 1) goto refresh;

            print("\n", 0xFFFFFF);

            if (compare_strings(command, "help")) {
                print("Available commands:\n", 0xFFFFFF);
                print("  help - show this message\n", 0xFFFFFF);
                print("  cln  - clear the screen\n", 0xFFFFFF);
                print("  ls  - list all files\n", 0xFFFFFF);
                print("  crt  - create a new file\n", 0xFFFFFF);
                print("  draw - draw a rectangle\n", 0xFFFFFF);
                print("  status - check system status\n", 0xFFFFFF);
                print("  livetime - print system livetime irq0 ticks\n", 0xFFFFFF);
            }
            else if (compare_strings(command, "cln")) {
                screen_clear();
                ncount = 1;
            }
            else if (compare_strings(command, "ls")) {
                list_files();
            }
            else if (compare_strings(command, "crt")) {
                print("Name: ", 1);
                input_wait_string(value);
                print("\n", 1);
                print("Content: ", 1);
                input_wait_string(content);
                print("\n", 1);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(value, name_11);

                uint8_t buffer[512] = {0};
                for (int j = 0; j < len; j++) buffer[j] = (uint8_t)content[j];

                create_file(name_11, buffer, len);
            }    
            else if (compare_strings(command, "draw")) {
                char val[16];
                int r_w, r_h, r_x, r_y;

                print("Enter width: ", 1);
                input_wait_string(val);
                r_w = atoi(val);

                print("\nEnter height: ", 1);
                input_wait_string(val);
                r_h = atoi(val);

                print("\nEnter x: ", 1);
                input_wait_string(val);
                r_x = atoi(val);

                print("\nEnter y: ", 1);
                input_wait_string(val);
                r_y = atoi(val);

                print("\n", 1);
                draw_rect(r_x, r_y, r_w, r_h, 1);
            }
            else if (compare_strings(command, "livetime")) {
                itoa(timer_ticks / 18, timer_str);
                print(timer_str, 1);
                print("\n", 1);
            }
                else if (compare_strings(command, "status")) {
                    unsigned char battery_status = check_battery();
                    if (battery_status) {
                        print("Battery: OK\n", 1);
                    } else {
                        print("Battery: BAD Please insert a new CMOS battery\n", 1);
                    }
                }
            else {
                if (command[0] != '\0') {
                    print("Uncnown command. Type 'help'\n", 1);
                }
            }
        }
        else if (current_mode == 2) {
            int code = get_scancode();
            if (code != 0) {
                handle_hotkeys(code);
                if (code == 0x3C) {
                    show_crt_window = 1;
                    ncount = 1;
                }
            }

            if (ncount == 1) goto refresh;

            if (show_crt_window == 1) {
                is_window_crt = 1;

                for (int i = 0; i < 256; i++) {
                    value[i] = 0;
                }

                x = 126;
                y = 156;
                input_wait_string(value);

                unsigned char value_int = atoi(value);

                if (ncount == 1) goto refresh;

                for (int i = 0; i < 5; i++) {
                    unsigned char data = read(0x50 + i);
                    if (data == 0) {
                        write(0x50 + i, value_int);
                        break;
                    }
                }

                show_crt_window = 0;
                is_window_crt = 0;
                ncount = 1;
            }
        }
        else {
            int code = get_scancode();
            if (code != 0) handle_hotkeys(code);
            if (ncount == 1) goto refresh;
        }
    }
}


void __attribute__((section(".text.entry"))) kernel_main() {
    asm volatile("cli");
    init_mouse();
	screen_clear();
    init_idt();
    asm volatile("sti");
    init_palette();
	prompt();
}
