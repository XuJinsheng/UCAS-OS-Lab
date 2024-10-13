#define _NEW
#include <Trie.hpp>
#include <common.h>
#include <kalloc.hpp>
#include <locks.hpp>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>

void init_locks()
{
}

class mutex_lock : public KernelObject
{
public:
	int id;
	// SpinLock lock;
	Thread *owner;
	WaitQueue wait_queue;

	void release(Thread *t)
	{
		if (owner != t)
			return;
		owner = wait_queue.wakeup_one();
	}
	virtual void on_thread_kill(Thread *t) override
	{
		release(t);
		KernelObject::on_thread_kill(t);
	}
	virtual ~mutex_lock() = default;
};

TrieLookup<mutex_lock> mutex_keys;
std::vector<mutex_lock *> mutex_array;

int Syscall::mutex_init(int key)
{
	mutex_lock *p = mutex_keys.lookup(key);
	if (p == nullptr)
	{
		p = new mutex_lock;
		p->id = mutex_array.size();
		mutex_array.push_back(p);
	}
	current_running->add_kernel_object(p);
	return p->id;
}

void Syscall::mutex_acquire(int mutex_idx)
{
	if (mutex_idx >= mutex_array.size() || mutex_array[mutex_idx] == nullptr)
		return;
	mutex_lock &lock = *mutex_array[mutex_idx];
	if (lock.owner == current_running)
		return;
	if (lock.owner != nullptr)
	{
		lock.wait_queue.push((Thread *)current_running);
		current_running->block();
		do_scheduler();
	}
	lock.owner = current_running;
}

void Syscall::mutex_release(int mutex_idx)
{
	if (mutex_idx >= mutex_array.size() || mutex_array[mutex_idx] == nullptr)
		return;
	mutex_array[mutex_idx]->release(current_running);
}