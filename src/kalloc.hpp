
#pragma once

#include <common.h>

extern void init_kernel_heap();

extern void *kalloc(size_t size);
extern void kfree(void *ptr);

#define PAGE_SIZE 4096 // 4K
