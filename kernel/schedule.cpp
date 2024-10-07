#define _NEW
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <thread.hpp>

std::queue<Thread*,std::deque<Thread*,Kallocator<Thread*>>> ready_queue;

void do_scheduler()
{
	if (current_running->status == Thread::TASK_RUNNING)
	{
		current_running->status = Thread::TASK_READY;
		add_ready_thread(current_running);
	}
	if (ready_queue.empty())
	{
		// 休眠，等待中断唤醒
		// TODO: 打开中断
		asm volatile("wfi");
	}
	else
	{
		Thread *from_thread = current_running;
		Thread *next_thread = ready_queue.front();
		ready_queue.pop();
		next_thread->status = Thread::TASK_RUNNING;
		current_running = next_thread;
		switch_context_entry(from_thread->kernel_context.regs, next_thread->kernel_context.regs);
	}
}
void add_ready_thread(Thread *thread)
{
	ready_queue.push(thread);
}