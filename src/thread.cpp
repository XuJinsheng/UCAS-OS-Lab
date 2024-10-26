#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <string.h>
#include <syscall.hpp>
#include <thread.hpp>

SpinLock thread_global_lock;
std::vector<Thread *> thread_table;

Thread::Thread(Process *process, size_t trank)
	: user_context(), process(process), tid(thread_table.size()), trank(trank)
{
	thread_global_lock.lock();
	thread_table.push_back(this);
	thread_global_lock.unlock();

	user_context.regs[16] = trank; // a7

	kernel_stack_top = (ptr_t)kalloc(2 * PAGE_SIZE) + PAGE_SIZE * 2;
	ptr_t *ksp = (ptr_t *)kernel_stack_top;
	ksp -= 14;
	ksp[0] = (ptr_t)kernel_thread_first_run;
	ksp[13] = kernel_stack_top;
	kernel_stack_top = (ptr_t)ksp;
}

void Thread::wakeup()
{
	if (--status_block_cnt == 0)
	{
		if (!status_exited)
		{
			add_ready_thread(this);
		}
	}
}

void Thread::exit()
{
	if (status_exited)
		return;
	thread_own_lock.lock();
	status_exited = true;
	wait_exited_queue.wakeup_all();
	thread_own_lock.unlock();
	if (--process->active_thread_cnt == 0)
	{
		process->kill();
	}
	if (this == current_cpu->current_thread)
	{
		do_scheduler();
	}
}

Thread *get_thread(size_t pid)
{
	lock_guard guard(thread_global_lock);
	if (pid >= thread_table.size())
		return nullptr;
	if (thread_table[pid]->has_exited())
		return nullptr;
	return thread_table[pid];
}

void Syscall::sys_exit_thread(void)
{
	current_cpu->current_thread->exit();
}

void Syscall::sys_kill_thread(size_t tid)
{
	Thread *t = get_thread(tid);
	if (t == nullptr)
		return;
	t->exit();
}
void Syscall::sys_wait_thread(size_t tid)
{
	Thread *t = get_thread(tid);
	if (t == nullptr)
		return;
	t->wait_exited_queue.push(current_cpu->current_thread);
	current_cpu->current_thread->block();
	do_scheduler();
}

void print_threads(bool killed)
{
	thread_global_lock.lock();
	printk("[Thread Table], %ld threads\n", thread_table.size());
	printk("| TID | PID | status   | name             |\n");
	for (Thread *t : thread_table)
	{
		if (!killed && t->has_exited())
			continue;
		const char *status = "";
		switch (t->status())
		{
		case Thread::Status::BLOCKED:
			status = "BLOCKED";
			break;
		case Thread::Status::READY:
			status = " READY ";
			break;
		case Thread::Status::RUNNING:
			status = "RUNNING";
			break;
		case Thread::Status::EXITED:
			status = "EXITED ";
			break;
		}
		printk("| %3d | %3d | %s | %15s |\n", t->tid, t->process->pid, status, t->process->name.c_str());
	}
	thread_global_lock.unlock();
}