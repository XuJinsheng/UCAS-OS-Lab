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
int64_t sys_barrier_init(int64_t key, size_t goal);
void sys_barrier_wait(size_t bar_idx);
void sys_barrier_destroy(size_t bar_idx);
int64_t sys_condition_init(int64_t key);
void sys_condition_wait(size_t cond_idx, size_t mutex_idx);
void sys_condition_signal(size_t cond_idx);
void sys_condition_broadcast(size_t cond_idx);
void sys_condition_destroy(size_t cond_idx);
int64_t sys_mbox_open(const char *name);
void sys_mbox_close(size_t mbox_id);
int64_t sys_mbox_recv(size_t mbox_idx, void *msg, size_t msg_length);
int64_t sys_mbox_send(size_t mbox_idx, const void *msg, size_t msg_length);

// time.cpp
long get_timebase(void);
long get_tick(void);

// schedule.cpp
void yield(void);
void sleep(uint32_t time);

void sys_ps(int process, int killed); // in syscall.cpp
// process.cpp
int sys_exec(const char *name, int argc, char **argv);
void sys_exit(void);
int sys_kill(size_t pid);
int sys_waitpid(size_t pid);
int sys_getpid();
void sys_task_set(size_t pid, long mask);
size_t sys_create_thread(ptr_t func, ptr_t arg);
// thread.cpp
void sys_exit_thread(void);
void sys_kill_thread(size_t tid);
void sys_wait_thread(size_t tid);

// page.cpp
ptr_t sys_shmpageget(int key);
void sys_shmpagedt(ptr_t va);
int sys_brk(void *addr);
void *sys_sbrk(intptr_t increment);

// net.cpp
long sys_net_send(void *txpacket, long length);
long sys_net_recv(void *rxbuffer, long pkt_num, int *pkt_lens);

// filesystem.cpp
int sys_mkfs(void);
int sys_statfs(void);
int sys_cd(const char *path);
int sys_mkdir(const char *path);
int sys_rmdir(const char *path);
int sys_ls(const char *path, int option);
int sys_touch(const char *path);
int sys_cat(const char *path);
int sys_fopen(const char *path, int mode);
int sys_fread(int fd, char *buff, int length);
int sys_fwrite(int fd, const char *buff, int length);
int sys_fclose(int fd);
int sys_ln(const char *src_path, const char *dst_path);
int sys_rm(const char *path);
int sys_lseek(int fd, int offset, int whence);
} // namespace Syscall