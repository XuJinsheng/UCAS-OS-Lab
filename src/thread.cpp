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

Thread *idle_thread;
void init_pcb()
{
	idle_thread = current_running = new Thread(nullptr);
	idle_thread->kernel_stack_top = 0;
}

std::vector<Thread *> thread_table;

Thread::Thread(Thread *parent)
	: user_context(), cursor_x(0), cursor_y(0), pid(thread_table.size()), status(Status::READY), parent(parent)
{
	thread_table.push_back(this);
	if (parent != nullptr)
		parent->children.push_back(this);

	user_context.regs[1] = (ptr_t)alloc_user_page(2) + PAGE_SIZE * 2;
	kernel_stack_top = (ptr_t)allocKernelPage(2) + PAGE_SIZE * 2;
	ptr_t *ksp = (ptr_t *)kernel_stack_top;
	ksp -= 14;
	ksp[0] = (ptr_t)user_trap_return;
	ksp[13] = kernel_stack_top;
	kernel_stack_top = (ptr_t)ksp;
}

void *Thread::alloc_user_page(size_t numPage)
{
	void *ret = allocUserPage(numPage);
	user_memory.push(ret);
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
		return;
	assert(status != Status::RUNNING);
	status = Status::READY;
	add_ready_thread(this);
}

void Thread::kill()
{
	status = Status::EXITED;
	wait_kill_queue.wakeup_all();
	kernel_objects.foreach ([this](KernelObject *obj) { obj->on_thread_kill(this); });
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
	if (pid >= thread_table.size())
		return nullptr;
	if (thread_table[pid]->status == Thread::Status::EXITED)
		return nullptr;
	return thread_table[pid];
}
void Syscall::sys_exit(void)
{
	current_running->kill();
	do_scheduler();
}
int Syscall::sys_kill(size_t pid)
{
	Thread *t = get_thread(pid);
	if (t == nullptr)
		return 0;
	t->kill();
	if (t == current_running)
		do_scheduler();
	return 1;
}
int Syscall::sys_waitpid(size_t pid)
{
	Thread *t = get_thread(pid);
	if (t == nullptr)
		return 0;
	t->wait_kill_queue.push(current_running);
	current_running->block();
	do_scheduler();
	return pid;
}
int Syscall::sys_getpid()
{
	return current_running->pid;
}
void Syscall::sys_ps(void)
{
	printk("[Process Table], %d total\n", thread_table.size());
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
		printk("pid=%d,\t status=%s\n", t->pid, status);
	}
}
int Syscall::sys_exec(const char *name, int argc, char **argv)
{
	ptr_t entry = load_task_img_by_name(name);
	if (entry == 0)
		return 0;

	Thread *t = new Thread(current_running);

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