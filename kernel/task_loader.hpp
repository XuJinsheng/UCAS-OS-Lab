#pragma once

#include <common.h>

#define TASK_MEM_BASE 0x52000000
#define TASK_MAXNUM 16
#define TASK_SIZE 0x10000


extern void	init_task_info();
extern uint64_t load_task_img(int taskid);
extern uint64_t load_task_img_by_name(const char *taskname);