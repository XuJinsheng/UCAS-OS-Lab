#include <stdlib.h>
#include <unistd.h>

static size_t CeilPower2(size_t x)
{
	x--;
	x |= x >> 1;
	x |= x >> 2;
	x |= x >> 4;
	x |= x >> 8;
	x |= x >> 16;
	x |= x >> 32;
	x++;
	return x;
}
size_t maxul(size_t a, size_t b)
{
	return a > b ? a : b;
}

const size_t mem_begin = 1ul << 30, mem_end = 2ul << 30;
const size_t batch_size = 4096, capacity = (mem_end - mem_begin) / batch_size;
static size_t *longest = (size_t *)0x300000;
static void pool_init()
{
	for (size_t i = 2 * capacity - 2; i > capacity - 2; i--)
		longest[i] = 1;
	for (size_t i = 2 * capacity - 2; i > 0; i--)
		longest[(i - 1) / 2] = longest[i] * 2;
}
static size_t pool_alloc(size_t size) // cnt is power of 2
{
	if (size == 0)
		size = 1;
	size_t nodesize = capacity;
	size_t index = 0;
	while (nodesize > size)
	{
		nodesize /= 2;
		size_t lc = index * 2 + 1, rc = index * 2 + 2;
		if (longest[rc] < size || (size <= longest[lc] && longest[lc] <= longest[rc]))
			index = lc;
		else
			index = rc;
	}
	longest[index] = 0;
	size_t offset = (index + 1) * nodesize - capacity;
	while (index)
	{
		index = (index - 1) / 2;
		longest[index] = maxul(longest[index * 2 + 1], longest[index * 2 + 2]);
	}
	return offset;
}
static size_t pool_free(size_t offset)
{
	size_t index = offset + capacity - 1;
	size_t nodesize;
	for (nodesize = 1; longest[index]; index = (index - 1) / 2)
	{
		nodesize *= 2;
	}
	longest[index] = nodesize;
	size_t free_size = nodesize;
	while (index)
	{
		index = (index - 1) / 2;
		nodesize *= 2;
		size_t ll = longest[index * 2 + 1], rl = longest[index * 2 + 2];
		if (ll + rl == nodesize)
			longest[index] = nodesize;
		else
			longest[index] = maxul(ll, rl);
	}
	return free_size;
}

void *malloc(size_t size)
{
	size = CeilPower2(size);
	return (void *)(mem_begin + pool_alloc(size / batch_size) * batch_size);
}
void free(void *ptr)
{
	size_t offset = (size_t)ptr - mem_begin;
	offset /= batch_size;
	pool_free(offset);
}
void __crt_init()
{
	sys_brk((void *)mem_end);
	pool_init();
}