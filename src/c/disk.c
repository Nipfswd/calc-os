#include <disk.h>
#include <stdio.h>
#include <mouse.h>
#include <utils.h>
#include <keyboard.h>

struct File files[MAX_FILES] __attribute__((section(".bss.safe"), aligned(4)));
struct App apps[MAX_APPS] __attribute__((section(".bss.safe"), aligned(4)));

void run_app(const char *name) {
	int found = 0;

	for (int i = 0; i < MAX_APPS; i++) {
		if (apps[i].exists == 1 && compare_strings(name, apps[i].name)) {
			void (*run_me)() = (void (*)())0x100000;
			run_me();
			found = 1;
			break;
		}
	}
	if (!found) {
		print("App not found!\n", 0x0C);
	}
}