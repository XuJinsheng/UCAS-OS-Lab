#pragma once

#include <common.h>
#include <thread.hpp>

void do_scheduler(); // 会根据thread状态自动添加至ready队列
void add_ready_thread(Thread *thread);
