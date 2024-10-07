#define _NEW
#include <common.h>
#include <kalloc.hpp>
#include <locks.hpp>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>

class mutex_lock
{
public:
	int key;
	// SpinLock lock;
	Thread *owner;
	std::queue<Thread *> wait_queue;
};
static mutex_lock locks[16];
static int lock_cnt = 0;

void init_locks()
{
}

int Syscall::mutex_init(int key)
{
	for (int i = 0; i < lock_cnt; i++)
	{
		if (locks[i].key == key)
			return i;
	}
	locks[lock_cnt].key = key;
	return lock_cnt++;
}

void Syscall::mutex_acquire(int mutex_idx)
{
	if (mutex_idx >= lock_cnt)
		return;
	mutex_lock &lock = locks[mutex_idx];
	if (lock.owner == current_running)
		return;
	if (lock.owner != nullptr)
	{
		lock.wait_queue.push(current_running);
		current_running->status = Thread::TASK_BLOCKED;
		do_scheduler();
	}
	lock.owner = current_running;
}

void Syscall::mutex_release(int mutex_idx)
{
	if (mutex_idx >= lock_cnt)
		return;
	mutex_lock &lock = locks[mutex_idx];
	if (lock.owner != current_running)
		return;
	if (lock.wait_queue.empty())
	{
		lock.owner = nullptr;
		return;
	}
	lock.owner = lock.wait_queue.front();
	lock.wait_queue.pop();
	lock.owner->status = Thread::TASK_READY;
	add_ready_thread(lock.owner);
}