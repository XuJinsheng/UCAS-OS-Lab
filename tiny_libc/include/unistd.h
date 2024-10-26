#ifndef __UNISTD_H__
#define __UNISTD_H__

#include <stdint.h>
typedef int32_t pid_t;
typedef pid_t pthread_t;

void sys_sleep(uint32_t time);
void sys_yield(void);
void sys_write(char *buff);
void sys_move_cursor(int x, int y);
void sys_reflush(void);
void sys_clear();
long sys_get_timebase(void);
long sys_get_tick(void);
int sys_mutex_init(int key);
void sys_mutex_acquire(int mutex_idx);
void sys_mutex_release(int mutex_idx);

/************************************************************/
void sys_ps(int process, int killed);
int sys_getchar(void);
pid_t sys_exec(char *name, int argc, char **argv);
void sys_exit(void);
int sys_kill(pid_t pid);
int sys_waitpid(pid_t pid);
pid_t sys_getpid();

int sys_barrier_init(int key, int goal);
void sys_barrier_wait(int bar_idx);
void sys_barrier_destroy(int bar_idx);
int sys_condition_init(int key);
void sys_condition_wait(int cond_idx, int mutex_idx);
void sys_condition_signal(int cond_idx);
void sys_condition_broadcast(int cond_idx);
void sys_condition_destroy(int cond_idx);
int sys_mbox_open(char *name);
void sys_mbox_close(int mbox_id);
int sys_mbox_send(int mbox_idx, void *msg, int msg_length);
int sys_mbox_recv(int mbox_idx, void *msg, int msg_length);

/************************************************************/
void sys_task_set(pid_t pid, long mask);

/************************************************************/
void *sys_shmpageget(int key);
void sys_shmpagedt(void *addr);

#endif
