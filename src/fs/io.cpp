#include "time.hpp"
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
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	assert(blockid < BLOCK_END / SECTOR_SIZE);
	assert((ptr_t)dest % BLOCK_SIZE == 0);
	assert((long)dest < 0); // high space
	bios_sd_read((void *)kva2pa((ptr_t)dest), 8, blockid);
}
void write_block_direct(const void *src, uint32_t blockid) // do not use user space pointer
{
	blockid = BLOCK_START / SECTOR_SIZE + blockid * 8;
	assert(blockid < BLOCK_END / SECTOR_SIZE);
	assert((ptr_t)src % BLOCK_SIZE == 0);
	assert((long)src < 0); // high space
	bios_sd_write((void *)kva2pa((ptr_t)src), 8, blockid);
}

constexpr int CACHE_SIZE = 64;

long age = 0;
void *cache_data[CACHE_SIZE];
uint32_t cache_blockid[CACHE_SIZE];
bool cache_dirty[CACHE_SIZE];
long cache_age[CACHE_SIZE];

SpinLock cache_lock;

bool policy_writethrough = true;
long policy_writeback_interval = 0;

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
	lock_guard guard(cache_lock);
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

Block::Block(int blockid)
{
	cacheid = cache_find(blockid);
	data = (uint8_t *)cache_data[cacheid];
}
Block::~Block()
{
}
void Block::update()
{
	lock_guard guard(cache_lock);
	if (policy_writethrough)
		write_block_direct(cache_data[cacheid], cache_blockid[cacheid]);
	else
		cache_dirty[cacheid] = true;
}
void cache_writeback()
{
	for (int i = 0; i < CACHE_SIZE; i++)
	{
		if (cache_dirty[i])
		{
			write_block_direct(cache_data[i], cache_blockid[i]);
			cache_dirty[i] = false;
		}
	}
}
void cache_scan_timer()
{
	static long last_time = 0;
	if (policy_writethrough)
		return;
	if (!cache_lock.try_lock())
		return;
	if (last_time + policy_writeback_interval < get_timer())
	{
		last_time = get_timer();
		cache_writeback();
	}
	cache_lock.unlock();
}
void cache_set_policy(int policy_seconds)
{
	lock_guard guard(cache_lock);
	if (policy_seconds == -1)
	{
		policy_writethrough = true;
		policy_writeback_interval = 0;
		cache_writeback();
	}
	else if (policy_seconds >= 0)
	{
		policy_writethrough = false;
		policy_writeback_interval = policy_seconds;
	}
}
} // namespace FS
