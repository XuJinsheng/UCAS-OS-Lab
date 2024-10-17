#pragma once

#include <Trie.hpp>
#include <common.h>
#include <spinlock.hpp>
struct user_context_reg_t
{
	ptr_t regs[31];
	ptr_t sstatus;
	ptr_t sepc;
	ptr_t scause;
	ptr_t stval;
};
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

class KernelObject
{
private:
	friend Thread;
	std::atomic<size_t> ref_count = 0;

public:
	KernelObject() = default;
	KernelObject(const KernelObject &) = delete;
	KernelObject &operator=(const KernelObject &) = delete;
	virtual ~KernelObject() = default;
	virtual void on_thread_unregister(Thread *)
	{
		if (--ref_count == 0)
		{
			delete this;
		}
	}
};

class WaitQueue
{
	std::queue<Thread *> que;

public:
	void push(Thread *t)
	{
		que.push(t);
	}
	Thread *wakeup_one();
	void wakeup_all();
	~WaitQueue()
	{
		wakeup_all();
	}
};

class Thread
{
public:
	// fixed for asm
	user_context_reg_t user_context;
	ptr_t kernel_stack_top; // offset 280

	SpinLock thread_own_lock;

	Thread(Thread *parent, std::string name);
	~Thread() = default;
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;

private:
	TrieLookup<KernelObject *> kernel_objects;
	std::queue<void *> user_memory;

public:
	int cursor_x = 0, cursor_y = 0;
	const int pid;
	const std::string name;

	Thread *parent;
	std::vector<Thread *> children;

	WaitQueue wait_kill_queue;
	void *alloc_user_page(size_t numPage);
	bool register_kernel_object(KernelObject *obj) // true: insert success, false: already exists
	{
		if (kernel_objects.insert((size_t)obj, obj))
		{
			obj->ref_count++;
			return true;
		}
		return false;
	}
	bool unregister_kernel_object(KernelObject *obj) // true: remove success, false: not found
	{
		if (kernel_objects.remove((size_t)obj))
			return false;
		obj->on_thread_unregister(this);
		return true;
	}

public:
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

	size_t cpu_mask = -1;
	CPU *running_cpu = nullptr;

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
	void kill();
	bool is_exited() const
	{
		return status_exited;
	}
};
static_assert(offsetof(Thread, kernel_stack_top) == 280, "Thread layout for asm error");

extern void init_processor(size_t hartid);