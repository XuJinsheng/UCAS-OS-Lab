#ifndef INCLUDE_PRINTK_H_
#define INCLUDE_PRINTK_H_

#include <stdarg.h>
#include "common.h"
__BEGIN_DECLS


/* kernel print */
int printk(const char *fmt, ...);

/* vt100 print */
int printv(const char *fmt, ...);

/* (QEMU-only) save print content to logfile */
int printl(const char *fmt, ...);

__END_DECLS

#endif
