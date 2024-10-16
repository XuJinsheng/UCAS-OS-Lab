#define _NEW
#include <arch/CSR.h>
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <schedule.hpp>
#include <syscall.hpp>
#include <thread.hpp>
#include <time.hpp>

std::queue<Thread *> ready_queue;
using SleepItem = std::pair<uint64_t, Thread *>;
std::priority_queue<SleepItem, std::vector<SleepItem>, std::greater<SleepItem>> sleeping_queue;
void check_sleeping()
{
	uint64_t current_time = get_timer();
	while (!sleeping_queue.empty() && sleeping_queue.top().first < current_time)
	{
		Thread *t = sleeping_queue.top().second;
		sleeping_queue.pop();
		t->wakeup();
	}
}
void do_scheduler()
{
	// check SIE clear
	if (current_cpu->current_thread->status == Thread::Status::RUNNING)
	{
		current_cpu->current_thread->status = Thread::Status::READY;
		if (current_cpu->current_thread != current_cpu->idle_thread)
			add_ready_thread(current_cpu->current_thread);
	}
	check_sleeping();
	Thread *next_thread;
	do
	{
		if (ready_queue.empty())
		{
			next_thread = current_cpu->idle_thread;
		}
		else
		{
			next_thread = ready_queue.front();
			ready_queue.pop();
		}
	} while (next_thread->status != Thread::Status::READY);
	Thread *from_thread = current_cpu->current_thread;
	current_cpu->current_thread = next_thread;
	next_thread->status = Thread::Status::RUNNING;
	switch_context_entry(from_thread, next_thread);
}
void add_ready_thread(Thread *thread)
{
	ready_queue.push(thread);
}

void Syscall::yield(void)
{
	do_scheduler();
}

void Syscall::sleep(uint32_t time)
{
	size_t wakeup_time = get_timer() + time;
	sleeping_queue.push({wakeup_time, current_cpu->current_thread});
	current_cpu->current_thread->block();
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