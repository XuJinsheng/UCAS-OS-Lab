#pragma once

#include <common.h>

class Thread;

class CPU
{
public:
	Thread *current_thread;
	ptr_t scratch_for_asm;

	Thread *idle_thread;
	size_t cpu_id, hartid;
	size_t own_spinlock_count = 0;
};
static_assert(offsetof(CPU, scratch_for_asm) == 8, "Processor layout for asm error");
register CPU *current_cpu asm("tp");