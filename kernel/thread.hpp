#pragma once

#include <common.h>
struct user_context_reg_t
{
	ptr_t regs[31];
	ptr_t sstatus;
	ptr_t sepc;
	ptr_t scause;
	ptr_t stval;
};
struct kernel_context_reg_t
{
	ptr_t regs[14];
};

class Thread
{
public:
	// fixed for asm
	user_context_reg_t user_context;
	ptr_t kernel_stack_top;



	int cursor_x, cursor_y;
	int pid;

	enum
	{
		TASK_BLOCKED,
		TASK_RUNNING,
		TASK_READY,
		TASK_EXITED,
	} status;

	uint64_t wakeup_time;

	Thread(ptr_t start_address);
	~Thread() = default;
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;
};

extern void init_pcb();
register Thread * current_running asm("tp");
