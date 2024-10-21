#include <assert.h>
#include <kalloc.hpp>
#include <page.hpp>
#include <spinlock.hpp>

void *operator new(size_t size)
{
	return kalloc(size);
}

/* void* operator new(size_t size, std::nothrow_t const&) noexcept
{
	return kalloc(size);
} */

void operator delete(void *ptr) noexcept
{
	return kfree(ptr);
}

void operator delete(void *ptr, size_t) noexcept
{
	return kfree(ptr);
}

void *operator new[](size_t size)
{
	return kalloc(size);
}

/* void* operator new[](size_t size, std::nothrow_t const&) noexcept
{
	return kalloc(size);
} */

void operator delete[](void *ptr) noexcept
{
	return kfree(ptr);
}

void operator delete[](void *ptr, size_t) noexcept
{
	return kfree(ptr);
}

constexpr size_t CeilPower2(size_t x)
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
template <size_t capacity> struct BuddyPool
{
	using power_t = size_t;
	// size_t capacity;
	power_t longest[capacity * 2 - 1];
	void init(size_t size = capacity)
	{
		// capacity = size;
		for (size_t i = 2 * capacity - 2; i > capacity - 2; i--)
			longest[i] = 1;
		for (size_t i = 2 * capacity - 2; i > 0; i--)
			longest[(i - 1) / 2] = longest[i] * 2;
	}
	size_t alloc(size_t size) // cnt is power of 2
	{
		if (size == 0)
			size = 1;
		assert(longest[0] >= size);
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
			longest[index] = std::max(longest[index * 2 + 1], longest[index * 2 + 2]);
		}
		return offset;
	}
	void free(size_t offset)
	{
		assert(offset < capacity);
		size_t index = offset + capacity - 1;
		size_t nodesize;
		for (nodesize = 1; longest[index]; index = (index - 1) / 2)
		{
			nodesize *= 2;
			assert(index != 0);
		}
		longest[index] = nodesize;
		while (index)
		{
			index = (index - 1) / 2;
			nodesize *= 2;
			size_t ll = longest[index * 2 + 1], rl = longest[index * 2 + 2];
			if (ll + rl == nodesize)
				longest[index] = nodesize;
			else
				longest[index] = std::max(ll, rl);
		}
	}
};
template <size_t batch_size, size_t mem_start, size_t mem_end> class Allocator
{
	BuddyPool<(mem_end - mem_start) / batch_size> pool;

public:
	void init(size_t used = 0)
	{
		pool.init();
		if (used)
		{
			used = CeilPower2(used);
			size_t offset = pool.alloc(used / batch_size);
			assert(offset == 0);
		}
	}
	void *alloc(size_t size)
	{
		size = CeilPower2(size);
		size_t ptr = mem_start + pool.alloc(size / batch_size) * batch_size;
		return (void *)ptr;
	}
	void free(void *ptr)
	{
		size_t offset = (size_t)ptr - mem_start;
		assert(offset % batch_size == 0);
		offset /= batch_size;
		pool.free(offset);
	}
};

static constexpr ptr_t SMALL_BEGIN = 0x50800000, SMALL_END = 0x51000000;
static constexpr ptr_t PAGE_START = 0x51000000, PAGE_BEGIN = 0x50000000, PAGE_END = 0x60000000;
static constexpr ptr_t HEAP_STORAGE_BEGIN = 0x50510000 + 4096;
using SMALL_POOL = Allocator<128, SMALL_BEGIN, SMALL_END>;
using PAGE_POOL = Allocator<4096, PAGE_BEGIN, PAGE_END>;
static SMALL_POOL *ksmall;
static PAGE_POOL *kpage;

static_assert(HEAP_STORAGE_BEGIN + sizeof(SMALL_POOL) + sizeof(PAGE_POOL) < PAGE_START, "memory layout error");
void init_kernel_heap()
{
	ksmall = (SMALL_POOL *)pa2kva(HEAP_STORAGE_BEGIN);
	kpage = (PAGE_POOL *)(ksmall + 1);

	ksmall->init();
	kpage->init(PAGE_START - PAGE_BEGIN); // alloced for early page root dir
}

SpinLock alloc_lock;
void *kalloc(size_t size)
{
	lock_guard guard(alloc_lock);
	void *p;
	if (size > 1024)
		p = kpage->alloc(size);
	else
		p = ksmall->alloc(size);
	return (void *)pa2kva((ptr_t)p);
}
void kfree(void *ptr)
{
	lock_guard guard(alloc_lock);
	ptr = (void *)kva2pa((ptr_t)ptr);
	if ((ptr_t)ptr < PAGE_START)
		ksmall->free(ptr);
	else
		kpage->free(ptr);
}
