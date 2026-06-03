#include <stdint.h>
#include <mm.h>

struct Stack {
    int *data;
    int size;
    int sp;
};

struct Stack* stack_init(int size) {
    struct Stack* st = kmalloc(sizeof(struct Stack));
    if (!st) {
        print("Forth: cannot allocate Stack", 15);
    }

    st->size = size;
    st->sp = -1;

    st->data = kmalloc(size * sizeof(int));
    if (!st->data) {
        print("Forth: cannot allocate stack data", 15);
    }
        
    return st;
}

void stack_push(struct Stack* st, int value) {
    if (st->sp == st->size - 1) {
        int new_size = st->size + 32;
        int* new_data = kmalloc(new_size * sizeof(int));
        if (!new_data) {
            print("Forth: cannot expand stack", 15);
        }

        for (int i = 0; i <= st->sp; i++) {
            new_data[i] = st->data[i];
        }

        kfree(st->data);
        st->data = new_data;
        st->size = new_size;
    }

    st->data[++st->sp] = value;
}