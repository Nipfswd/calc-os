#ifndef DISK_H
#define DISK_H

#define APPS_START_SECTOR 150  
#define MAX_APPS 16           
#define APP_DATA_START 200
#define MAX_FILES 16
#define MAX_SECTORS 100

struct File {
	char name[32];
	char content[476];
	int exists;
};

struct App {
	char name[32];
	int start_lba;    
	int size_sectors; 
	int exists;
};

extern struct File files[MAX_FILES];
extern struct App apps[MAX_APPS];

void run_app(const char* name);

#endif