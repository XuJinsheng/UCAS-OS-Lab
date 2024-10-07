#include <arch/CSR.h>
#include <arch/trap_entry.h>
#include <assert.h>
#include <common.h>
#include <kstdio.h>
#include <syscall.hpp>
#include <thread.hpp>
#include <trap.hpp>

void init_trap()
{
	csr_clear(CSR_SSTATUS, SR_SPP);
	csr_set(CSR_SSTATUS, SR_SPIE);
	csr_write(CSR_STVEC, user_trap_entry);
}

void handle_other(user_context_reg_t *regs)
{
	const char *reg_name[] = {" ra  ", " sp  ", " gp  ", " tp  ", " t0  ", " t1  ", " t2  ", "s0/fp",
							  " s1  ", " a0  ", " a1  ", " a2  ", " a3  ", " a4  ", " a5  ", " a6  ",
							  " a7  ", " s2  ", " s3  ", " s4  ", " s5  ", " s6  ", " s7  ", " s8  ",
							  " s9  ", " s10 ", " s11 ", " t3  ", " t4  ", " t5  ", " t6  "};
	for (int i = 0; i < 31; i += 3)
	{
		for (int j = 0; j < 3 && i + j < 31; ++j)
		{
			printk("%s : %016lx ", reg_name[i + j], regs->regs[i + j]);
		}
		printk("\n\r");
	}
	printk("sstatus: 0x%lx stval: 0x%lx scause: %lu\n\r", regs->sstatus, regs->stval, regs->scause);
	printk("sepc: 0x%lx\n\r", regs->sepc);
	assert(0);
}

void user_trap_handler()
{
	user_context_reg_t &regs = current_running->user_context;
	switch (regs.scause)
	{
	case EXC_SYSCALL: {
		ptr_t res = handle_syscall(regs.regs + 9);
		regs.regs[9] = res;
		regs.sepc += 4;
		break;
	}
	default:
		handle_other(&regs);
		break;
	}
}