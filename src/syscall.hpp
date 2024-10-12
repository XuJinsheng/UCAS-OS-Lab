#pragma once

#include <common.h>

void init_syscall();
ptr_t handle_syscall(const ptr_t args[8]);

namespace Syscall
{

void yield(void);

void move_cursor(int x, int y);

void write(char *buff);

void reflush(void);

int mutex_init(int key);

void mutex_acquire(int mutex_idx);

void mutex_release(int mutex_idx);

long get_timebase(void);

long get_tick(void);

void sleep(uint32_t time);

} // namespace Syscall