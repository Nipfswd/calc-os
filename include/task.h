#ifndef TASK_H
#define TASK_H
#include <stdint.h>

typedef struct {
    void* esp; 
    uint8_t id;
    uint8_t state;
    uint8_t is_active;
} Task;

extern Task task_list[4];
extern int current_task;

#endif 