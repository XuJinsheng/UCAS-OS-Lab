#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <thread.hpp>

Thread *WaitQueue::wakeup_one()
{
	if (!que.empty())
	{
		Thread *t = que.front();
		que.pop();
		t->wakeup();
		return t;
	}
	else
		return nullptr;
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

void init_pcb()
{
	current_running = new Thread((ptr_t) nullptr);
	current_running->status = Thread::Status::BLOCKED;
}

static int next_pid = 0;

Thread::Thread(ptr_t start_address) : user_context(), cursor_x(0), cursor_y(0), status(Status::READY)
{
	pid = next_pid++;
	user_context.sepc = start_address;
	user_context.regs[1] = (ptr_t)alloc_user_memory(PAGE_SIZE) + PAGE_SIZE;
	kernel_stack_top = (ptr_t)allocKernelPage(1) + PAGE_SIZE;
	ptr_t *ksp = (ptr_t *)kernel_stack_top;
	ksp -= 14;
	ksp[0] = (ptr_t)user_trap_return;
	ksp[13] = kernel_stack_top;
	kernel_stack_top = (ptr_t)ksp;
}

void *Thread::alloc_user_memory(size_t size)
{
	void *ret = allocUserPage(size / PAGE_SIZE + size % PAGE_SIZE != 0);
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
	assert(status == Status::BLOCKED);
	status = Status::READY;
	add_ready_thread(this);
}

void Thread::kill()
{
	status = Status::EXITED;
	while (!kernel_objects.empty())
	{
		KernelObject *obj = kernel_objects.front();
		kernel_objects.pop();
		obj->on_thread_kill(this);
	}
	while (!user_memory.empty())
	{
		void *mem = user_memory.front();
		user_memory.pop();
		freeUserPage(mem);
	}
}