#define _NEW
#include <Trie.hpp>
#include <assert.h>
#include <common.h>
#include <locks.hpp>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <string.h>
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

class barrier : public IdObject
{
	size_t goal;
	size_t count;
	WaitQueue wait_queue;

public:
	barrier(size_t goal) : goal(goal), count(0)
	{
	}
	void wait()
	{
		count++;
		if (count < goal)
		{
			wait_queue.push((Thread *)current_running);
			current_running->block();
			do_scheduler();
		}
		else
		{
			count = 0;
			wait_queue.wakeup_all();
		}
	}
	inline static IdPool pool;
	virtual ~barrier()
	{
		pool.remove(this);
	}
};

int64_t Syscall::sys_barrier_init(int64_t key, size_t goal)
{
	return barrier::pool.init(key, [goal]() { return new barrier(goal); });
}
void Syscall::sys_barrier_wait(size_t bar_idx)
{
	barrier *bar = (barrier *)barrier::pool.get(bar_idx);
	if (bar)
		bar->wait();
}
void Syscall::sys_barrier_destroy(size_t bar_idx)
{
	barrier::pool.close(bar_idx);
}

class condition : public IdObject
{
	WaitQueue wait_queue;

public:
	void wait(mutex_lock *lock)
	{
		wait_queue.push((Thread *)current_running);
		lock->release(current_running);
		current_running->block();
		do_scheduler();
		lock->acquire();
	}
	void signal()
	{
		wait_queue.wakeup_one();
	}
	void broadcast()
	{
		wait_queue.wakeup_all();
	}
	inline static IdPool pool;
	virtual ~condition()
	{
		pool.remove(this);
	}
};

int64_t Syscall::sys_condition_init(int64_t key)
{
	return condition::pool.init(key, []() { return new condition(); });
}
void Syscall::sys_condition_wait(size_t cond_idx, size_t mutex_idx)
{
	condition *cond = (condition *)condition::pool.get(cond_idx);
	mutex_lock *lock = (mutex_lock *)mutex_lock::pool.get(mutex_idx);
	if (cond && lock)
		cond->wait(lock);
}
void Syscall::sys_condition_signal(size_t cond_idx)
{
	condition *cond = (condition *)condition::pool.get(cond_idx);
	if (cond)
		cond->signal();
}
void Syscall::sys_condition_broadcast(size_t cond_idx)
{
	condition *cond = (condition *)condition::pool.get(cond_idx);
	if (cond)
		cond->broadcast();
}
void Syscall::sys_condition_destroy(size_t cond_idx)
{
	condition::pool.close(cond_idx);
}

class mailbox : public IdObject
{
	constexpr static size_t MAX_MSG_SIZE = 4096;
	std::queue<uint8_t> msg_queue;

	WaitQueue send_wait_queue, recv_wait_queue;

public:
	mailbox()
	{
	}
	int64_t send(const void *msg, size_t msg_length)
	{
		const uint8_t *p = (const uint8_t *)msg;
		size_t length = msg_length;
		while (length--)
		{
			if (msg_queue.size() >= MAX_MSG_SIZE)
			{
				send_wait_queue.push((Thread *)current_running);
				current_running->block();
				do_scheduler();
			}
			msg_queue.push(*p++);
			recv_wait_queue.wakeup_one();
		}
		return msg_length;
	}
	int64_t recv(void *msg, size_t msg_length)
	{
		uint8_t *p = (uint8_t *)msg;
		size_t length = msg_length;
		while (length--)
		{
			if (msg_queue.empty())
			{
				recv_wait_queue.push((Thread *)current_running);
				current_running->block();
				do_scheduler();
			}
			*p++ = msg_queue.front();
			msg_queue.pop();
			send_wait_queue.wakeup_one();
		}
		return msg_length;
	}

	inline static IdPool pool;
	virtual ~mailbox()
	{
		pool.remove(this);
	}
};
int64_t Syscall::sys_mbox_open(const char *name)
{
	// use hash to generate key
	size_t key = 0;
	for (int i = 0; name[i]; i++)
		key = key * 131 + name[i];
	return mailbox::pool.init(key, []() { return new mailbox(); });
}
void Syscall::sys_mbox_close(size_t mbox_id)
{
	mailbox::pool.close(mbox_id);
}
int64_t Syscall::sys_mbox_recv(size_t mbox_idx, void *msg, size_t msg_length)
{
	mailbox *mbox = (mailbox *)mailbox::pool.get(mbox_idx);
	if (mbox)
		return mbox->recv(msg, msg_length);
	return -1;
}
int64_t Syscall::sys_mbox_send(size_t mbox_idx, const void *msg, size_t msg_length)
{
	mailbox *mbox = (mailbox *)mailbox::pool.get(mbox_idx);
	if (mbox)
		return mbox->send(msg, msg_length);
	return -1;
}