#ifndef INCLUDE_PRINTK_H_
#define INCLUDE_PRINTK_H_

#include <common.h>
__BEGIN_DECLS

int getchar();

size_t getline(char *buf, size_t size);

int putstr(const char *);

/* kernel print */
int printk(const char *fmt, ...);

/* vt100 print */
int printv(const char *fmt, ...);

/* (QEMU-only) save print content to logfile */
int printl(const char *fmt, ...);

__END_DECLS

#endif
