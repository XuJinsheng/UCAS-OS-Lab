#pragma once

#include <common.h>

#define SYSCALL_SLEEP 2
#define SYSCALL_YIELD 7
#define SYSCALL_WRITE 20
#define SYSCALL_MOVE_CURSOR 22
#define SYSCALL_REFLUSH 23
#define SYSCALL_GET_TIMEBASE 30
#define SYSCALL_GET_TICK 31
#define SYSCALL_MUTEX_INIT 40
#define SYSCALL_MUTEX_ACQ 41
#define SYSCALL_MUTEX_RELEASE 42
#define SYSCALL_SET_SCHE_WORKLOAD 43

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

void set_sche_workload(int workload);

}