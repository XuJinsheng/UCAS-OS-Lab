#include "common.h"
#include <asm.h>
#include <asm/unistd.h>
#include <assert.h>
#include <csr.h>
#include <os/irq.h>
#include <os/kernel.h>
#include <os/loader.h>
#include <os/lock.h>
#include <os/mm.h>
#include <os/sched.h>
#include <os/string.h>
#include <os/task.h>
#include <os/time.h>
#include <printk.h>
#include <screen.h>
#include <sys/syscall.h>
#include <type.h>

extern void ret_from_exception();

// Task info array
task_info_t tasks[TASK_MAXNUM];
short task_num = 0;

static void init_jmptab(void)
{
	volatile long (*(*jmptab))() = (volatile long (*(*))())KERNEL_JMPTAB_BASE;

	jmptab[CONSOLE_PUTSTR] = (long (*)())bios_putstr;
	jmptab[CONSOLE_PUTCHAR] = (long (*)())bios_putchar;
	jmptab[CONSOLE_GETCHAR] = (long (*)())bios_getchar;
	jmptab[SD_READ] = (long (*)())bios_sd_read;
	jmptab[SD_WRITE] = (long (*)())bios_sd_write;
	jmptab[QEMU_LOGGING] = (long (*)())bios_logging;
	jmptab[SET_TIMER] = (long (*)())bios_set_timer;
	jmptab[READ_FDT] = (long (*)())bios_read_fdt;
	jmptab[MOVE_CURSOR] = (long (*)())screen_move_cursor;
	jmptab[PRINT] = (long (*)())printk;
	jmptab[YIELD] = (long (*)())do_scheduler;
	jmptab[MUTEX_INIT] = (long (*)())do_mutex_lock_init;
	jmptab[MUTEX_ACQ] = (long (*)())do_mutex_lock_acquire;
	jmptab[MUTEX_RELEASE] = (long (*)())do_mutex_lock_release;

	// TODO: [p2-task1] (S-core) initialize system call table.
}

static void init_task_info(void)
{
	// TODO: [p1-task4] Init 'tasks' array via reading app-info sector
	// NOTE: You need to get some related arguments from bootblock first
	// the sdcard block_id of task_info_t is stored in 0x502001f0
	// the number of tasks is stored in 0x502001f4
	// assert(sizeof(task_info_t) == 32);
	int block_id = *(int *)0x502001f0;
	bios_sd_read(tasks, 1, block_id);
	task_num = *(short *)0x502001f4;
}

/************************************************************/
static void init_pcb_stack(
	ptr_t kernel_stack, ptr_t user_stack, ptr_t entry_point,
	pcb_t *pcb)
{
	/* TODO: [p2-task3] initialization of registers on kernel stack
	 * HINT: sp, ra, sepc, sstatus
	 * NOTE: To run the task in user mode, you should set corresponding bits
	 *     of sstatus(SPP, SPIE, etc.).
	 */
	regs_context_t *pt_regs =
		(regs_context_t *)(kernel_stack - sizeof(regs_context_t));

	/* TODO: [p2-task1] set sp to simulate just returning from switch_to
	 * NOTE: you should prepare a stack, and push some values to
	 * simulate a callee-saved context.
	 */
	switchto_context_t *pt_switchto =
		(switchto_context_t *)((ptr_t)pt_regs - sizeof(switchto_context_t));
}

static void init_pcb(void)
{
	/* TODO: [p2-task1] load needed tasks and init their corresponding PCB */

	/* TODO: [p2-task1] remember to initialize 'current_running' */
}

static void init_syscall(void)
{
	// TODO: [p2-task3] initialize system call table.
}
/************************************************************/

extern func_t __preinit_array_start[0], __preinit_array_end[0], __init_array_start[0], __init_array_end[0];
int main(void)
{
	// Init C++ static variables
	for (func_t *func = __preinit_array_start; func < __preinit_array_end; func++)
		(*func)();
	for (func_t *func = __init_array_start; func < __init_array_end; func++)
		(*func)();

	// Init jump table provided by kernel and bios(ΦωΦ)
	init_jmptab();

	// Init task information (〃'▽'〃)
	init_task_info();

	// Init Process Control Blocks |•'-'•) ✧
	init_pcb();
	printk("> [INIT] PCB initialization succeeded.\n");

	// Read CPU frequency (｡•ᴗ-)_
	time_base = bios_read_fdt(TIMEBASE);

	// Init lock mechanism o(´^｀)o
	init_locks();
	printk("> [INIT] Lock mechanism initialization succeeded.\n");

	// Init interrupt (^_^)
	init_exception();
	printk("> [INIT] Interrupt processing initialization succeeded.\n");

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
	task_interact();

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
