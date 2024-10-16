#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <string.h>
#include <syscall.hpp>
#include <task_loader.hpp>
#include <thread.hpp>

Thread *WaitQueue::wakeup_one()
{
	if (que.empty())
		return nullptr;
	Thread *t = que.front();
	que.pop();
	t->wakeup();
	return t;
}
void WaitQueue::wakeup_all()
{
	while (!que.empty())
	{
		Thread *t = que.front();
		que.pop();
		t->wakeup();
	}
}

void init_processor(size_t hartid)
{
	static std::atomic<size_t> cpu_id_cnt;
	current_cpu = new CPU();
	current_cpu->cpu_id = cpu_id_cnt++;
	current_cpu->hartid = hartid;
	current_cpu->idle_thread = current_cpu->current_thread = new Thread(nullptr, "kernel-idle");
	current_cpu->idle_thread->kernel_stack_top = 0;
}

SpinLock thread_global_lock;
std::vector<Thread *> thread_table;

Thread::Thread(Thread *parent, std::string name)
	: user_context(), cursor_x(0), cursor_y(0), pid(thread_table.size()), status(Status::READY), parent(parent),
	  name(name)
{
	thread_global_lock.lock();
	thread_table.push_back(this);
	if (parent != nullptr)
		parent->children.push_back(this);

	user_context.regs[1] = (ptr_t)alloc_user_page(2) + PAGE_SIZE * 2;
	kernel_stack_top = (ptr_t)allocKernelPage(2) + PAGE_SIZE * 2;
	ptr_t *ksp = (ptr_t *)kernel_stack_top;
	ksp -= 14;
	ksp[0] = (ptr_t)kernel_thread_first_run;
	ksp[13] = kernel_stack_top;
	kernel_stack_top = (ptr_t)ksp;
	thread_global_lock.unlock();
}

void *Thread::alloc_user_page(size_t numPage)
{
	void *ret = allocUserPage(numPage);
	thread_own_lock.lock();
	user_memory.push(ret);
	thread_own_lock.unlock();
	return ret;
}
void Thread::block()
{
	assert(status == Status::RUNNING);
	status = Status::BLOCKED;
}

void Thread::wakeup()
{
	if (status == Status::EXITED)
	{
		return;
	}
	assert(status != Status::RUNNING);
	status = Status::READY;
	add_ready_thread(this);
}

void Thread::kill()
{
	lock_guard guard(thread_own_lock);
	status = Status::EXITED;
	wait_kill_queue.wakeup_all();
	kernel_objects.foreach ([this](KernelObject *obj) { obj->on_thread_unregister(this); });
	if (parent != nullptr)
	{
		for (Thread *child : children)
			if (child->status != Status::EXITED)
			{
				child->parent = parent;
				parent->children.push_back(child);
			}
			else
			{
				// TODO: where to place dead child? delete maybe cause pointer invalid
				child->parent = thread_table[0];
				thread_table[0]->children.push_back(child);
			}
	}
	while (!user_memory.empty())
	{
		void *mem = user_memory.front();
		user_memory.pop();
		freeUserPage(mem);
	}
}

Thread *get_thread(size_t pid)
{
	lock_guard guard(thread_global_lock);
	if (pid >= thread_table.size())
		return nullptr;
	if (thread_table[pid]->status == Thread::Status::EXITED)
		return nullptr;
	return thread_table[pid];
}
void Syscall::sys_exit(void)
{
	current_cpu->current_thread->kill();
	do_scheduler();
}
int Syscall::sys_kill(size_t pid)
{
	Thread *t = get_thread(pid);
	if (t == nullptr)
		return 0;
	t->kill();
	if (t == current_cpu->current_thread)
		do_scheduler();
	return 1;
}
int Syscall::sys_waitpid(size_t pid)
{
	Thread *t = get_thread(pid);
	if (t == nullptr)
		return 0;
	t->thread_own_lock.lock();
	t->wait_kill_queue.push(current_cpu->current_thread);
	t->thread_own_lock.unlock();
	current_cpu->current_thread->block();
	do_scheduler();
	return pid;
}
int Syscall::sys_getpid()
{
	return current_cpu->current_thread->pid;
}
void Syscall::sys_ps(void)
{
	thread_global_lock.lock();
	printk("[Process Table], %d total\n", thread_table.size());
	int index = 0;
	for (Thread *t : thread_table)
	{
		const char *status = "";
		switch (t->status)
		{
		case Thread::Status::BLOCKED:
			status = "BLOCKED";
			break;
		case Thread::Status::READY:
			status = "READY";
			break;
		case Thread::Status::RUNNING:
			status = "RUNNING";
			break;
		case Thread::Status::EXITED:
			status = "EXITED";
			break;
		}
		printk("[%d]:pid=%d,\t status=%s,\t name=%s\n", ++index, t->pid, status, t->name.c_str());
	}
	thread_global_lock.unlock();
}
int Syscall::sys_exec(const char *name, int argc, char **argv)
{
	ptr_t entry = load_task_img_by_name(name);
	if (entry == 0)
		return 0;

	Thread *t = new Thread(current_cpu->current_thread, name);

	char **argv_copy = (char **)t->alloc_user_page(1);
	char *argv_data = (char *)(argv_copy + argc);
	for (int i = 0; i < argc; i++)
	{
		size_t len = strlen(argv[i]) + 1;
		memcpy(argv_data, argv[i], len);
		argv_copy[i] = argv_data;
		argv_data += len;
	}

	t->user_context.sepc = entry;
	t->user_context.regs[9] = argc;
	t->user_context.regs[10] = (ptr_t)argv_copy;
	add_ready_thread(t);

	return t->pid;
}