#include <pthread.h>
#include <unistd.h>

/* TODO:[P4-task4] pthread_create/wait */
void pthread_create(pthread_t *thread, void (*start_routine)(void *), void *arg)
{
	*thread = sys_create_thread(start_routine, arg);
}

int pthread_join(pthread_t thread)
{
	sys_wait_thread(thread);
}
