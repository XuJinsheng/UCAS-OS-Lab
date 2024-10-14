#pragma once

#include <common.h>

void init_syscall();
ptr_t handle_syscall(const ptr_t args[8]);

namespace Syscall
{

// screen, implemented in syscall.cpp
int sys_getchar(void);
void move_cursor(int x, int y);
void write(const char *buff);
void reflush(void);
void sys_clear();

// locks.cpp
int mutex_init(size_t key);
void mutex_acquire(size_t mutex_idx);
void mutex_release(size_t mutex_idx);

// time.cpp
long get_timebase(void);
long get_tick(void);

// schedule.cpp
void yield(void);
void sleep(uint32_t time);

// thread.cpp
void sys_ps(void);
int sys_exec(const char *name, int argc, char **argv);
void sys_exit(void);
int sys_kill(size_t pid);
int sys_waitpid(size_t pid);
int sys_getpid();

} // namespace Syscall