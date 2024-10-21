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
static inline void set_satp(unsigned mode, unsigned asid, unsigned long pa)
{
	unsigned long __v =
		(unsigned long)(((unsigned long)mode << SATP_MODE_SHIFT) | ((unsigned long)asid << SATP_ASID_SHIFT) | pa >> 12);
	__asm__ __volatile__("sfence.vma\ncsrw satp, %0" : : "rK"(__v) : "memory");
}

/* Sv-39 mode
 * 0x0000_0000_0000_0000-0x0000_003f_ffff_ffff is for user mode
 * 0xffff_ffc0_0000_0000-0xffff_ffff_ffff_ffff is for kernel mode
 */
constexpr ptr_t PGDIR_PA = 0x51000000; // use 51000000 page as PGDIR
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
	early_pgdir[get_vpn(base, 2)] = PageEntry{.ppn = base >> 12, .G = 1, .U = 0, .XWR = PageAttr::RWX};
	early_pgdir[get_vpn(pa2kva(base), 2)] = PageEntry{.ppn = base >> 12, .G = 1, .U = 0, .XWR = PageAttr::RWX};
}
void ARRTIBUTE_BOOTKERNEL clear_temp_vm(PageEntry root[512])
{
	constexpr ptr_t base = 0x40000000;
	root[get_vpn(pa2kva(base), 2)] = (PageEntry)0;
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
	memcpy(root, (void *)pa2kva(PGDIR_PA), 4096);
	clear_temp_vm(root);
}
PageDir::~PageDir()
{
}
void PageDir::enable(int asid)
{
	set_satp(SATP_MODE_SV39, asid, kva2pa((ptr_t)root));
	local_flush_tlb_all();
}
void PageDir::flushcpu(int cpu_id, int asid)
{
	size_t old = flush_mask.fetch_and(~(1 << cpu_id));
	if ((old >> cpu_id) & 1)
		local_flush_tlb_asid(asid);
}

PageEntry *PageDir::lookup(ptr_t va)
{
	PageEntry *pte = root + get_vpn(va, 2);
	if (!pte->V)
	{
		ptr_t pdir = (ptr_t)kalloc(PAGE_SIZE);
		bzeropage((void *)pdir, 1);
		*pte = PageEntry{.ppn = pdir >> 12, .XWR = PageAttr::Noleaf};
	}
	pte = (PageEntry *)pte[get_vpn(va, 1)].to_kva();
	if (!pte->V)
	{
		ptr_t pdir = (ptr_t)kalloc(PAGE_SIZE);
		bzeropage((void *)pdir, 1);
		*pte = PageEntry{.ppn = pdir >> 12, .XWR = PageAttr::Noleaf};
	}
	pte = (PageEntry *)pte[get_vpn(va, 0)].to_kva();
	return pte;
}
// return new alloced address for the va, address is in va
void PageDir::map_va_kva(ptr_t va, ptr_t kva)
{
	PageEntry *pte = lookup(va);
	*pte = PageEntry{.ppn = kva2pa(kva) >> 12, .XWR = PageAttr::RWX};
	local_flush_tlb_addr(va);
	flush_mask = -1;
}

ptr_t PageDir::alloc_page_for_va(ptr_t va)
{
	ptr_t kva = (ptr_t)kalloc(PAGE_SIZE);
	map_va_kva(va, kva);
	return kva;
}