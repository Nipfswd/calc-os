#include <fat.h>
#include <cmos.h>
#include <video.h>
#include <utils.h>
#include <mouse.h>
#include <keyboard.h>
#include <idt.h>
#include <stdint.h>
#include <ata.h>
#include <mm.h>
#include <sound.h>
#include <pci.h>

char command[256];
char name[128];
char content[512];

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

static void draw_file_icons() {
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

            int col = icon_index % 3;
            int row = icon_index / 3;
            int icon_x = 20 + col * 180;
            int icon_y = 100 + row * 100;

            draw_rect(icon_x, icon_y, 130, 30, 1);

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
    beep(233, 4); 
    beep(349, 4); 
    beep(311, 4);
    beep(466, 4); 
    beep(523, 4);
    
    beep(698, 6); 
    beep(695, 3); 
    beep(690, 3);
}

void shell() {
refresh:
    ncount = 0;

    draw_rect(0, 0, 640, 400, 1);

    draw_button(10, 5, 56, 26, "CalcOS", 0, 1);

    if (current_mode == 0) {
        if (draw_0 == 1) {
            draw_rect(0, 40, 640, 480, 0);

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", 0, 1);
                draw_button(191, 11, 15, 15, "x", 1, 0);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", 2, 0);
                draw_button(351, 11, 15, 15, "x", 1, 0);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, 2);

                draw_button(10, 31, 70, 26, "Terminal", 0, 1);
                draw_button(10, 51, 70, 26, "Explorer", 1, 0);

                draw_button(10, 85, 15, 15, "x", 0, 1);
            }

            x = 0;
            y = 50;

        } else {
            current_mode = 5;
            ncount = 1;
        }
    }
    else if (current_mode == 2) {
        draw_rect(0, 40, 640, 480, 0);

        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", 0, 1);
            draw_button(191, 11, 15, 15, "x", 1, 0);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", 2, 0);
            draw_button(351, 11, 15, 15, "x", 1, 0);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, 2);

            draw_button(10, 31, 70, 26, "Terminal", 0, 1);
            draw_button(10, 51, 70, 26, "Explorer", 1, 0);


            draw_button(10, 85, 15, 15, "x", 0, 1);
        }

        draw_button(0, 440, 640, 40, "F2 - create a new file", 1, 0);

        draw_file_icons();

        if (show_crt_window == 1) {
            is_window_crt = 1;
            draw_rect(104, 100, 432, 260, 0);
            draw_rect(100, 96, 432, 260, 1);
            draw_rect(104, 100, 424, 252, 2);

            draw_rect(104, 100, 424, 32, 0);

            x = 118;
            y = 104;
            print("Create a new file", 1);

            x = 118;
            y = 140;
            print("Name:", 1);
            draw_rect(118, 152, 404, 24, 1);
            draw_rect(120, 154, 400, 20, 0);

            x = 118;
            y = 190;
            print("Content:", 1);
            draw_rect(118, 242, 404, 24, 1);
            draw_rect(120, 244, 400, 20, 0);
        }
    }
    else if (current_mode == 3) {
        draw_rect(0, 40, 640, 480, 0);
        
        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", 0, 1);
            draw_button(191, 11, 15, 15, "x", 1, 0);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", 2, 0);
            draw_button(351, 11, 15, 15, "x", 1, 0);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, 2);

            draw_button(10, 31, 70, 26, "Terminal", 0, 1);
            draw_button(10, 51, 70, 26, "Explorer", 1, 0);

            draw_button(10, 85, 15, 15, "x", 0, 1);
        }

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
        if (draw_1 == 1) {
           draw_rect(0, 40, 640, 480, 0);

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", 0, 1);
                draw_button(191, 11, 15, 15, "x", 1, 0);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", 2, 0);
                draw_button(351, 11, 15, 15, "x", 1, 0);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, 2);

                draw_button(10, 31, 70, 26, "Terminal", 0, 1);
                draw_button(10, 51, 70, 26, "Explorer", 1, 0);

                draw_button(10, 85, 15, 15, "x", 0, 1);
            }

            if (is_button_files == 1) {
                draw_button(156, 100, 320, 36, "Files", 1, 0);
                draw_button(156, 200, 320, 36, "System", 2, 0);
            }
            else if (is_button_apps == 1) {
                draw_button(156, 100, 320, 36, "Files", 2, 0);
                draw_button(156, 200, 320, 36, "System", 1, 0);
            } 
        } else {
            current_mode = 0;
            ncount = 1;
        }
        is_button_calc = 0;
    }

    while (1) {
        if (ncount == 1) goto refresh;

        update_system();

        if (ncount == 1) goto refresh;

        if (current_mode == 0) {
            print("> ", 1);
            input_wait_string(command);

            if (ncount == 1) goto refresh;

            print("\n", 1);

            if (compare_strings(command, "help")) {
                print("Available commands:\n", 1);
                print("  help - show this message\n", 1);
                print("  cln  - clear the screen\n", 1);
                print("  ls  - list all files\n", 1);
                print("  crt  - create a new file\n", 1);
                print("  draw - draw a rectangle\n", 1);
                print("  status - check system status\n", 1);
                print("  livetime - print system livetime irq0 ticks\n", 1);
                print("  cat - print file content\n", 1);
                print("  devices - print PCI devices\n", 1);
                print("  send - send a byte to the network\n", 1);
            }
            else if (compare_strings(command, "cat")) {
                content_clear();
                print("Name: ", 1);
                input_wait_string(name);
                print("\n", 1);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

                uint8_t buffer[512] = {0};
                for (int j = 0; j < len; j++) buffer[j] = (uint8_t)content[j];

                read_file(name_11, content);
                print(content, 1);
                print("\n", 1);
            }
            else if (compare_strings(command, "cln")) {
                screen_clear();
                ncount = 1;
            }
            else if (compare_strings(command, "ls")) {
                list_files();
            }
            else if (compare_strings(command, "crt")) {
                name_clear();
                content_clear();
                print("Name: ", 1);
                input_wait_string(name);
                print("\n", 1);
                print("Content: ", 1);
                input_wait_string(content);
                print("\n", 1);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

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
            else if (compare_strings(command, "devices")) {
                pci_print_devices();
            }
            else if (compare_strings(command, "send")) {
                uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                rtl8111_send('A', dest_mac);
            }
            else {
                if (command[0] != '\0') {
                    print("Unknown command. Type 'help'\n", 1);
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
                name_clear();
                content_clear();

                if (ncount == 1) goto refresh;

                asm volatile("cli");
                x = 126;
                y = 156;
                input_wait_string(name);

                x = 156;
                y = 246;
                input_wait_string(content);
                asm volatile("sti");

                if (ncount == 1) goto refresh;

                print("\n", 1);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

                uint8_t buffer[512] = {0};
                for (int j = 0; j < len; j++) buffer[j] = (uint8_t)content[j];

                create_file(name_11, buffer, len);

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

void boot() {
    is_scaled = 1;
    screen_clear();
    print("Booting CalcOS...\n", 1);

    print("Scanning devices...", 1);
    pci_scan();

    init_mouse();
    print("[OK]\n", 1);

    init_identity_paging();
    enable_paging();
    print("[OK]\n", 1);

    rtl8111_init();
    print("[OK]\n", 1);
    delay_ticks(15);

    screen_clear();
    init_palette();
    is_scaled = 0;
    print("Success!", 1);
    is_scaled = 1;
    delay_ticks(15);

    draw_rect(0, 0, 640, 480, 1);

    x = 0;
    y = 10;

    print("Welcome to CalcOS!", 0);
    play_startup_sound();

    is_scaled = 0;
}

void __attribute__((section(".text.entry"))) kernel_main() {
    asm volatile("cli");
	screen_clear();
    init_idt();
    asm volatile("sti");
    boot();
    shell();
}
