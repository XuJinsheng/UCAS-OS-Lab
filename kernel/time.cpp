#include <arch/bios_func.h>
#include <common.h>
#include <schedule.hpp>
#include <syscall.hpp>
#include <time.hpp>

uint64_t time_elapsed = 0;
uint64_t time_base = 0;

void init_timer()
{
	time_base = bios_read_fdt(TIMEBASE);
}
uint64_t get_ticks()
{
	__asm__ __volatile__("rdtime %0" : "=r"(time_elapsed));
	return time_elapsed;
}

uint64_t get_timer()
{
	return get_ticks() / time_base;
}

uint64_t get_time_base()
{
	return time_base;
}

void latency(uint64_t time)
{
	uint64_t begin_time = get_timer();

	while (get_timer() - begin_time < time)
		;
	return;
}

long Syscall::get_timebase(void)
{
	return time_base;
}

long Syscall::get_tick(void)
{
	return get_ticks();
}

void handle_irq_timer()
{
	bios_set_timer(get_ticks() + TIMER_INTERVAL);
	do_scheduler();
}