#define _NEW
#include <Trie.hpp>
#include <assert.h>
#include <common.h>
#include <locks.hpp>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>

void init_locks()
{
}
class IdPool;
class IdObject : public KernelObject
{
	size_t handle, trie_idx;
	friend IdPool;
};
class IdPool
{
	TrieLookup<IdObject *> keymap;
	std::vector<IdObject *> table;

public:
	int64_t init(size_t key, auto create_fn)
	{
		IdObject *p = keymap.lookup(key);
		if (p == nullptr)
		{
			p = create_fn();
			p->handle = table.size();
			table.push_back(p);
			p->trie_idx = keymap.insert(key, p);
		}
		current_running->register_kernel_object(p);
		return p->handle;
	}
	IdObject *get(size_t handle)
	{
		if (handle >= table.size())
			return nullptr;
		return table[handle]; // may be nullptr
	}
	void close(size_t handle)
	{
		IdObject *obj = get(handle);
		if (obj == nullptr)
			return;
		if (obj->ref_count == 1 && current_running->unregister_kernel_object(obj))
			table[handle] = nullptr;
	}
	void remove(IdObject *obj)
	{
		if (obj->handle >= table.size())
			return;
		table[obj->handle] = nullptr;
		keymap.remove_by_idx(obj->trie_idx);
	}
};

class mutex_lock : public IdObject
{
public:
	// SpinLock lock;
	Thread *owner;
	WaitQueue wait_queue;
	mutex_lock() : owner(nullptr)
	{
	}
	void acquire()
	{
		if (owner == current_running)
			return;
		if (owner != nullptr)
		{
			wait_queue.push((Thread *)current_running);
			current_running->block();
			do_scheduler();
		}
		owner = current_running;
	}
	void release(Thread *t)
	{
		if (owner != t)
			return;
		owner = wait_queue.wakeup_one();
	}
	virtual void on_thread_unregister(Thread *t) override
	{
		release(t);
		KernelObject::on_thread_unregister(t);
	}
	inline static IdPool pool;
	virtual ~mutex_lock()
	{
		pool.remove(this);
	}
};

int Syscall::mutex_init(size_t key)
{
	return mutex_lock::pool.init(key, []() { return new mutex_lock(); });
}

void Syscall::mutex_acquire(size_t mutex_idx)
{
	mutex_lock *lock = (mutex_lock *)mutex_lock::pool.get(mutex_idx);
	if (lock)
		lock->acquire();
}

void Syscall::mutex_release(size_t mutex_idx)
{
	mutex_lock *lock = (mutex_lock *)mutex_lock::pool.get(mutex_idx);
	if (lock)
		lock->release(current_running);
}
