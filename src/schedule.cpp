#define _NEW
#include <arch/CSR.h>
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <syscall.hpp>
#include <thread.hpp>
#include <time.hpp>

std::queue<Thread *> ready_queue;
std::vector<Thread *> sleeping_queue;
void check_sleeping()
{
	uint64_t current_time = get_timer();
	auto it = std::ranges::remove_if(sleeping_queue,
									 [current_time](Thread *thread) { return thread->wakeup_time <= current_time; });
	for (Thread *t : it)
	{
		t->status = Thread::TASK_READY;
		add_ready_thread(t);
	}
	sleeping_queue.erase(it.begin(), it.end());
}
void do_scheduler()
{
	// check SIE clear
	if (current_running->status == Thread::TASK_RUNNING)
	{
		current_running->status = Thread::TASK_READY;
		add_ready_thread(current_running);
	}
	check_sleeping();
	if (ready_queue.empty())
	{
		// 休眠，等待中断唤醒
		// TODO: 打开中断
		assert(0);
	}
	else
	{
		Thread *next_thread = ready_queue.front();
		ready_queue.pop();
		next_thread->status = Thread::TASK_RUNNING;
		switch_context_entry(next_thread);
	}
}
void add_ready_thread(Thread *thread)
{
	ready_queue.push(thread);
}

void Syscall::sleep(uint32_t time)
{
	current_running->wakeup_time = get_timer() + time;
	sleeping_queue.push_back((Thread *)current_running);
	current_running->status = Thread::TASK_BLOCKED;
	do_scheduler();
}

void enable_preempt()
{
	csr_set(CSR_SSTATUS, SR_SIE);
}

void disable_preempt()
{
	csr_clear(CSR_SSTATUS, SR_SIE);
}