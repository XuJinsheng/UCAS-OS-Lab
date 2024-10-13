#pragma once

#include <common.h>

void init_syscall();
ptr_t handle_syscall(const ptr_t args[8]);

namespace Syscall
{

// screen, implemented in syscall.cpp
int sys_getchar(void);
void move_cursor(int x, int y);
void write(char *buff);
void reflush(void);

// locks.cpp
int mutex_init(int key);
void mutex_acquire(int mutex_idx);
void mutex_release(int mutex_idx);

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
int sys_kill(int pid);
int sys_waitpid(int pid);
int sys_getpid();

} // namespace Syscall