
#pragma once

#include <common.h>

extern void init_kernel_heap();

extern void *kalloc(size_t size, size_t align = 16);
extern void kfree(void *ptr);

#define PAGE_SIZE 4096 // 4K
extern void *allocKernelPage(int numPage);
extern void *allocUserPage(int numPage);
extern void freeUserPage(void *ptr);
