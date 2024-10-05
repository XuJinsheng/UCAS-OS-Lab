#include <kalloc.hpp>

#define MEM_SIZE 32
#define PAGE_SIZE 4096 // 4K
#define INIT_KERNEL_STACK 0x50500000
#define INIT_USER_STACK 0x52500000
#define FREEMEM_KERNEL (INIT_KERNEL_STACK + PAGE_SIZE)
#define FREEMEM_USER INIT_USER_STACK

/* Rounding; only works for n = power of two */
#define ROUND(a, n) (((((uint64_t)(a)) + (n) - 1)) & ~((n) - 1))
#define ROUNDDOWN(a, n) (((uint64_t)(a)) & ~((n) - 1))
static ptr_t kernMemCurr = FREEMEM_KERNEL;
static ptr_t userMemCurr = FREEMEM_USER;

void *kalloc(size_t size)
{
	size_t s = 1;
	while (s < size)
		s <<= 1;
	ptr_t ret = ROUND(kernMemCurr, s);
	kernMemCurr = ret + s;
	return (void *)ret;
}
void kfree(void *ptr)
{
	// do nothing
}
void *allocKernelPage(int numPage)
{
	// align PAGE_SIZE
	ptr_t ret = ROUND(kernMemCurr, PAGE_SIZE);
	kernMemCurr = ret + numPage * PAGE_SIZE;
	return (void *)ret;
}

void *allocUserPage(int numPage)
{
	// align PAGE_SIZE
	ptr_t ret = ROUND(userMemCurr, PAGE_SIZE);
	userMemCurr = ret + numPage * PAGE_SIZE;
	return (void *)ret;
}
