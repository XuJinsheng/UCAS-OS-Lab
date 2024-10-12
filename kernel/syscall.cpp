#include <assert.h>
#include <common.h>
#include <drivers/screen.h>
#include <schedule.hpp>
#include <syscall.hpp>
#include <thread.hpp>

enum class SyscallID
{
	SLEEP = 2,
	YIELD = 7,
	WRITE = 20,
	MOVE_CURSOR = 22,
	REFLUSH = 23,
	GET_TIMEBASE = 30,
	GET_TICK = 31,
	MUTEX_INIT = 40,
	MUTEX_ACQ = 41,
	MUTEX_RELEASE = 42
};
void init_syscall()
{
}

ptr_t handle_syscall(const ptr_t args[8])
{
	ptr_t ret = 0;
	switch ((SyscallID)args[7])
	{
	case SyscallID::SLEEP:
		Syscall::sleep(args[0]);
		break;
	case SyscallID::YIELD:
		Syscall::yield();
		break;
	case SyscallID::WRITE:
		Syscall::write((char *)args[0]);
		break;
	case SyscallID::MOVE_CURSOR:
		Syscall::move_cursor(args[0], args[1]);
		break;
	case SyscallID::REFLUSH:
		Syscall::reflush();
		break;
	case SyscallID::GET_TIMEBASE:
		ret = Syscall::get_timebase();
		break;
	case SyscallID::GET_TICK:
		ret = Syscall::get_tick();
		break;
	case SyscallID::MUTEX_INIT:
		ret = Syscall::mutex_init(args[0]);
		break;
	case SyscallID::MUTEX_ACQ:
		Syscall::mutex_acquire(args[0]);
		break;
	case SyscallID::MUTEX_RELEASE:
		Syscall::mutex_release(args[0]);
		break;
	default:
		assert(0);
	}
	return ret;
}

void Syscall::yield(void)
{
	do_scheduler();
}

void Syscall::move_cursor(int x, int y)
{
	screen_move_cursor(x, y);
}

void Syscall::write(char *buff)
{
	screen_write(buff);
}

void Syscall::reflush(void)
{
	screen_reflush();
}
