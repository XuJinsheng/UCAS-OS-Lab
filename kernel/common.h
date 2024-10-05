#ifndef _XJS_COMMON_H
#define _XJS_COMMON_H

#include <stddef.h>
#include <stdint.h>
typedef uintptr_t ptr_t;
typedef void (*func_t)(void);

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS \
	extern "C"        \
	{

#define __END_DECLS }
#else
#define __BEGIN_DECLS
#define __END_DECLS
#endif
#endif

#endif