#include <arch/CSR.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <page.hpp>
#include <string.h>

typedef uint64_t PTE;

constexpr ptr_t get_vpn(ptr_t va, unsigned sizeid)
{
	va >>= 12 + 9 * sizeid;
	return va & 0x1ff;
}

static inline void local_flush_tlb_all(void)
{
	__asm__ __volatile__("sfence.vma" : : : "memory");
}
static inline void local_flush_tlb_addr(unsigned long addr)
{
	__asm__ __volatile__("sfence.vma %0" : : "r"(addr) : "memory");
}
static inline void local_flush_tlb_asid(unsigned long addr)
{
	__asm__ __volatile__("sfence.vma x0,%0" : : "r"(addr) : "memory");
}
static inline void local_flush_icache_all(void)
{
	asm volatile("fence.i" ::: "memory");
}

#define SATP_MODE_SV39 8
#define SATP_MODE_SV48 9
#define SATP_ASID_SHIFT 44lu
#define SATP_MODE_SHIFT 60lu
static inline void set_satp(unsigned long mode, unsigned long asid, unsigned long pa)
{
	csr_write(CSR_SATP, ((mode << SATP_MODE_SHIFT) | (asid << SATP_ASID_SHIFT) | pa >> 12));
}

/* Sv-39 mode
 * 0x0000_0000_0000_0000-0x0000_003f_ffff_ffff is for user mode
 * 0xffff_ffc0_0000_0000-0xffff_ffff_ffff_ffff is for kernel mode
 */
constexpr ptr_t PGDIR_PA = 0x50510000;
#define ARRTIBUTE_BOOTKERNEL __attribute__((section(".bootkernel")))
void ARRTIBUTE_BOOTKERNEL enable_vm()
{
	// write satp to enable paging
	set_satp(SATP_MODE_SV39, 0, PGDIR_PA);
	local_flush_tlb_all();
}
void ARRTIBUTE_BOOTKERNEL setup_vm()
{
	// map 0x4000_0000-0x8000_0000 to 0x4000_0000-0x8000_0000 for boot use
	// map 0x4000_0000-0x8000_0000 to 0xffff_ffc_4000_0000+0x4000_000 for kernel use
	PageEntry *early_pgdir = (PageEntry *)PGDIR_PA;
	bzeropage(early_pgdir, 1);
	constexpr ptr_t base = 0x40000000;
	early_pgdir[get_vpn(base, 2)] = PageEntry{.XWR = PageAttr::RWX, .U = 0, .G = 1, .ppn = base >> 12};
	early_pgdir[get_vpn(pa2kva(base), 2)] = PageEntry{.XWR = PageAttr::RWX, .U = 0, .G = 1, .ppn = base >> 12};
}
void set_kernel_vm(PageEntry root[512])
{
	bzeropage(root, 1);
	constexpr ptr_t base = 0x40000000;
	root[get_vpn(pa2kva(base), 2)] = PageEntry{.XWR = PageAttr::RWX, .U = 0, .G = 1, .ppn = base >> 12};
}

extern uintptr_t _start[];
extern "C" int ARRTIBUTE_BOOTKERNEL boot_kernel(unsigned long mhartid)
{
	if (mhartid == 0)
	{
		setup_vm();
	}
	enable_vm();

	typedef void (*kernel_entry_t)(unsigned long);
	((kernel_entry_t)pa2kva((ptr_t)_start))(mhartid);

	return 0;
}

PageDir::PageDir()
{
	root = (PageEntry *)kalloc(PAGE_SIZE);
	set_kernel_vm(root);
}
void PageDir::enable(int cpu_id, int asid)
{
	set_satp(SATP_MODE_SV39, asid, kva2pa((ptr_t)root));
	flushcpu(cpu_id, asid);
}
void PageDir::flushcpu(int cpu_id, int asid)
{
	local_flush_tlb_all();
	/* size_t old = flush_mask.fetch_and(~(1 << cpu_id));
	if ((old >> cpu_id) & 1)
		local_flush_tlb_asid(asid); */
}

PageEntry *PageDir::lookup(ptr_t va)
{
	PageEntry *pte = root + get_vpn(va, 2);
	if (pte->XWR != PageAttr::Noleaf)
		return pte;
	else if (!pte->V)
	{
		ptr_t pdir = (ptr_t)kalloc(PAGE_SIZE);
		bzeropage((void *)pdir, 1);
		pte->set_as_dir(kva2pa(pdir));
	}
	pte = ((PageEntry *)(pte->to_kva())) + get_vpn(va, 1);
	if (pte->XWR != PageAttr::Noleaf)
		return pte;
	else if (!pte->V)
	{
		ptr_t pdir = (ptr_t)kalloc(PAGE_SIZE);
		bzeropage((void *)pdir, 1);
		pte->set_as_dir(kva2pa(pdir));
	}
	pte = ((PageEntry *)(pte->to_kva())) + get_vpn(va, 0);
	return pte;
}
void PageDir::map_va_kva(ptr_t va, ptr_t kva)
{
	PageEntry *pte = lookup(va);
	assert(pte->V == false);
	pte->set_as_leaf(kva2pa(kva));
	flush_mask = -1;
}

ptr_t PageDir::alloc_page_for_va(ptr_t va)
{
	PageEntry *pte = lookup(va);
	if (pte->V)
		return (ptr_t)pte->to_kva();
	ptr_t kva = (ptr_t)kalloc(PAGE_SIZE);
	map_va_kva(va, kva);
	return kva;
}
static void freeUserRecur(PageEntry pte[512])
{
	for (int i = 0; i < 512; i++)
	{
		if (pte[i].V && pte[i].XWR == PageAttr::Noleaf)
			freeUserRecur((PageEntry *)pte[i].to_kva());
		else if (pte[i].V && pte[i].U && pte[i].OSflag == PageOSFlag::Normal)
		{
			kfree((void *)pte[i].to_kva());
		}
		else if (pte[i].V && pte[i].U && pte[i].OSflag == PageOSFlag::Swapped)
		{
			// sdcardfree(pte[i].ppn);
		}
		else
			assert(!pte[i].V || !pte[i].U);
		pte[i].V = 0;
	}
}
void PageDir::free_user_private_mem()
{
	freeUserRecur(root);
}