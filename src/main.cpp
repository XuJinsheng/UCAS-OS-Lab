#include <arch/bios_func.h>
#include <common.h>
#include <drivers/screen.h>
#include <fs/fs.hpp>
#include <kalloc.hpp>
#include <kstdio.h>
#include <locks.hpp>
#include <net.hpp>
#include <schedule.hpp>
#include <syscall.hpp>
#include <task_loader.hpp>
#include <thread.hpp>
#include <time.hpp>
#include <trap.hpp>

extern func_t __preinit_array_start[0], __preinit_array_end[0], __init_array_start[0], __init_array_end[0];
extern ptr_t __bss_start[0], __BSS_END__[0];
int main(int hartid)
{
	if (!hartid)
	{
		// Clear BSS section
		for (ptr_t *bss = __bss_start; bss < __BSS_END__; bss++)
			*bss = 0;

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
		init_processor(hartid);
		printk("> [INIT] PCB initialization succeeded.\n");

		// Init screen (QAQ)
		init_screen();
		printk("> [INIT] SCREEN initialization succeeded.\n");

		// Setup timer interrupt, Read CPU frequency (｡•ᴗ-)_
		init_timer();
		printk("> [INIT] Timer initialization succeeded.\n");

		// Init lock mechanism o(´^｀)o
		init_locks();
		printk("> [INIT] Lock mechanism initialization succeeded.\n");

		// Init trap (^_^)
		init_trap();
		printk("> [INIT] Trap processing initialization succeeded.\n");

		// Init system call table (0_0)
		init_syscall();
		printk("> [INIT] System call initialized successfully.\n");

		// Init PLIC and network (QAQ)
		init_net();
		printk("> [INIT] Network initialization succeeded.\n");

		// Init file system (QAQ)
		if (init_filesystem())
			printk("> [INIT] File system found.\n");
		else
			printk("> [INIT] File system not found, creating a new one.\n");

		// Create the first user process
		Syscall::sys_exec("shell", 0, nullptr);

		// Wake up other cores, this core has masked SIP
		bios_send_ipi(nullptr);
	}
	else
	{
		init_processor(hartid);
		init_timer();
		init_trap();
		printk("> [INIT] Core %d, mhartid=%d initialization succeeded.\n", current_cpu->cpu_id, hartid);
	}

	// Infinite while loop, where CPU stays in a low-power state (QAQQQQQQQQQQQ)
	while (1)
	{
		disable_preempt();
		idle_cleanup();
		enable_preempt();
		asm volatile("wfi");
	}

	return 0;
}
