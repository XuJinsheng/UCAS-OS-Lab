#pragma once

#include <Trie.hpp>
#include <common.h>
struct user_context_reg_t
{
	ptr_t regs[31];
	ptr_t sstatus;
	ptr_t sepc;
	ptr_t scause;
	ptr_t stval;
};
class Thread;

class KernelObject
{
private:
	friend Thread;

public:
	size_t ref_count = 0;
	KernelObject() = default;
	KernelObject(const KernelObject &) = delete;
	KernelObject &operator=(const KernelObject &) = delete;
	virtual ~KernelObject() = default;
	virtual void on_thread_unregister(Thread *)
	{
		ref_count--;
		if (ref_count == 0)
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

	int cursor_x, cursor_y;
	const int pid;

	enum class Status
	{
		BLOCKED,
		RUNNING,
		READY,
		EXITED,
	} status;
	Thread *parent;
	std::vector<Thread *> children;

	const std::string name;
	Thread(Thread *parent, std::string name);
	~Thread() = default;
	Thread(const Thread &) = delete;
	Thread &operator=(const Thread &) = delete;

private:
	TrieLookup<KernelObject *> kernel_objects;
	std::queue<void *> user_memory;

public:
	WaitQueue wait_kill_queue;
	void *alloc_user_page(size_t numPage);
	bool register_kernel_object(KernelObject *obj) // true: insert success, false: already exists
	{
		if (!kernel_objects.insert((size_t)obj, obj))
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
	void block(); // need to be added to block queue manually
	void wakeup();
	void kill();
};
static_assert(offsetof(Thread, kernel_stack_top) == 280, "Thread layout for asm error");

extern void init_pcb();
register Thread *current_running asm("tp");
extern Thread *idle_thread;