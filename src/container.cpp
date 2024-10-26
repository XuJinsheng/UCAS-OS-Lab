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