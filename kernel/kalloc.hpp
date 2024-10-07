
#ifndef INCLUDE_KALLOC_H_
#define INCLUDE_KALLOC_H_

#include <common.h>

extern void init_kernel_heap();

extern void *kalloc(size_t size, size_t align = 16);
extern void kfree(void *ptr);

#define PAGE_SIZE 4096 // 4K
extern void *allocKernelPage(int numPage);
extern void *allocUserPage(int numPage);

template <typename T> class Kallocator
{
public:
	using value_type = T;
	using size_type = size_t;
	using difference_type = ptrdiff_t;
	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;

	template <typename U> struct rebind
	{
		typedef Kallocator<U> other;
	};

	Kallocator() noexcept
	{
	}
	Kallocator(const Kallocator &) noexcept
	{
	}
	template <typename U> Kallocator(const Kallocator<U> &) noexcept
	{
	}

	~Kallocator() noexcept
	{
	}

	pointer address(reference x) const noexcept
	{
		return &x;
	}
	const_pointer address(const_reference x) const noexcept
	{
		return &x;
	}

	pointer allocate(size_type n, const void *hint = 0)
	{
		return static_cast<pointer>(kalloc(n * sizeof(T)));
	}
	void deallocate(pointer p, size_type n)
	{
		kfree(p);
	}

	size_type max_size() const noexcept
	{
		return size_t(-1) / sizeof(T);
	}

	void construct(pointer p, const T &val)
	{
		new (p) T(val);
	}
	void destroy(pointer p)
	{
		p->~T();
	}
};

#endif /* INCLUDE_KALLOC_H_ */
