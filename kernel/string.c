#include <string.h>

void memcpy(uint8_t *dest, const uint8_t *src, size_t len)
{
	for (; len != 0; len--)
	{
		*dest++ = *src++;
	}
}

void *memmove(void *dest, const void *src, size_t count)
{
	char *tmp;
	const char *s;

	if (dest <= src)
	{
		tmp = (char *)dest;
		s = (char *)src;
		while (count--)
			*tmp++ = *s++;
	}
	else
	{
		tmp = (char *)dest;
		tmp += count;
		s = (char *)src;
		s += count;
		while (count--)
			*--tmp = *--s;
	}
	return dest;
}

void memset(void *dest, uint8_t val, size_t len)
{
	uint8_t *dst = (uint8_t *)dest;

	for (; len != 0; len--)
	{
		*dst++ = val;
	}
}

void bzero(void *dest, size_t len)
{
	memset(dest, 0, len);
}

size_t strlen(const char *src)
{
	int i = 0;
	while (src[i] != '\0')
	{
		i++;
	}
	return i;
}

int strcmp(const char *str1, const char *str2)
{
	while (*str1 && *str2)
	{
		if (*str1 != *str2)
		{
			return (*str1) - (*str2);
		}
		++str1;
		++str2;
	}
	return (*str1) - (*str2);
}

int strncmp(const char *str1, const char *str2, size_t n)
{
	for (size_t i = 0; i < n; ++i)
		if (str1[i] == '\0' || str1[i] != str2[i])
			return str1[i] - str2[i];
	return 0;
}

char *strcpy(char *dest, const char *src)
{
	char *tmp = dest;

	while (*src)
	{
		*dest++ = *src++;
	}

	*dest = '\0';

	return tmp;
}

char *strncpy(char *dest, const char *src, size_t n)
{
	char *tmp = dest;

	while (*src && n-- > 0)
	{
		*dest++ = *src++;
	}

	while (n-- > 0)
	{
		*dest++ = '\0';
	}

	return tmp;
}

char *strcat(char *dest, const char *src)
{
	char *tmp = dest;

	while (*dest != '\0')
	{
		dest++;
	}
	while (*src)
	{
		*dest++ = *src++;
	}

	*dest = '\0';

	return tmp;
}
