/*
 * The Minimal snprintf() implementation
 *
 * Copyright (c) 2013,2014 Michal Ludvig <michal@logix.cz>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the auhor nor the names of its contributors
 *       may be used to endorse or promote products derived from this software
 *       without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * ----
 *
 * This is a minimal snprintf() implementation optimised
 * for embedded systems with a very limited program memory.
 * mini_snprintf() doesn't support _all_ the formatting
 * the glibc does but on the other hand is a lot smaller.
 * Here are some numbers from my STM32 project (.bin file size):
 *      no snprintf():      10768 bytes
 *      mini snprintf():    11420 bytes     (+  652 bytes)
 *      glibc snprintf():   34860 bytes     (+24092 bytes)
 * Wasting nearly 24kB of memory just for snprintf() on
 * a chip with 32kB flash is crazy. Use mini_snprintf() instead.
 *
 */
#include <arch/bios_func.h>
#include <drivers/screen.h>
#include <kstdio.h>
#include <stdarg.h>

int getchar()
{
	int ch = -1;
	while (ch == -1)
	{
		ch = bios_getchar();
	}
	if (ch == '\r')
		ch = '\n';
	return ch;
}
size_t getline(char *buf, size_t size)
{
	size_t i = 0;
	while (i < size - 1)
	{
		buf[i] = getchar();
		bios_putchar(buf[i]);
		if (buf[i] == '\r' || buf[i] == '\n')
		{
			break;
		}
		i++;
	}
	buf[i] = '\0';
	return i;
}

int putstr(const char *s)
{
	screen_write(s);
	screen_reflush();
	return 0;
}

static unsigned int mini_strlen(const char *s)
{
	unsigned int len = 0;
	while (s[len] != '\0')
		len++;
	return len;
}

static unsigned int mini_itoa(long value, unsigned int radix, unsigned int uppercase, unsigned int unsig, char *buffer,
							  unsigned int n_pad, char cpad)
{
	/*
	 * Glucose180 modified: add `n_pad` and `cpad` to support
	 * padding with different chars ('0' or ' ', etc).
	 */
	char *pbuffer = buffer;
	int negative = 0;
	unsigned int i, len;

	/* No support for unusual radixes. */
	if (radix > 16)
		return 0;

	if (value < 0 && !unsig)
	{
		negative = 1;
		value = -value;
	}

	/* This builds the string back to front ... */
	do
	{
		int digit = 0;
		if (unsig)
		{
			digit = (unsigned long)value % (unsigned)radix;
		}
		else
		{
			digit = value % radix;
		}
		*(pbuffer++) = (digit < 10 ? '0' + digit : (uppercase ? 'A' : 'a') + digit - 10);
		if (unsig)
		{
			value = (unsigned long)value / (unsigned)radix;
		}
		else
		{
			value /= radix;
		}
	} while (value != 0);

	for (i = (pbuffer - buffer); i < n_pad; i++)
		*(pbuffer++) = cpad; //'0';

	if (negative)
		*(pbuffer++) = '-';

	*(pbuffer) = '\0';

	/* ... now we reverse it (could do it recursively but will
	 * conserve the stack space) */
	len = (pbuffer - buffer);
	for (i = 0; i < len / 2; i++)
	{
		char j = buffer[i];
		buffer[i] = buffer[len - i - 1];
		buffer[len - i - 1] = j;
	}

	return len;
}

struct mini_buff
{
	char *buffer, *pbuffer;
	unsigned int buffer_len;
};

static int _putc(int ch, struct mini_buff *b)
{
	if ((unsigned int)((b->pbuffer - b->buffer) + 1) >= b->buffer_len)
		return 0;
	*(b->pbuffer++) = ch;
	*(b->pbuffer) = '\0';
	return 1;
}

static int _puts(char *s, unsigned int len, struct mini_buff *b)
{
	unsigned int i;

	if (b->buffer_len - (b->pbuffer - b->buffer) - 1 < len)
		len = b->buffer_len - (b->pbuffer - b->buffer) - 1;

	/* Copy to buffer */
	if (s == NULL)
		/*
		 * Added by Glucose180: if `s` is `NULL`,
		 * pad the buffer with space ' '.
		 */
		for (i = 0U; i < len; ++i)
			*(b->pbuffer++) = ' ';
	else
		for (i = 0U; i < len; ++i)
			*(b->pbuffer++) = s[i];
	*(b->pbuffer) = '\0';

	return len;
}

static int mini_vsnprintf(char *buffer, unsigned int buffer_len, const char *fmt, va_list va)
{
	struct mini_buff b;
	char bf[24];
	char ch;

	b.buffer = buffer;
	b.pbuffer = buffer;
	b.buffer_len = buffer_len;

	while ((ch = *(fmt++)))
	{
		if ((unsigned int)((b.pbuffer - b.buffer) + 1) >= b.buffer_len)
			break;
		if (ch != '%')
			_putc(ch, &b);
		else
		{
			unsigned int n_pad = 0U;
			char c_pad = ' ';
			char longflag = 0;
			char *ptr;
			unsigned int len;

			ch = *(fmt++);

			/*
			 * Zero or space padding requested:
			 * space padding is added by Glucose180.
			 */
			if (ch >= '0' && ch <= '9')
			{
				c_pad = (ch == '0' ? '0' : ' ');
				--fmt;
				while ((ch = *(fmt++)))
				{
					if (ch == '\0')
						goto end;
					if (ch >= '0' && ch <= '9')
					{
						n_pad = n_pad * 10 + ch - '0';
					}
					else
					{
						break;
					}
				}
			}
			if (ch == 'l')
			{
				longflag = 1;
				ch = *(fmt++);
			}

			switch (ch)
			{
			case 0:
				goto end;

			case 'l':
				longflag = 1;
				break;

			case 'u':
				len = mini_itoa(longflag == 0 ? (unsigned long)va_arg(va, unsigned int) : va_arg(va, unsigned long), 10,
								0, (ch == 'u'), bf, n_pad, c_pad);
				_puts(bf, len, &b);
				longflag = 0;
				break;
			case 'd':
				len = mini_itoa(longflag == 0 ? (long)va_arg(va, int) : va_arg(va, unsigned long), 10, 0, (ch == 'u'),
								bf, n_pad, c_pad);
				_puts(bf, len, &b);
				longflag = 0;
				break;
			case 'x':
			case 'X':
				len = mini_itoa(longflag == 0 ? (unsigned long)va_arg(va, unsigned int) : va_arg(va, unsigned long), 16,
								(ch == 'X'), 1, bf, n_pad, c_pad);
				_puts(bf, len, &b);
				longflag = 0;
				break;

			case 'c':
				_putc((char)(va_arg(va, int)), &b);
				break;

			case 's':
				ptr = va_arg(va, char *);
				_puts(ptr, len = mini_strlen(ptr), &b);
				if (len < n_pad)
					/* Space padding */
					_puts(NULL, n_pad - len, &b);
				break;

			default:
				_putc(ch, &b);
				break;
			}
		}
	}
end:
	return b.pbuffer - b.buffer;
}

static int _vprint(const char *fmt, va_list _va, void (*output)(const char *))
{
	va_list va;
	va_copy(va, _va);

	int ret;
	char buff[256];

	ret = mini_vsnprintf(buff, 256, fmt, va);

	buff[ret] = '\0';

	output(buff);

	return ret;
}

static void _output_wrapper(const char *buff)
{
	screen_write(buff);
	screen_reflush();
}

int vprintk(const char *fmt, va_list _va)
{
	return _vprint(fmt, _va, _output_wrapper);
}

int printk(const char *fmt, ...)
{
	int ret = 0;
	va_list va;

	va_start(va, fmt);
	ret = vprintk(fmt, va);
	va_end(va);

	return ret;
}

int printv(const char *fmt, ...)
{
	int ret = 0;
	va_list va;

	va_start(va, fmt);
	ret = _vprint(fmt, va, bios_putstr);
	va_end(va);

	return ret;
}

int printl(const char *fmt, ...)
{
	int ret = 0;
	va_list va;

	va_start(va, fmt);
	ret = _vprint(fmt, va, bios_logging);
	va_end(va);

	return ret;
}
