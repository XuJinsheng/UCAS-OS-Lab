#ifndef ARCH_IO_H
#define ARCH_IO_H

#define RISCV_FENCE(p, s) __asm__ __volatile__("fence " #p "," #s : : : "memory")

/*
 * Generic virtual read/write.  Note that we don't support half-word
 * read/writes.  We define __arch_*[bl] here, and leave __arch_*w
 * to the architecture specific code.
 */
#define __arch_getb(a) (*(unsigned char *)(a))
#define __arch_getw(a) (*(unsigned short *)(a))
#define __arch_getl(a) (*(unsigned int *)(a))
#define __arch_getq(a) (*(unsigned long long *)(a))

#define __arch_putb(v, a) (*(unsigned char *)(a) = (v))
#define __arch_putw(v, a) (*(unsigned short *)(a) = (v))
#define __arch_putl(v, a) (*(unsigned int *)(a) = (v))
#define __arch_putq(v, a) (*(unsigned long long *)(a) = (v))

static inline void writeb(unsigned char val, volatile void *addr)
{
	RISCV_FENCE(ow, ow);
	__arch_putb(val, addr);
}

static inline void writew(unsigned short val, volatile void *addr)
{
	RISCV_FENCE(ow, ow);
	__arch_putw(val, addr);
}

static inline void writel(unsigned int val, volatile void *addr)
{
	RISCV_FENCE(ow, ow);
	__arch_putl(val, addr);
}

static inline void writeq(unsigned long long val, volatile void *addr)
{
	RISCV_FENCE(ow, ow);
	__arch_putq(val, addr);
}

static inline unsigned char readb(const volatile void *addr)
{
	unsigned char val;

	val = __arch_getb(addr);
	RISCV_FENCE(ir, ir);
	return val;
}

static inline unsigned short readw(const volatile void *addr)
{
	unsigned short val;

	val = __arch_getw(addr);
	RISCV_FENCE(ir, ir);
	return val;
}

static inline unsigned int readl(const volatile void *addr)
{
	unsigned int val;

	val = __arch_getl(addr);
	RISCV_FENCE(ir, ir);
	return val;
}

static inline unsigned long long readq(const volatile void *addr)
{
	unsigned long long val;

	val = __arch_getq(addr);
	RISCV_FENCE(ir, ir);
	return val;
}

static inline void local_flush_dcache()
{
	RISCV_FENCE(iorw, iorw);
}

#endif // ARCH_IO_H