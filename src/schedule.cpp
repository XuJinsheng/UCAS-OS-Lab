#define _NEW
#include <arch/CSR.h>
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>
#include <time.hpp>

std::list<Thread *> ready_queue;
using SleepItem = std::pair<uint64_t, Thread *>;
std::priority_queue<SleepItem, std::vector<SleepItem>, std::greater<SleepItem>> sleeping_queue;

SpinLock ready_lock, sleep_lock;
void check_sleeping()
{
	sleep_lock.lock();
	uint64_t current_time = get_timer();
	while (!sleeping_queue.empty() && sleeping_queue.top().first < current_time)
	{
		Thread *t = sleeping_queue.top().second;
		sleeping_queue.pop();
		t->wakeup();
	}
	sleep_lock.unlock();
}
void do_scheduler(Thread *next_thread)
{
	Thread *from_thread = current_cpu->current_thread;
	from_thread->status_running = false; // must be clear before select ready thread

	if (next_thread == nullptr)
	{
		// check SIE clear
		check_sleeping();
		ready_lock.lock();
		if (current_cpu->current_thread->status() == Thread::Status::READY)
		{
			if (current_cpu->current_thread != current_cpu->idle_thread)
				add_ready_thread_without_lock(current_cpu->current_thread);
		}

		for (auto it = ready_queue.begin(); it != ready_queue.end(); ++it)
		{
			if ((*it)->status() != Thread::Status::READY)
			{
				it = ready_queue.erase(it);
				continue;
			}
			if (!((*it)->process->cpu_mask & (1 << current_cpu->cpu_id)))
				continue;
			next_thread = *it;
			ready_queue.erase(it);
			break;
		}
		if (next_thread == nullptr)
		{
			next_thread = current_cpu->idle_thread;
		}
		ready_lock.unlock();
	}

	// note: from_thread may equal to next_thread
	current_cpu->current_thread = next_thread;
	from_thread->running_cpu = nullptr;
	next_thread->running_cpu = current_cpu;
	from_thread->status_running = false;
	next_thread->status_running = true;
	if (from_thread->process != next_thread->process)
		next_thread->process->pagedir.enable(current_cpu->cpu_id, next_thread->process->pid);
	else
		next_thread->process->pagedir.flushcpu(current_cpu->cpu_id, next_thread->process->pid);
	switch_context_entry(from_thread, next_thread);
}
void kernel_thread_first_run()
{
	user_trap_return();
}

void add_ready_thread(Thread *thread)
{
	ready_lock.lock();
	add_ready_thread_without_lock(thread);
	ready_lock.unlock();
}
void add_ready_thread_without_lock(Thread *thread)
{
	ready_queue.push_back(thread);
}
void do_block(WaitQueue &wait_queue)
{
	wait_queue.push(current_cpu->current_thread);
	current_cpu->current_thread->block();
	do_scheduler();
}
void do_block(WaitQueue &wait_queue, SpinLock &lock)
{
	wait_queue.push(current_cpu->current_thread);
	current_cpu->current_thread->block();
	lock.unlock();
	do_scheduler();
}

void Syscall::yield(void)
{
	do_scheduler();
}

void Syscall::sleep(uint32_t time)
{
	size_t wakeup_time = get_timer() + time;
	sleep_lock.lock();
	sleeping_queue.push({wakeup_time, current_cpu->current_thread});
	sleep_lock.unlock();
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

void assert_no_preempt()
{
	ptr_t sstatus = csr_read(CSR_SSTATUS);
	assert(!(sstatus & SR_SIE));
}