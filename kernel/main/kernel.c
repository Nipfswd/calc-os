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
#include <mm.h>
#include <forth.h>
#include <task.h>

char command[256];
char name[128];
char content[512];
char byte_str[16];

void system() {
refresh:
    ncount = 0;

    draw_rect(0, 0, 1024, 40, 7);

    draw_button(10, 5, 56, 26, "CalcOS", 0, 15);

    asm volatile("sti");

    if (current_mode == 0) {
        if (draw_0 == 1) {
            draw_rect(0, 40, 1024, 728, 0);

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", COLOR_RED, 15);
                draw_button(191, 11, 15, 15, "x", 15, 0);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", COLOR_GREEN, 0);
                draw_button(351, 11, 15, 15, "x", 15, 0);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, 7);

                draw_button(10, 31, 70, 26, "Terminal", COLOR_RED, 15);
                draw_button(10, 51, 70, 26, "Explorer", COLOR_GREEN, 0);

                draw_button(10, 85, 15, 15, "x", 0, 15);
                draw_button(65, 85, 15, 15, "r", 0, 15);
            }

            x = 0;
            y = 50;

        } else {
            current_mode = 5;
            ncount = 1;
        }
    }
    else if (current_mode == 2) {
        draw_desktop();

        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(191, 11, 15, 15, "x", 15, 0);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, 0);
            draw_button(351, 11, 15, 15, "x", 15, 0);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, 7);

            draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, 0);

            draw_button(10, 85, 15, 15, "x", 0, 15);
            draw_button(65, 85, 15, 15, "r", 0, 15);
        }

        draw_button(0, 728, 1024, 40, "F2 - create a new file", COLOR_BLUE, 15);

        draw_file_icons();

        if (show_crt_window == 1) {
            is_window_crt = 1;
            int wx = 296; 
            int wy = 244;

            draw_rect(wx + 4, wy + 4, 432, 260, 0);
            draw_rect(wx,     wy,     432, 260, 15);
            draw_rect(wx + 4, wy + 4, 424, 252, 7);
            draw_rect(wx + 4, wy + 4, 424, 32, 0);

            x = wx + 18;
            y = wy + 8;
            print("Create a new file", 15);

            x = wx + 18;
            y = wy + 44;
            print("Name:", 15);
            draw_rect(wx + 18, wy + 56, 404, 24, 15);
            draw_rect(wx + 20, wy + 58, 400, 20, 0);

            x = wx + 18;
            y = wy + 94;
            print("Content:", 15);
            draw_rect(wx + 18, wy + 146, 404, 24, 15); 
            draw_rect(wx + 20, wy + 148, 400, 20, 0);
        }
    }
    else if (current_mode == 3) {
        draw_desktop();
        
        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(191, 11, 15, 15, "x", 15, 0);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, 0);
            draw_button(351, 11, 15, 15, "x", 15, 0);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, 7);

            draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, 0);

            draw_button(10, 85, 15, 15, "x", 0, 15);
            draw_button(65, 85, 15, 15, "r", 0, 15);
        }

        x = 0;
        y = 96;
        print("System Information:\n", 15);
        print("Battery Status: ", 15);
        unsigned char battery_status = check_battery();
        if (battery_status) {
            print("OK\n", 15);
        } else {
            print("BAD. Please insert a new CMOS battery\n", 15);
        }
    }
    else if (current_mode == 5) {
        draw_desktop();

        if (draw_0 == 1) {
            draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(191, 11, 15, 15, "x", 15, 0);
        }

        if (draw_1 == 1) {
            draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, 0);
            draw_button(351, 11, 15, 15, "x", 15, 0);
        }

        if (is_button_calc == 1) {
            draw_rect(10, 31, 72, 70, 7);

            draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, 15);
            draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, 0);

            draw_button(10, 85, 15, 15, "x", 0, 15);
            draw_button(65, 85, 15, 15, "r", 0, 15);
        }

        draw_pack_icons();

        draw_button(0, 728, 1024, 40, "F2 - Send a pack", COLOR_BLUE, 15);

        if (is_window_send == 1) {
            int wx = 296; 
            int wy = 244;

            draw_rect(wx + 4, wy + 4, 432, 260, 0);
            draw_rect(wx,     wy,     432, 260, 15);
            draw_rect(wx + 4, wy + 4, 424, 252, 7);
            draw_rect(wx + 4, wy + 4, 424, 32, 0);

            x = wx + 18;
            y = wy + 8;
            print("Send a pack", 15);

            x = wx + 18;
            y = wy + 44;
            print("Byte:", 15);
            draw_rect(wx + 18, wy + 56, 404, 24, 15);
            draw_rect(wx + 20, wy + 58, 400, 20, 0);
        }
    }
    else {
        is_scaled = 0;

        if (draw_1 == 1) {
                draw_desktop();

            if (draw_0 == 1) {
                draw_button(78, 5, 136, 26, "Terminal", COLOR_GREEN, 15);
                draw_button(191, 11, 15, 15, "x", 15, 0);
            }

            if (draw_1 == 1) {
                draw_button(238, 5, 136, 26, "Explorer", COLOR_RED, 0);
                draw_button(351, 11, 15, 15, "x", 15, 0);
            }

            if (is_button_calc == 1) {
                draw_rect(10, 31, 72, 70, 7);

                draw_button(10, 31, 70, 26, "Terminal", COLOR_GREEN, 15);
                draw_button(10, 51, 70, 26, "Explorer", COLOR_RED, 0);

                draw_button(10, 85, 15, 15, "x", 0, 15);
                draw_button(65, 85, 15, 15, "r", 0, 15);
            }

            if (is_button_files == 1) {
                draw_button(352, 250, 320, 36, "Files", COLOR_RED, 0);
                draw_button(352, 350, 320, 36, "System", COLOR_GREEN, 0);
                draw_button(352, 450, 320, 36, "Ethernet", COLOR_GREEN, 0);
            }
            else if (is_button_apps == 1) {
                draw_button(352, 250, 320, 36, "Files", COLOR_GREEN, 0);
                draw_button(352, 350, 320, 36, "System", COLOR_RED, 0);
                draw_button(352, 450, 320, 36, "Ethernet", COLOR_GREEN, 0);
            } 
            else if (is_button_ethernet == 1) {
                draw_button(352, 250, 320, 36, "Files", COLOR_GREEN, 0);
                draw_button(352, 350, 320, 36, "System", COLOR_GREEN, 0);
                draw_button(352, 450, 320, 36, "Ethernet", COLOR_RED, 0);
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
            is_scaled = 2;
            print("> ", 15);
            input_wait_string(command);

            if (ncount == 1) goto refresh;

            print("\n", 15);

            if (compare_strings(command, "help")) {
                print("Available commands:\n", 15);
                print("  help - show this message\n", 15);
                print("  cln  - clear the screen\n", 15);
                print("  ls  - list all files\n", 15);
                print("  touch  - create a new file\n", 15);
                print("  draw - draw a rectangle\n", 15);
                print("  status - check system status\n", 15);
                print("  livetime - print system livetime irq0 ticks\n", 15);
                print("  cat - print file content\n", 15);
                print("  devices - print PCI devices\n", 15);
                print("  send - send a byte to the network\n", 15);
                print("  behave - receive a byte from the network\n", 15);
                print("  reboot - reboot the system\n", 15);
                print("  forth - Forth interpreter\n", 15);
            }
            else if (compare_strings(command, "reboot")) {
                reboot();
            }
            else if (compare_strings(command, "cat")) {
                content_clear();
                print("Name: ", 15);
                input_wait_string(name);
                print("\n", 15);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

                read_file(name_11, content);
                print(content, 15);
                print("\n", 15);
            }
            else if (compare_strings(command, "cln")) {
                screen_clear();
                ncount = 1;
            }
            else if (compare_strings(command, "ls")) {
                list_files();
            }
            else if (compare_strings(command, "touch")) {
                is_scaled = 2;
                name_clear();
                content_clear();
                
                print("Enter file name: ", 15);
                input_wait_string(name);
                print("\n", 15);

                char name_11[11];
                format_fat_name(name, name_11);

                screen_clear();

                draw_rect(0, 0, 1024, 30, 7); 
                x = 10; y = 8;
                print("Editing: ", 0);
                print(name, 0);

                draw_rect(0, 738, 1024, 30, 7); 
                x = 10; y = 746;
                print("F2: Save and Exit", 0);

                x = 10;
                y = 40;

                uint8_t file_buffer[512];
                for (int i = 0; i < 512; i++) file_buffer[i] = 0;
                int buffer_ptr = 0;

                while (1) {
                    update_system(); 

                    int code = get_scancode();
                    if (code == 0) continue;

                    handle_hotkeys(code);

                    if (code == 0x3C) {
                        create_file(name_11, file_buffer, buffer_ptr);
                        
                        screen_clear();
                        current_mode = 0;
                        ncount = 1; 
                        break; 
                    }

                    if (code == 0x1C) {
                        if (buffer_ptr < 511) {
                            file_buffer[buffer_ptr++] = '\n';
                            put_char('\n', 15); 
                        }
                        continue;
                    }

                    if (code == 0x0E) {
                        if (buffer_ptr > 0) {
                            buffer_ptr--;
                            if (file_buffer[buffer_ptr] != '\n') {
                                x = x - 8; 
                                draw_rect(x, y, 8, 8, 0); 
                            }
                            file_buffer[buffer_ptr] = 0;
                        }
                        continue;
                    }

                    char letter = 0;
                    switch (code) {
                        case 0x18: letter = 'o'; break; 
                        case 0x23: letter = 'h'; break;
                        case 0x2E: letter = 'c'; break; 
                        case 0x1E: letter = 'a'; break;
                        case 0x26: letter = 'l'; break; 
                        case 0x12: letter = 'e'; break;
                        case 0x13: letter = 'r'; break; 
                        case 0x2D: letter = 'x'; break;
                        case 0x17: letter = 'i'; break; 
                        case 0x14: letter = 't'; break;
                        case 0x19: letter = 'p'; break; 
                        case 0x10: letter = 'q'; break;
                        case 0x11: letter = 'w'; break; 
                        case 0x15: letter = 'y'; break;
                        case 0x16: letter = 'u'; break; 
                        case 0x1F: letter = 's'; break;
                        case 0x20: letter = 'd'; break; 
                        case 0x21: letter = 'f'; break;
                        case 0x22: letter = 'g'; break; 
                        case 0x24: letter = 'j'; break;
                        case 0x25: letter = 'k'; break; 
                        case 0x2C: letter = 'z'; break;
                        case 0x2F: letter = 'v'; break; 
                        case 0x30: letter = 'b'; break;
                        case 0x31: letter = 'n'; break; 
                        case 0x32: letter = 'm'; break;
                        case 0x39: letter = ' '; break;
                        case 0x02: letter = '1'; break; 
                        case 0x03: letter = '2'; break;
                        case 0x04: letter = '3'; break; 
                        case 0x05: letter = '4'; break;
                        case 0x06: letter = '5'; break; 
                        case 0x07: letter = '6'; break;
                        case 0x08: letter = '7'; break; 
                        case 0x09: letter = '8'; break;
                        case 0x0A: letter = '9'; break; 
                        case 0x0B: letter = '0'; break;
                        case 0x0C: letter = '-'; break; 
                        case 0x0D: letter = '+'; break;
                        case 0x34: letter = '.'; break; 
                        case 0x35: letter = '/'; break;
                        case 0x1A: letter = '['; break; 
                        case 0x1B: letter = ']'; break;
                        case 0x33: letter = ','; break; 
                        case 0x28: letter = '\''; break;
                        case 0x27: letter = ';'; break;
                        default:   letter = 0;   break;
                    }

                    if (letter != 0 && buffer_ptr < 511) {
                        put_char(letter, 15); 
                        file_buffer[buffer_ptr] = letter;
                        buffer_ptr++;
                    }
                }
            }   
            else if (compare_strings(command, "draw")) {
                char val[16];
                int r_w, r_h, r_x, r_y;

                print("Enter width: ", 15);
                input_wait_string(val);
                r_w = atoi(val);

                print("\nEnter height: ", 15);
                input_wait_string(val);
                r_h = atoi(val);

                print("\nEnter x: ", 15);
                input_wait_string(val);
                r_x = atoi(val);

                print("\nEnter y: ", 15);
                input_wait_string(val);
                r_y = atoi(val);

                print("\n", 15);
                draw_rect(r_x, r_y, r_w, r_h, 15);
            }
            else if (compare_strings(command, "livetime")) {
                itoa(timer_ticks / 18, timer_str);
                print(timer_str, 15);
                print("\n", 15);
            }
            else if (compare_strings(command, "status")) {
                unsigned char battery_status = check_battery();
                if (battery_status) {
                    print("Battery: OK\n", 15);
                } else {
                    print("Battery: BAD Please insert a new CMOS battery\n", 15);
                }
            }
            else if (compare_strings(command, "devices")) {
                pci_print_devices();
            }
            else if (compare_strings(command, "send")) {
                while (get_scancode() != 0); 
                print("Enter a byte: ", 15);
                for (int i = 0; i < 16; i++) byte_str[i] = 0;
                for (volatile int j = 0; j < 200000; j++);

                while (1) {
                    ncount = 0; 
                    input_wait_string(byte_str);
                    if (byte_str[0] == '\0') {
                        ncount = 0;
                        continue;
                    }
                    break;
                }

                int byte_val = atoi_super(byte_str);
                uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                if (byte_val < 0 || byte_val > 255) {
                    print("Invalid byte value. Must be between 0 and 255.\n", 15);
                } else {
                    send_pack((uint8_t)byte_val, dest_mac);
                }
                print("\n", 15);
            }
            else if (compare_strings(command, "behave")) {
                read_pack();
            }
           else if (compare_strings(command, "forth")) {
                name_clear();
                print("Enter Forth file: ", 15);
                input_wait_string(name);

                int len = 0;
                while (content[len] != '\0') len++;
                if (len > 512) len = 512;

                char name_11[11];
                format_fat_name(name, name_11);

                uint8_t buffer[512] = {0};
                for (int j = 0; j < len; j++) buffer[j] = (uint8_t)content[j];

                read_file(name_11, content);
                create_task(2);
                print("\n", 15);
            }
            else {
                if (command[0] != '\0') {
                    print("Unknown command. Type 'help'\n", 15);
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

                int wx = 296; 
                int wy = 244;

                x = wx + 26;
                y = wy + 60;
                input_wait_string(name);

                x = wx + 26;
                y = wy + 150;
                input_wait_string(content);

                if (ncount == 1) goto refresh;

                print("\n", 15);

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
        else if (current_mode == 5) {
            int code = get_scancode();
            if (code != 0) {
                handle_hotkeys(code);
                if (code == 0x3C) {
                    is_window_send = 1;
                    ncount = 1;
                }
            }

            if (ncount == 1) goto refresh;

            if (is_window_send == 1) {
                while (get_scancode() != 0); 
                for (int i = 0; i < 16; i++) byte_str[i] = 0;
                for (volatile int j = 0; j < 200000; j++);

                while (1) {
                    ncount = 0; 

                    int wx = 296; 
                    int wy = 244;

                    x = wx + 26;
                    y = wy + 60;
                    input_wait_string(byte_str);
                    if (byte_str[0] == '\0') {
                        ncount = 0;
                        continue;
                    }
                    break;
                }

                int byte_val = atoi_super(byte_str);
                uint8_t dest_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
                send_pack((uint8_t)byte_val, dest_mac);
                is_window_send = 0;
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
    print("Booting CalcOS...\n", 15);

    print("Scanning devices...\n", 15);
    pci_scan();

    init_mouse();
    print("[OK]\n", 15);
    delay_ticks(10);

    init_memory_manager();
    print("[OK]\n", 15);
    delay_ticks(10);

    int is_rtl8139_found = rtl8139_find();
    if (is_rtl8139_found) {
        rtl8139_init();
        print("[OK]\n", 15);
    } else {
        print("[Failed to find RTL8139]\n", 15);
    }

    screen_clear();
    init_palette();
    is_scaled = 0;
    print("Success!", 15);
    is_scaled = 1;
    delay_ticks(15);

    draw_rect(0, 0, 1024, 768, 15);

    x = 0;
    y = 10;

    print("Welcome to CalcOS!", 0);
    play_startup_sound();

    is_scaled = 0;
    __asm__ __volatile__("sti");
}

void __attribute__((section(".text.entry"))) kernel_main() {
    task_list[3].is_active = 0;
    asm volatile("cli");
	screen_clear();
    init_idt();
    asm volatile("sti");
    boot();
    task_list[3].is_active = 1;
    system();
}
