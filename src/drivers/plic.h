#ifndef PLIC_H
#define PLIC_H

#include <common.h>

#define PLIC_E1000_PYNQ_IRQ 3
#define PLIC_E1000_QEMU_IRQ 33

#define MAX_DEVICES 1024
#define MAX_CONTEXTS 15872

/*
 * Each interrupt source has a priority register associated with it.
 * We always hardwire it to one in Linux.
 */
#define PRIORITY_BASE 0
#define PRIORITY_PER_ID 4

/*
 * Each hart context has a vector of interrupt enable bits associated with it.
 * There's one bit for each interrupt source.
 */
#define ENABLE_BASE 0x2000
#define ENABLE_PER_HART 0x80

/*
 * Each hart context has a set of control registers associated with it.  Right
 * now there's only two: a source priority threshold over which the hart will
 * take an interrupt, and a register to claim interrupts.
 */
#define CONTEXT_BASE 0x200000
#define CONTEXT_PER_HART 0x1000
#define CONTEXT_THRESHOLD 0x00
#define CONTEXT_CLAIM 0x04

__BEGIN_DECLS

extern int plic_init(uint64_t plic_regs_addr, uint32_t nr_irqs);

/*
 * Handling an interrupt is a two-step process: first you claim the interrupt
 * by reading the claim register, then you complete the interrupt by writing
 * that source ID back to the same claim register.  This automatically enables
 * and disables the interrupt, so there's nothing else to do.
 */
extern uint32_t plic_claim(void);
extern void plic_complete(int hwirq);
__END_DECLS

#endif // !PLIC_H
