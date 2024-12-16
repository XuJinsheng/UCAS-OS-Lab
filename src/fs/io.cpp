#include <arch/bios_func.h>
#include <assert.h>
#include <common.h>
#include <fs/fs.hpp>
#include <kalloc.hpp>
#include <page.hpp>
#include <string.h>

namespace FS
{

void read_block_direct(void *dest, uint32_t blockid) // do not use user space pointer
{
	assert(blockid < BLOCK_END / SECTOR_SIZE); // 1GB
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	assert((ptr_t)dest % BLOCK_SIZE == 0);
	assert((long)dest < 0); // high space
	bios_sd_read((void *)kva2pa((ptr_t)dest), 8, blockid);
}
void write_block_direct(const void *src, uint32_t blockid) // do not use user space pointer
{
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	assert(blockid < BLOCK_START / SECTOR_SIZE * 2); // 1GB
	assert((ptr_t)src % BLOCK_SIZE == 0);
	assert((long)src < 0); // high space
	bios_sd_write((void *)kva2pa((ptr_t)src), 8, blockid);
}

constexpr int CACHE_SIZE = 32;

long age = 0;
void *cache_data[CACHE_SIZE];
uint32_t cache_blockid[CACHE_SIZE];
bool cache_dirty[CACHE_SIZE];
long cache_age[CACHE_SIZE];

void cache_init()
{
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		cache_data[i] = kalloc(BLOCK_SIZE);
		cache_blockid[i] = -1;
		cache_dirty[i] = false;
		cache_age[i] = -1;
	}
}
int cache_find(uint32_t blockid)
{
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		if (cache_blockid[i] == blockid)
		{
			cache_age[i] = age++;
			return i;
		}
	}
	int selected = 0;
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		if (cache_age[i] < cache_age[selected])
		{
			selected = i;
		}
	}
	if (cache_dirty[selected])
	{
		write_block_direct(cache_data[selected], cache_blockid[selected]);
	}
	cache_blockid[selected] = blockid;
	read_block_direct(cache_data[selected], blockid);
	cache_dirty[selected] = false;
	cache_age[selected] = age++;
	return selected;
}

void read_block(void *dest, uint32_t blockid)
{
	int i = cache_find(blockid);
	memcpy(dest, cache_data[i], BLOCK_SIZE);
}
void write_block(void *src, uint32_t blockid)
{
	int i = cache_find(blockid);
	memcpy(cache_data[i], src, BLOCK_SIZE);
	write_block_direct(src, blockid);
}
} // namespace FS
