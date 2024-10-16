#pragma once

#include <common.h>
#include <thread.hpp>

void do_scheduler(); // 会根据thread状态自动添加至ready队列
void add_ready_thread(Thread *thread);
void add_ready_thread_without_lock(Thread *thread);
void kernel_thread_first_run();
void enable_preempt();
void disable_preempt();