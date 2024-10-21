#pragma once

#include <common.h>
#include <page.hpp>

constexpr ptr_t USER_ENTRYPOINT = 0x200000;
extern void init_task_info(); // need early page table
extern bool load_task_img(int taskid, PageDir &pdir);
extern int find_task_idx_by_name(const char *taskname); //-1 for not found