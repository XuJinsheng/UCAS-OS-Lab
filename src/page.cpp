#include <arch/CSR.h>
#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <container.hpp>
#include <kalloc.hpp>
#include <page.hpp>
#include <string.h>
#include <syscall.hpp>
#include <thread.hpp>
#include <time.hpp>

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
	early_pgdir[get_vpn(base, 2)] =
		PageEntry{.XWR = PageAttr::RWX, .U = 0, .G = 1, .OSflag = PageOSFlag::Normal, .ppn = base >> 12};
	early_pgdir[get_vpn(pa2kva(base), 2)] =
		PageEntry{.XWR = PageAttr::RWX, .U = 0, .G = 1, .OSflag = PageOSFlag::Normal, .ppn = base >> 12};
}
void set_kernel_vm(PageEntry root[512]) // set kernel space for process
{
	bzeropage(root, 1);
	constexpr ptr_t base = 0x40000000;
	memcpy(root, (void *)pa2kva(PGDIR_PA), PAGE_SIZE);
	root[get_vpn(base, 2)].clear(); // donot modify origin mapping for other hart's booting
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
void PageDir::updated()
{
	flush_mask = -1;
	current_process->send_intr_to_running_thread();
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
void PageDir::map_va_kva(ptr_t va, ptr_t kva, PageOSFlag osflag)
{
	PageEntry *pte = lookup(va);
	assert(pte->V == false);
	pte->set_as_leaf(kva2pa(kva));
	pte->OSflag = osflag;
	updated();
}

ptr_t PageDir::alloc_page_for_va(ptr_t va)
{
	if (va >= user_mem_bound)
	{
		printk("User memory out of bound\n");
		assert(0);
		return 0;
	}
	lock_guard guard(lock);
	// if (free_heap_size < 128 * 1024 * 1024 && active_private_mem > 64 * 1024 * 1024)
	if (active_private_mem > 64 * 1024 * 1024)
	{
		PageEntry *pte;
		do
		{
			ptr_t evict_va = private_mem_fifo.front();
			private_mem_fifo.pop();
			pte = lookup(evict_va);
		} while (!(pte->V && pte->U && pte->XWR != PageAttr::Noleaf && pte->OSflag == PageOSFlag::Normal));
		size_t sdcard_page = sdcard_alloc(8);
		bios_sd_write((void *)pte->to_pa(), 8, sdcard_page);
		kfree((void *)pte->to_kva());
		pte->set_as_swapped(sdcard_page);
		active_private_mem -= PAGE_SIZE;
	}
	PageEntry *pte = lookup(va);
	if (pte->V)
		return (ptr_t)pte->to_kva();
	ptr_t kva = (ptr_t)kalloc(PAGE_SIZE);
	if (pte->OSflag == PageOSFlag::Swapped)
	{
		size_t sdcard_page = pte->to_sdcard_page();
		bios_sd_read((void *)kva2pa(kva), 8, sdcard_page);
		sdcard_free(sdcard_page);
	}
	map_va_kva(va, kva, PageOSFlag::Normal);
	active_private_mem += PAGE_SIZE;
	private_mem_fifo.push(va);
	return (ptr_t)pte->to_kva();
}
static void freeUserRecur(PageEntry pdir[512])
{
	for (int i = 0; i < 512; i++)
	{
		if (pdir[i].V && pdir[i].U && pdir[i].XWR == PageAttr::Noleaf)
		{
			freeUserRecur((PageEntry *)pdir[i].to_kva());
		}
		else if (pdir[i].V && pdir[i].U && pdir[i].OSflag == PageOSFlag::Normal)
		{
			kfree((void *)pdir[i].to_kva());
		}
		else if (pdir[i].U && pdir[i].OSflag == PageOSFlag::Swapped)
		{
			sdcard_free(pdir[i].to_sdcard_page());
		}
		else
			assert(!pdir[i].V || !pdir[i].U);
	}
	kfree(pdir);
}
void PageDir::free_user_private_mem()
{
	freeUserRecur(root);
}

ptr_t PageDir::attach_shared_page(ptr_t kva)
{
	lock_guard guard(lock);
	ptr_t va = shared_page_start;
	map_va_kva(va, kva, PageOSFlag::Shared);
	shared_page_start += PAGE_SIZE;
	updated();
	return va;
}
ptr_t PageDir::free_shared_page(ptr_t va)
{
	lock_guard guard(lock);
	PageEntry *pte = lookup(va);
	assert(pte->V);
	pte->V = 0;
	updated();
	return pte->to_kva();
}

class SharedPage : KernelObject
{
public:
	const ptr_t key, kva;
	ptr_t cnt;
	SharedPage(ptr_t key) : key(key), kva((ptr_t)kalloc(PAGE_SIZE)), cnt(0)
	{
	}
	~SharedPage()
	{
		kfree((void *)kva);
	}
};
std::vector<SharedPage *> shared_pages;
ptr_t Syscall::sys_shmpageget(int key)
{
	auto it = std::find_if(shared_pages.begin(), shared_pages.end(), [key](SharedPage *p) { return p->key == key; });
	if (it == shared_pages.end())
	{
		shared_pages.push_back(new SharedPage(key));
		it = shared_pages.end() - 1;
	}
	(*it)->cnt++;
	return current_process->pageroot.attach_shared_page((*it)->kva);
}
void Syscall::sys_shmpagedt(ptr_t va)
{
	ptr_t kva = current_process->pageroot.free_shared_page(va);
	for (auto it = shared_pages.begin(); it != shared_pages.end(); it++)
	{
		if ((*it)->kva == kva)
		{
			(*it)->cnt--;
			if ((*it)->cnt == 0)
			{
				delete *it;
				shared_pages.erase(it);
			}
			break;
		}
	}
}

int Syscall::sys_brk(void *addr)
{
	current_process->pageroot.set_user_mem_bound((ptr_t)addr);
	return 1;
}
void PageDir::set_user_mem_bound(ptr_t addr)
{
	user_mem_bound = addr;
}
void *Syscall::sys_sbrk(intptr_t increment)
{
	return (void *)current_process->pageroot.increase_active_private_mem(increment);
}
ptr_t PageDir::increase_active_private_mem(size_t size)
{
	ptr_t old = user_mem_bound;
	user_mem_bound += size;
	return old;
}