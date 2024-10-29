#include <CPU.hpp>
#include <arch/bios_func.h>
#include <common.h>
#include <drivers/e1000.h>
#include <drivers/e1000.hpp>
#include <drivers/plic.h>
#include <kalloc.hpp>
#include <net.hpp>
#include <page.hpp>
#include <schedule.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>

static SpinLock net_buffer_lock; // network buffer lock
static SpinLock e1000_lock;		 // e1000 lock
static WaitQueue send_wait_queue, recv_wait_queue;

void init_net()
{
	uint64_t e1000 = bios_read_fdt(ETHERNET_ADDR);
	uint64_t plic_addr = bios_read_fdt(PLIC_ADDR);
	uint32_t nr_irqs = (uint32_t)bios_read_fdt(NR_IRQS);
	plic_addr = (ptr_t)ioremap(plic_addr, 0x4000 * PAGE_SIZE);
	e1000 = (ptr_t)ioremap(e1000, 8 * PAGE_SIZE);
	plic_init(plic_addr, nr_irqs);
	e1000_init((void *)e1000);
}
void handle_e1000_irq()
{
	uint32_t icr = e1000_read_reg(e1000, E1000_ICR);
	uint32_t ims = e1000_read_reg(e1000, E1000_IMS);
	if (icr & ims & E1000_ICR_RXDMT0)
	{
		net_buffer_lock.lock();
		recv_wait_queue.wakeup_all();
		net_buffer_lock.unlock();
	}
	if (icr & ims & E1000_ICR_TXQE)
	{
		e1000_write_reg(e1000, E1000_IMC, E1000_IMC_TXQE);
		net_buffer_lock.lock();
		send_wait_queue.wakeup_all();
		net_buffer_lock.unlock();
	}
}
void handle_ext_irq()
{
	uint32_t id = plic_claim();
	switch (id)
	{
	case PLIC_E1000_PYNQ_IRQ:
	case PLIC_E1000_QEMU_IRQ:
		handle_e1000_irq();
		break;
	default:
		break;
	}
	plic_complete(id);
}

long Syscall::sys_net_send(void *txpacket, long length)
{
	net_buffer_lock.lock();
	while (e1000_transmit(txpacket, length) == -1)
	{
		// Enable TXQE interrupt if transmit queue is full
		e1000_write_reg(e1000, E1000_IMS, E1000_IMS_TXQE);
		send_wait_queue.push(current_cpu->current_thread);
		current_cpu->current_thread->block();
		net_buffer_lock.unlock();
		do_scheduler();
		net_buffer_lock.lock();
	}
	net_buffer_lock.unlock();
	return length;
}
long Syscall::sys_net_recv(void *rxbuffer, long pkt_num, int *pkt_lens)
{
	long offset = 0;
	uint8_t *rx = (uint8_t *)rxbuffer;
	for (long i = 0; i < pkt_num; i++)
	{
		net_buffer_lock.lock();
		while ((pkt_lens[i] = e1000_poll(rx + offset)) == -1)
		{
			recv_wait_queue.push(current_cpu->current_thread);
			current_cpu->current_thread->block();
			net_buffer_lock.unlock();
			do_scheduler();
			net_buffer_lock.lock();
		}
		net_buffer_lock.unlock();
		offset += pkt_lens[i];
	}
	return offset;
}