#include <arch/trap_entry.h>
#include <common.h>
#include <kalloc.hpp>
#include <thread.hpp>

void init_pcb()
{
	current_running = new Thread((ptr_t) nullptr);
	current_running->status = Thread::TASK_BLOCKED;
}

static int next_pid = 1;

Thread::Thread(ptr_t start_address)
	: user_context(), kernel_stack_top(0), cursor_x(0), cursor_y(0), pid(0), status(TASK_READY), wakeup_time(0)
{
	pid = next_pid++;
	user_context.sepc = start_address;
	user_context.regs[1] = (ptr_t)allocUserPage(1) + PAGE_SIZE;
	kernel_stack_top = (ptr_t)allocKernelPage(1) + PAGE_SIZE;
	kernel_stack_top -= 13 * sizeof(ptr_t);
	*(ptr_t *)kernel_stack_top = (ptr_t)user_trap_return;
}
