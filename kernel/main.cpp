#include <common.h>
#include <drivers/screen.h>
#include <kalloc.hpp>
#include <kstdio.h>
#include <locks.hpp>
#include <schedule.hpp>
#include <syscall.hpp>
#include <task_loader.hpp>
#include <thread.hpp>
#include <trap.hpp>

extern func_t __preinit_array_start[0], __preinit_array_end[0], __init_array_start[0], __init_array_end[0];
int main(void)
{
	// Init memory heap
	init_kernel_heap();

	// Init C++ static variables
	for (func_t *func = __preinit_array_start; func < __preinit_array_end; func++)
		(*func)();
	for (func_t *func = __init_array_start; func < __init_array_end; func++)
		(*func)();

	// Init task information (〃'▽'〃)
	init_task_info();

	// Init Thread Control Blocks |•'-'•) ✧
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// Read CPU frequency (｡•ᴗ-)_
	// time_base = bios_read_fdt(TIMEBASE);

	// Init lock mechanism o(´^｀)o
	init_locks();
	printk("> [INIT] Lock mechanism initialization succeeded.\n");

	// Init trap (^_^)
	init_trap();
	printk("> [INIT] Trap processing initialization succeeded.\n");

	// Init system call table (0_0)
	init_syscall();
	printk("> [INIT] System call initialized successfully.\n");

	// Init screen (QAQ)
	init_screen();
	printk("> [INIT] SCREEN initialization succeeded.\n");

	// TODO: [p2-task4] Setup timer interrupt and enable all interrupt globally
	// NOTE: The function of sstatus.sie is different from sie's

	// TODO: Load tasks by either task id [p1-task3] or task name [p1-task4],
	//   and then execute them.
	add_ready_thread(new Thread(load_task_img_by_name("print1")));
	add_ready_thread(new Thread(load_task_img_by_name("print2")));
	add_ready_thread(new Thread(load_task_img_by_name("fly")));

	// Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
	while (1)
	{
		// If you do non-preemptive scheduling, it's used to surrender control
		do_scheduler();

		// If you do preemptive scheduling, they're used to enable CSR_SIE and wfi
		// enable_preempt();
		// asm volatile("wfi");
	}

	return 0;
}
