#include <assert.h>
#include <kalloc.hpp>

#define MEM_SIZE 32
#define INIT_KERNEL_STACK 0x50500000
#define INIT_USER_STACK 0x52500000
#define FREEMEM_KERNEL (INIT_KERNEL_STACK + PAGE_SIZE)
#define FREEMEM_USER INIT_USER_STACK

namespace std
{
void terminate() noexcept
{
	assert(false);
}
void __throw_length_error(char const *s)
{
	printl(s);
	assert(false);
}
void __throw_bad_alloc()
{
	printl("bad alloc");
	assert(false);
}
void __throw_bad_array_new_length()
{
	printl("bad array new length");
	assert(false);
}

} // namespace std

constexpr size_t cal_align(size_t a, size_t n)
{
	return (a + n - 1) & -n;
}
static ptr_t kernMemCurr = FREEMEM_KERNEL;
static ptr_t userMemCurr = FREEMEM_USER;

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

void init_kernel_heap()
{
}

void *kalloc(size_t size, size_t align)
{
	ptr_t ret = cal_align(kernMemCurr, 16);
	kernMemCurr = ret + size;
	return (void *)ret;
}
void kfree(void *ptr)
{
	// do nothing
}

void *allocKernelPage(int numPage)
{
	// align PAGE_SIZE
	ptr_t ret = cal_align(kernMemCurr, PAGE_SIZE);
	kernMemCurr = ret + numPage * PAGE_SIZE;
	return (void *)ret;
}

void *allocUserPage(int numPage)
{
	// align PAGE_SIZE
	ptr_t ret = cal_align(userMemCurr, PAGE_SIZE);
	userMemCurr = ret + numPage * PAGE_SIZE;
	return (void *)ret;
}
