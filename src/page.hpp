#pragma once
#include <common.h>

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

struct [[gnu::packed]] alignas(size_t) PageEntry
{
	ptr_t Reseverd1 : 9 = 0;
	ptr_t ppn : 44;
	ptr_t RSW : 2 = 0;
	ptr_t D : 1 = 1;
	ptr_t A : 1 = 1;
	ptr_t G : 1 = 0;
	ptr_t U : 1 = 1;
	PageAttr XWR : 3;
	ptr_t V : 1 = 1;

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

public:
	PageDir();
	~PageDir();
	void enable(int asid);
	void flushcpu(int cpu_id, int asid);
	PageEntry *lookup(ptr_t va);
	void map_va_kva(ptr_t va, ptr_t kva);
	ptr_t alloc_page_for_va(ptr_t va); // return kva
};