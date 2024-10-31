#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <fs/fs.hpp>
#include <kalloc.hpp>
#include <page.hpp>
#include <string.h>

void read_block(void *dest, uint32_t blockid, uint32_t blocks) // do not use user space pointer
{
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	blocks *= 8;
	assert(blockid < BLOCK_START / SECTOR_SIZE * 2); // 1GB
	assert((ptr_t)dest % BLOCK_SIZE == 0);
	assert((long)dest < 0); // high space
	bios_sd_read((void *)kva2pa((ptr_t)dest), blocks, blockid);
}
void write_block(void *src, uint32_t blockid, uint32_t blocks) // do not use user space pointer
{
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	blocks *= 8;
	assert(blockid < BLOCK_START / SECTOR_SIZE * 2); // 1GB
	assert((ptr_t)src % BLOCK_SIZE == 0);
	assert((long)src < 0); // high space
	bios_sd_write((void *)kva2pa((ptr_t)src), blocks, blockid);
}