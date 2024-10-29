#ifndef ASSERT_H
#define ASSERT_H

#include <kstdio.h>

[[noreturn]] static inline void _panic(const char *file_name, int lineno, const char *func_name)
{
	printl("%s:%d:%s: Assertion failed\n\r", file_name, lineno, func_name);
	printk("%s:%d:%s: Assertion failed\n\r", file_name, lineno, func_name);
	for (;;)
		;
}

#define assert(cond)                                                                                                   \
	{                                                                                                                  \
		if (!(cond))                                                                                                   \
		{                                                                                                              \
			_panic(__FILE__, __LINE__, __FUNCTION__);                                                                  \
		}                                                                                                              \
	}

#endif /* ASSERT_H */
