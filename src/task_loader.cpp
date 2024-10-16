#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <spinlock.hpp>
#include <string.h>

#define TASK_MEM_BASE 0x52000000
#define TASK_MAXNUM 16
#define TASK_SIZE 0x10000

// ATTENTION: The size of task_info_t must be 32 bytes
struct task_info_t
{
	unsigned long long entry_point;
	int sdcard_block_id;
	int sdcard_block_num;
	char name[16];
};

task_info_t tasks[TASK_MAXNUM];
bool task_loaded[TASK_MAXNUM];
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

uint64_t load_task_img(int taskid)
{
	assert(taskid < TASK_MAXNUM);
	uint64_t entry = tasks[taskid].entry_point;
	if (!task_loaded[taskid])
	{
		load_lock.lock();
		bios_sd_read((void *)entry, tasks[taskid].sdcard_block_num, tasks[taskid].sdcard_block_id);
		load_lock.unlock();
	}
	task_loaded[taskid] = true;
	return entry;
}

uint64_t load_task_img_by_name(const char *taskname)
{
	for (int i = 0; i < task_num; i++)
	{
		if (strcmp(tasks[i].name, taskname) == 0)
		{
			return load_task_img(i);
		}
	}
	return 0;
}
