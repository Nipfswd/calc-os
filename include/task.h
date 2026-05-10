#ifndef TASK_H
#define TASK_H

typedef struct {
    void* esp; 
    unsigned char id;
    unsigned char state;
} Task;

extern Task task_list[2];
extern int current_task;

#endif 