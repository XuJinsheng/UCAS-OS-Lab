#ifndef _XJS_COMMON_H
#define _XJS_COMMON_H

#include <stddef.h>
#include <stdint.h>
typedef uintptr_t ptr_t;
typedef void (*func_t)(void);

#define __used __attribute__((__used__))

#ifndef __BEGIN_DECLS
#ifdef __cplusplus
#define __BEGIN_DECLS                                                                                                  \
	extern "C"                                                                                                         \
	{
#define __END_DECLS }
#else
#define __BEGIN_DECLS                                                                                                  \
	extern                                                                                                             \
	{
#define __END_DECLS }
#endif
#endif

#ifdef __cplusplus
#define _NEW

inline void *operator new(size_t, void *place)
{
	return place;
}

inline void *operator new[](size_t, void *place)
{
	return place;
}
#include <bits/c++config.h>

#include <queue>

#include <vector>

#undef _GLIBCXX_HOSTED
#include <algorithm>
#include <ranges>

#endif
#endif