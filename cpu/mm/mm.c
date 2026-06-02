#include <stdint.h>
#include <mm.h>

#define BLOCK_SIZE sizeof(Block)
static Block* freeList = NULL;

void init_kernel_heap(void* start_addr, size_t heap_size) {
    freeList = (Block*)start_addr;
    freeList->size = heap_size - BLOCK_SIZE;
    freeList->free = 1;
    freeList->next = NULL;
}

void init_memory_manager() {
    init_kernel_heap((void*)HEAP_START, HEAP_SIZE);
}

void* kmalloc(size_t size) {
    Block* current = freeList;
    while (current) {
        if (current->free && current->size >= size) {
            if (current->size >= size + BLOCK_SIZE + 4) {
                Block* new_block = (Block*)((uint8_t*)current + BLOCK_SIZE + size);
                new_block->size = current->size - size - BLOCK_SIZE;
                new_block->free = 1;
                new_block->next = current->next;
                
                current->size = size;
                current->next = new_block;
            }
            current->free = 0;
            return (void*)((uint8_t*)current + BLOCK_SIZE);
        }
        current = current->next;
    }
    return NULL; 
}

void kfree(void* ptr) {
    if (!ptr) return;
    Block* block = (Block*)((uint8_t*)ptr - BLOCK_SIZE);
    block->free = 1;

    Block* current = freeList;
    while (current && current->next) {
        if (current->free && current->next->free) {
            current->size += BLOCK_SIZE + current->next->size;
            current->next = current->next->next;
        } else {
            current = current->next;
        }
    }
}