
#ifndef INCLUDE_KALLOC_H_
#define INCLUDE_KALLOC_H_

#include <common.h>

extern void init_kernel_heap();

extern void *kalloc(size_t size, size_t align = 16);
extern void kfree(void *ptr);

#define PAGE_SIZE 4096 // 4K
extern void *allocKernelPage(int numPage);
extern void *allocUserPage(int numPage);

#endif /* INCLUDE_KALLOC_H_ */
