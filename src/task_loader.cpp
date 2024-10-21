#include "kalloc.hpp"
#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <spinlock.hpp>
#include <string.h>
#include <task_loader.hpp>

#define TASK_MAXNUM 16

// ATTENTION: The size of task_info_t must be 32 bytes
struct task_info_t
{
	unsigned long long entry_point;
	int sdcard_block_id;
	int sdcard_block_num;
	char name[16];
};

task_info_t tasks[TASK_MAXNUM];
short task_num = 0;

static_assert(sizeof(tasks) == SECTOR_SIZE);

SpinLock load_lock;

void init_task_info(void)
{
	// the sdcard block_id of task_info_t is stored in 0x502001f0
	// the number of tasks is stored in 0x502001f4
	// assert(sizeof(task_info_t) == 32);
	int block_id = *(int *)0x502001f0;
	bios_sd_read(tasks, 1, block_id);
	task_num = *(short *)0x502001f4;
}

bool load_task_img(int taskid, PageDir &pdir)
{
	if (taskid >= task_num)
		return false;
	size_t block_num = tasks[taskid].sdcard_block_num;
	size_t sdcard_id = tasks[taskid].sdcard_block_id;
	size_t va = USER_ENTRYPOINT;
	size_t pages = block_num / 8 + block_num != 0;
	load_lock.lock();
	for (size_t i = 0; i < pages; i++)
	{
		ptr_t kva = pdir.alloc_page_for_va(va);
		bios_sd_read((void *)kva2pa(kva), block_num > 8 ? 8 : block_num, sdcard_id);
		va += PAGE_SIZE;
		sdcard_id += 8;
		block_num -= 8;
	}
	load_lock.unlock();
	return true;
}
int find_task_idx_by_name(const char *taskname)
{
	for (int i = 0; i < task_num; i++)
	{
		if (strcmp(tasks[i].name, taskname) == 0)
		{
			return i;
		}
	}
	return -1;
}
