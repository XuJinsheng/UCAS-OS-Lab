#pragma once

#include <common.h>

extern void init_task_info();
extern uint64_t load_task_img(int taskid);
extern uint64_t load_task_img_by_name(const char *taskname);