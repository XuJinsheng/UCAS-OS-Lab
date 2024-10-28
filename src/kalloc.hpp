
#pragma once

#include <common.h>

extern void init_kernel_heap();

extern void *kalloc(size_t size);
extern void kfree(void *ptr);

extern size_t free_heap_size;

extern size_t sdcard_alloc(size_t size);
extern void sdcard_free(size_t ptr);
#define PAGE_SIZE 4096 // 4K
