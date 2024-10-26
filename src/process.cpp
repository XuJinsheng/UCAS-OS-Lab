#include <CPU.hpp>
#include <arch/bios_func.h>
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <process.hpp>
#include <schedule.hpp>
#include <string.h>
#include <syscall.hpp>
#include <task_loader.hpp>
#include <thread.hpp>

static std::atomic<size_t> cpu_id_cnt;
Process *idle_process;
void init_processor(size_t hartid)
{
	if (hartid == 0)
	{
		idle_process = new Process(nullptr, "kernel-idle");
	}
	current_cpu = new CPU();
	current_cpu->cpu_id = cpu_id_cnt++;
	current_cpu->hartid = hartid;
	current_cpu->idle_thread = current_cpu->current_thread = idle_process->create_thread();
	current_cpu->idle_thread->kernel_stack_top = 0; // catch wrong stack for idle thread

	// do not add idle_thread to schedule queue
}

SpinLock process_global_lock;
std::vector<Process *> process_table;
std::queue<Process *> process_to_release;

Process::Process(Process *parent, std::string name) : pid(process_table.size()), name(name), parent(parent)
{
	if (parent != nullptr)
	{
		parent->process_own_lock.lock();
		parent->children.push_back(this);
		cpu_mask = parent->cpu_mask;
		parent->process_own_lock.unlock();
	}
	process_global_lock.lock();
	process_table.push_back(this);
	process_global_lock.unlock();
}

Process::~Process()
{
	process_global_lock.lock();
	process_table[pid] = nullptr;
	process_global_lock.unlock();
}

void Process::kill()
{
	if (is_killed)
		return;
	process_own_lock.lock();
	is_killed = true;
	wait_kill_queue.wakeup_all();
	for (Thread *t : threads)
		t->exit();
	process_own_lock.unlock();

	send_intr_to_running_thread();
	kernel_objects.foreach ([this](KernelObject *obj) { obj->on_process_unregister(this); });
	if (parent != nullptr)
	{
		for (Process *child : children)
			if (!child->is_killed)
			{
				child->parent = parent;
				parent->children.push_back(child);
			}
	}
	process_global_lock.lock();
	process_to_release.push(this);
	process_global_lock.unlock();
}

Thread *Process::create_thread()
{
	process_own_lock.lock();
	Thread *t = new Thread(this, threads.size());
	active_thread_cnt++;
	threads.push_back(t);
	process_own_lock.unlock();
	return t;
}

void Process::send_intr_to_running_thread()
{
	size_t hartmask = 0;
	for (Thread *t : threads)
	{
		if (t->status() == Thread::Status::RUNNING)
		{
			hartmask |= 1 << t->running_cpu->hartid;
		}
	}
	if (hartmask)
	{
		bios_send_ipi(&hartmask);
	}
}

Process *get_process(size_t pid)
{
	lock_guard guard(process_global_lock);
	if (pid >= process_table.size())
		return nullptr;
	if (process_table[pid]->is_killed)
		return nullptr;
	return process_table[pid];
}
void Syscall::sys_exit(void)
{
	current_cpu->current_thread->process->kill();
	do_scheduler();
}
int Syscall::sys_kill(size_t pid)
{
	Process *p = get_process(pid);
	if (p == nullptr)
		return 0;
	p->kill();
	if (p == current_cpu->current_thread->process)
		do_scheduler();
	return 1;
}
int Syscall::sys_getpid()
{
	return current_cpu->current_thread->process->pid;
}
int Syscall::sys_waitpid(size_t pid)
{
	Process *p = get_process(pid);
	if (p == nullptr)
		return 0;
	p->process_own_lock.lock();
	p->wait_kill_queue.push(current_cpu->current_thread);
	p->process_own_lock.unlock();
	current_cpu->current_thread->block();
	do_scheduler();
	return pid;
}
void Syscall::sys_task_set(size_t pid, long mask)
{
	Process *p = pid == 0 ? current_cpu->current_thread->process : get_process(pid);
	if (p == nullptr)
		return;
	p->cpu_mask = mask;
}

int Syscall::sys_exec(const char *name, int argc, char **argv)
{
	int task_idx = find_task_idx_by_name(name);
	if (task_idx == -1)
		return 0;
	Process *p = new Process(current_cpu->current_thread->process, name);
	Thread *t = p->create_thread();
	load_task_img(task_idx, p->pageroot);

	constexpr ptr_t argv_va = USER_ENTRYPOINT - PAGE_SIZE;
	t->user_context.sepc = USER_ENTRYPOINT;
	t->user_context.regs[9] = argc;
	t->user_context.regs[10] = argv_va;
	char **argv_copy = (char **)p->pageroot.alloc_page_for_va(argv_va);
	char *argv_data = (char *)(argv_copy + argc);
	char *argc_data_va = (char *)(argv_va + argc * sizeof(char *));
	for (int i = 0; i < argc; i++)
	{
		size_t len = strlen(argv[i]) + 1;
		memcpy(argv_data, argv[i], len);
		argv_copy[i] = argc_data_va;
		argv_data += len;
		argc_data_va += len;
	}

	add_ready_thread(t);

	return p->pid;
}

size_t Syscall::sys_create_thread(ptr_t func, ptr_t arg)
{
	Thread *t = current_cpu->current_thread->process->create_thread();
	t->user_context.sepc = USER_ENTRYPOINT;
	t->user_context.regs[9] = func;
	t->user_context.regs[10] = arg;
	add_ready_thread(t);
	return t->tid;
}

void print_processes(bool killed)
{
	process_global_lock.lock();
	printk("[Process Table], %ld CPUs, %ld processes\n", cpu_id_cnt.load(), process_table.size());
	printk("| PID | threads | mask  | name             |\n");
	for (Process *p : process_table)
	{
		if (!killed && p->is_killed)
			continue;
		printk("| %3d | %7d |  %4x  | %15s |\n", p->pid, p->active_thread_cnt.load(), p->cpu_mask & 0xffff,
			   p->name.c_str());
	}
	process_global_lock.unlock();
}