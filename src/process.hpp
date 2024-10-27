#pragma once

#include <CPU.hpp>
#include <common.h>
#include <container.hpp>
#include <page.hpp>
#include <spinlock.hpp>

class Process
{
public:
	SpinLock process_own_lock;
	Process(Process *parent, std::string name);
	~Process();
	Process(const Process &) = delete;
	Process &operator=(const Process &) = delete;

public:
	const int pid;
	const std::string name;
	PageDir pageroot;

	Process *parent;
	std::vector<Process *> children;
	std::vector<Thread *> threads; // add by thread

public:
	size_t cpu_mask = -1;
	int cursor_x = 0, cursor_y = 0;
	WaitQueue wait_kill_queue;
	std::atomic<size_t> active_thread_cnt; // inc and dec by thread
	bool is_killed = false;

private:
	TrieLookup<KernelObject *> kernel_objects;
	static size_t obj2key(KernelObject *obj)
	{
		return (size_t)obj & 0xffffffff;
	}

public:
	bool register_kernel_object(KernelObject *obj) // true: insert success, false: already exists
	{
		if (kernel_objects.insert(obj2key(obj), obj))
		{
			obj->ref_count++;
			return true;
		}
		return false;
	}
	bool unregister_kernel_object(KernelObject *obj) // true: remove success, false: not found
	{
		if (kernel_objects.remove(obj2key(obj)))
			return false;
		obj->on_process_unregister(this);
		return true;
	}

public:
	void kill(); // if thread killed to 0, thread call to kill process
	Thread *create_thread();
	void send_intr_to_running_thread();
};
extern Process *idle_process;
extern void init_processor(size_t hartid);
extern void print_processes(bool killed);
extern void idle_cleanup();