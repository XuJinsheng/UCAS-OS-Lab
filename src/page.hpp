#pragma once
#include <common.h>
#include <spinlock.hpp>

/* Translation between physical addr and kernel virtual addr */
static constexpr uintptr_t kva2pa(uintptr_t kva)
{
	constexpr uintptr_t KERNEL_MASK = 0xffffffc000000000ul;
	return kva - KERNEL_MASK;
}
static constexpr uintptr_t pa2kva(uintptr_t pa)
{
	constexpr uintptr_t KERNEL_MASK = 0xffffffc000000000ul;
	return pa + KERNEL_MASK;
}

enum class PageAttr
{
	Noleaf = 0,
	R = 0b001,
	RW = 0b011,
	X = 0b100,
	RX = 0b101,
	RWX = 0b111
};

enum class PageOSFlag : uint8_t
{
	Normal = 0,
	Shared = 1,
	Swapped = 2,
	Reseverd = 3
};

struct [[gnu::packed]] alignas(size_t) PageEntry
{
	ptr_t V : 1 = 1;
	PageAttr XWR : 3;
	ptr_t U : 1 = 1;
	ptr_t G : 1 = 0;
	ptr_t A : 1 = 1;
	ptr_t D : 1 = 1;
	PageOSFlag OSflag : 2;
	ptr_t ppn : 44;
	ptr_t Reseverd1 : 10 = 0;
	void clear()
	{
		*(size_t *)this = 0;
	}
	void set_as_leaf(ptr_t pa)
	{
		V = 1;
		XWR = PageAttr::RWX;
		U = 1;
		G = 0;
		A = D = 1;
		OSflag = PageOSFlag::Normal;
		ppn = pa >> 12;
	}
	void set_as_dir(ptr_t pa)
	{
		V = 1;
		XWR = PageAttr::Noleaf;
		U = 0;
		G = 0;
		A = D = 0;
		OSflag = PageOSFlag::Normal;
		ppn = pa >> 12;
	}
	void set_as_swapped(ptr_t sdcard_page)
	{
		V = 0;
		OSflag = PageOSFlag::Swapped;
		ppn = sdcard_page;
	}
	size_t to_sdcard_page()
	{
		return ppn;
	}
	ptr_t to_pa()
	{
		return ppn << 12;
	}
	ptr_t to_kva()
	{
		return pa2kva(to_pa());
	}
};

class PageDir
{
	PageEntry *root;
	std::atomic<size_t> flush_mask; // flush_mask[cpu_id] indicates flush
	SpinLock lock;
	ptr_t shared_page_start = 128ul << 30;
	size_t active_private_mem = 0;
	std::queue<ptr_t> private_mem_fifo;
	size_t user_mem_bound = 0x300000;

public:
	PageDir();
	~PageDir() = default;
	void enable(int cpu_id, int asid);
	void flushcpu(int cpu_id, int asid);
	void updated();
	PageEntry *lookup(ptr_t va);
	void map_va_kva(ptr_t va, ptr_t kva, PageOSFlag osflag = PageOSFlag::Normal); // require va is not mapped
	ptr_t alloc_page_for_va(ptr_t va);											  // return kva
	void free_user_private_mem();
	ptr_t attach_shared_page(ptr_t kva); // return va
	ptr_t free_shared_page(ptr_t va);	 // return kva
	void set_user_mem_bound(size_t bound);
	ptr_t increase_active_private_mem(size_t size);
};