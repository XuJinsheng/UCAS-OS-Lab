#pragma once

#include <common.h>

extern void init_kernel_heap();
extern void	init_task_info();
extern void	init_pcb();
extern void	init_locks();
extern void	init_trap();
extern void	init_syscall();
extern "C" void	init_screen();
