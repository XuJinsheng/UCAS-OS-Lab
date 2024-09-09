#ifndef __INCLUDE_LOADER_H__
#define __INCLUDE_LOADER_H__

#include <type.h>

uint64_t load_task_img(int taskid);
uint64_t load_task_img_by_name(const char *taskname);
void task_interact();

#endif