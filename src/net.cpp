#include <CPU.hpp>
#include <common.h>
#include <drivers/e1000.hpp>
#include <drivers/plic.h>
#include <net.hpp>
#include <page.hpp>
#include <spinlock.hpp>
#include <syscall.hpp>
#include <thread.hpp>

static SpinLock net_buffer_lock; // network buffer lock
static SpinLock e1000_lock;		 // e1000 lock
static WaitQueue send_wait_queue, recv_wait_queue;

void init_net()
{
}

void handle_ext_irq()
{
}

long Syscall::sys_net_send(void *txpacket, long length)
{
}
long Syscall::sys_net_recv(void *rxbuffer, long pkt_num, int *pkt_lens)
{
}