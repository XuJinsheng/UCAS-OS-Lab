#pragma once

#include <common.h>
#include <container.hpp>
#include <page.hpp>
#include <process.hpp>
#include <spinlock.hpp>
struct user_context_reg_t
{
	ptr_t regs[31];
	ptr_t sstatus;
	ptr_t sepc;
	ptr_t scause;
	ptr_t stval;
};

class Thread
{
public:
	// fixed for asm
	user_context_reg_t user_context;
	ptr_t kernel_stack_top; // offset 280

	SpinLock thread_own_lock;
	Process *const process;
	const int tid;
	const int trank;

	Thread(Process *process, size_t trank);
	~Thread() = default;
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;

public:
	WaitQueue wait_exited_queue;
	CPU *running_cpu = nullptr;

	enum class Status
	{
		BLOCKED,
		RUNNING,
		READY,
		EXITED,
	};
	std::atomic<int> status_block_cnt = 0;
	bool status_exited = false;
	bool status_running = false;

	Status status() const
	{
		if (status_exited)
			return Status::EXITED;
		if (status_block_cnt)
			return Status::BLOCKED;
		if (status_running)
			return Status::RUNNING;
		return Status::READY;
	}
	void block() // need to be added to block queue manually
	{
		status_block_cnt++;
	}
	void wakeup();
	void exit();
	bool has_exited() const
	{
		return status_exited;
	}
};
static_assert(offsetof(Thread, kernel_stack_top) == 280, "Thread layout for asm error");
extern void print_threads(bool killed);