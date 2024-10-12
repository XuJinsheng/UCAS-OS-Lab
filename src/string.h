#ifndef INCLUDE_STRING_H_
#define INCLUDE_STRING_H_

#include <common.h>
#include <stdint.h>

__BEGIN_DECLS

void memcpy(void *dest, const void *src, size_t len);
void memset(void *dest, uint8_t val, size_t len);
void *memmove(void *dest, const void *src, size_t count);
void bzero(void *dest, size_t len);
int strcmp(const char *str1, const char *str2);
int strncmp(const char *str1, const char *str2, size_t n);
char *strcpy(char *dest, const char *src);
char *strncpy(char *dest, const char *src, int n);
char *strcat(char *dest, const char *src);
size_t strlen(const char *src);

__END_DECLS
#endif
