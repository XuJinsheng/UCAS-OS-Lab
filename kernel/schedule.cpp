#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kalloc.hpp>
#include <schedule.hpp>
#include <thread.hpp>

template <typename T> class queue
{
private:
	constexpr static int MAX_SIZE = 16;
	T data[MAX_SIZE];
	int head, tail;

public:
	queue() : head(0), tail(0)
	{
	}
	~queue()
	{
	}
	void push(T val)
	{
		assert((tail + 1) % MAX_SIZE != head);
		data[tail] = val;
		tail = (tail + 1) % MAX_SIZE;
	}
	void pop()
	{
		assert(head != tail);
		head = (head + 1) % MAX_SIZE;
	}
	T front()
	{
		assert(head != tail);
		return data[head];
	}
	bool empty()
	{
		return head == tail;
	}
};

queue<Thread*> ready_queue;

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
		Thread *next_thread = ready_queue.front();
		ready_queue.pop();
		next_thread->status = Thread::TASK_RUNNING;
		current_running = next_thread;
		switch_context_entry(current_running->kernel_context.regs, next_thread->kernel_context.regs);
	}
}
void add_ready_thread(Thread *thread)
{
	ready_queue.push(thread);
}