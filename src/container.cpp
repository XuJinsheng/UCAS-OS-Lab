#include <common.h>
#include <container.hpp>
#include <thread.hpp>

Thread *WaitQueue::wakeup_one()
{
	if (que.empty())
		return nullptr;
	Thread *t = que.front();
	que.pop();
	t->wakeup();
	return t;
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

int64_t IdPool::init(size_t key, std::function<IdObject *()> create_fn)
{
	lock_guard guard(lock);
	IdObject *p = keymap.lookup(key);
	if (p == nullptr)
	{
		p = create_fn();
		p->handle = table.size();
		table.push_back(p);
		p->trie_idx = keymap.insert(key, p);
	}
	current_process->register_kernel_object(p);
	return p->handle;
}
IdObject *IdPool::get(size_t handle)
{
	lock_guard guard(lock);
	if (handle >= table.size())
		return nullptr;
	return table[handle]; // may be nullptr
}
void IdPool::close(size_t handle)
{
	IdObject *obj = get(handle); // lock guaranteed
	if (obj == nullptr)
		return;
	current_process->unregister_kernel_object(obj);
}
void IdPool::remove(IdObject *obj)
{
	lock_guard guard(lock);
	if (obj->handle >= table.size())
		return;
	table[obj->handle] = nullptr;
	keymap.remove_by_idx(obj->trie_idx);
}