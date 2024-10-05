#include <arch/bios_func.h>
#include <arch/biosdef.h>
#include <common.h>

#define BIOS_FUNC_ENTRY 0x50150000
#define IGNORE 0

static long call_bios(long which, long arg0, long arg1, long arg2, long arg3, long arg4)
{
	long (*bios_func)(long, long, long, long, long, long, long, long) =
		(long (*)(long, long, long, long, long, long, long, long))BIOS_FUNC_ENTRY;

	return bios_func(arg0, arg1, arg2, arg3, arg4, IGNORE, IGNORE, which);
}

void bios_putchar(char ch)
{
	call_bios((long)BIOS_PUTCHAR, (long)ch, IGNORE, IGNORE, IGNORE, IGNORE);
}

void bios_putstr(char *str)
{
	call_bios((long)BIOS_PUTSTR, (long)str, IGNORE, IGNORE, IGNORE, IGNORE);
}

int bios_getchar(void)
{
	return call_bios((long)BIOS_GETCHAR, IGNORE, IGNORE, IGNORE, IGNORE, IGNORE);
}

int bios_sd_read(void *mem_address, unsigned num_of_blocks, unsigned block_id)
{
	return (int)call_bios((long)BIOS_SDREAD, (long)mem_address, (long)num_of_blocks, (long)block_id, IGNORE, IGNORE);
}

int bios_sd_write(void *mem_address, unsigned num_of_blocks, unsigned block_id)
{
	return (int)call_bios((long)BIOS_SDWRITE, (long)mem_address, (long)num_of_blocks, (long)block_id, IGNORE, IGNORE);
}

void bios_set_timer(uint64_t stime_value)
{
	call_bios((long)BIOS_SETTIMER, (long)stime_value, IGNORE, IGNORE, IGNORE, IGNORE);
}

uint64_t bios_read_fdt(enum FDT_TYPE type)
{
	return (uint64_t)call_bios((long)BIOS_FDTREAD, (long)type, IGNORE, IGNORE, IGNORE, IGNORE);
}

void bios_logging(char *str)
{
	call_bios((long)BIOS_LOGGING, (long)str, IGNORE, IGNORE, IGNORE, IGNORE);
}