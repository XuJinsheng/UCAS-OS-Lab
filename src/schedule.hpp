#pragma once

#include <common.h>

class Thread;
class WaitQueue;
class SpinLock;
void do_scheduler(Thread *next_thread = nullptr); // 会根据thread状态自动添加至ready队列
void do_block(WaitQueue &wait_queue);
void do_block(WaitQueue &wait_queue, SpinLock &lock);
void add_ready_thread(Thread *thread);
void add_ready_thread_without_lock(Thread *thread);
void kernel_thread_first_run();
void enable_preempt();
void disable_preempt();
void assert_no_preempt();