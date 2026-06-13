#ifndef CWL_H
#define CWL_H
#include <stdint.h>

typedef struct {
    char title[128];
    char main_text[1024];
} CWL_Doc;

void browser_main();


#endif 