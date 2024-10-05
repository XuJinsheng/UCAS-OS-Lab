
#ifndef INCLUDE_KALLOC_H_
#define INCLUDE_KALLOC_H_

#include "common.h"

extern void *kalloc(size_t size);
extern void kfree(void *ptr);

extern void *allocKernelPage(int numPage);
extern void *allocUserPage(int numPage);

#endif /* INCLUDE_KALLOC_H_ */
