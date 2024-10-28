#ifndef __STDLIB_H__
#define __STDLIB_H__

#include <stddef.h>
#include <stdint.h>

#define RAND_MAX (INT32_MAX)

void srand(uint32_t seed);
int rand(void);

long atol(const char *str);
int atoi(const char *str);
int itoa(int num, char *str, int len, int base);

void *malloc(size_t size);
void free(void *ptr);

#endif