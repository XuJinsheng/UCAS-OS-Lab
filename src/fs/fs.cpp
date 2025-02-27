#include <CPU.hpp>
#include <assert.h>
#include <common.h>
#include <fs/fs.hpp>
#include <kstdio.h>
#include <process.hpp>
#include <string.h>
#include <thread.hpp>

namespace FS
{

SuperBlock superblock;
uint &cwd_node()
{
	return current_process->cwd_node_idx;
}

bool init_filesystem()
{
	cache_init();
	Block block(0);
	SuperBlock *sb = (SuperBlock *)block.data;
	if (sb->magic0 == SUPERBLOCK_MAGIC && sb->magic1 == SUPERBLOCK_MAGIC)
	{
		superblock = *sb;
		cwd_node() = superblock.root_inode;
		return true;
	}
	else
	{
		fs_mkfs();
		return false;
	}
}
void flush_superblock()
{
	Block block(0);
	SuperBlock *sb = (SuperBlock *)block.data;
	*sb = superblock;
	block.update();
}
int fs_mkfs()
{
	superblock.magic0 = SUPERBLOCK_MAGIC;
	superblock.fs_start = BLOCK_START;
	superblock.inode_map_start_block = 1;
	superblock.inode_map_size = MAX_INODE_NUM;
	superblock.inode_allocated = 0;
	superblock.inode_free_min = 0;
	superblock.block_map_start_block = superblock.inode_map_start_block + MAX_INODE_NUM / (8 * BLOCK_SIZE);
	superblock.block_map_size = MAX_BLOCK_NUM;
	superblock.inode_start_block = superblock.block_map_start_block + MAX_BLOCK_NUM / (8 * BLOCK_SIZE);
	superblock.block_start_block = superblock.inode_start_block + MAX_INODE_NUM / INODE_PER_BLOCK;
	superblock.block_allocated = superblock.block_start_block;
	assert(superblock.block_start_block < BLOCK_SIZE * 8);
	superblock.block_free_min = superblock.block_allocated + 1;
	superblock.magic1 = SUPERBLOCK_MAGIC;

	for (uint32_t i = 0; i < superblock.inode_map_size / (8 * BLOCK_SIZE); i++)
	{
		Block block(superblock.inode_map_start_block + i);
		memset(block.data, 0, BLOCK_SIZE);
		block.update();
	}
	{
		Block block(superblock.block_map_start_block);
		memset(block.data, 0, BLOCK_SIZE);
		for (int i = 0; i < superblock.block_start_block; i++)
			block.data[i / 8] |= 1 << (i % 8);
		block.update();
	}
	for (uint32_t i = 1; i < superblock.block_map_size / (8 * BLOCK_SIZE); i++)
	{
		Block block(superblock.block_map_start_block + i);
		memset(block.data, 0, BLOCK_SIZE);
		block.update();
	}

	superblock.root_inode = inode_alloc();
	cwd_node() = superblock.root_inode;
	flush_superblock();

	Inode root_inode = {.type = 1, .link_cnt = 65535};
	DirEntry dir[2] = {DirEntry{cwd_node(), 1, "."}, DirEntry{cwd_node(), 1, ".."}};
	inode_modify_data(true, root_inode, dir, 0, sizeof(dir));
	write_inode(superblock.root_inode, root_inode);
	return 0;
}
int fs_statfs()
{
	printk("[statfs]:\n");
	printk("magic0: %x\n", superblock.magic0);
	printk("fs_start: %x\n", superblock.fs_start);
	printk("inode_map: start_block: %x, size: %x, allocated: %x, free_min: %x\n", superblock.inode_map_start_block,
		   superblock.inode_map_size, superblock.inode_allocated, superblock.inode_free_min);
	printk("block_map: start_block: %x, size: %x, allocated: %x, free_min: %x\n", superblock.block_map_start_block,
		   superblock.block_map_size, superblock.block_allocated, superblock.block_free_min);
	printk("inode: start_block: %x, block: start_block: %x\n", superblock.inode_start_block,
		   superblock.block_start_block);
	printk("root_inode: %x\n", superblock.root_inode);
	printk("magic1: %x\n", superblock.magic1);
	return 0;
}

int inode_alloc()
{
	for (uint i = superblock.inode_free_min / (BLOCK_SIZE * 8); i < superblock.inode_map_size / (BLOCK_SIZE * 8); i++)
	{
		Block block(superblock.inode_map_start_block + i);
		for (uint j = 0; j < BLOCK_SIZE; j++)
		{
			if (block.data[j] != 0xff)
			{
				for (uint k = 0; k < 8; k++)
				{
					if ((block.data[j] & (1 << k)) == 0)
					{
						block.data[j] |= 1 << k;
						block.update();
						int inode = i * BLOCK_SIZE * 8 + j * 8 + k;
						superblock.inode_allocated++;
						superblock.inode_free_min = inode + 1;
						flush_superblock();
						return inode;
					}
				}
			}
		}
	}
	assert(0); // no space for inode
	return -1;
}
void inode_free(uint ino)
{
	assert(ino < MAX_INODE_NUM);
	uint i = ino / (BLOCK_SIZE * 8);
	uint j = ino % (BLOCK_SIZE * 8) / 8;
	uint k = ino % 8;
	Block block(superblock.inode_map_start_block + i);
	assert(block.data[j] & (1 << k));
	block.data[j] &= ~(1 << k);
	block.update();
	superblock.inode_allocated--;
	if (ino < superblock.inode_free_min)
		superblock.inode_free_min = ino;
	flush_superblock();
}

int block_alloc()
{
	for (uint i = superblock.block_free_min / (BLOCK_SIZE * 8); i < superblock.block_map_size / (BLOCK_SIZE * 8); i++)
	{
		Block block(superblock.block_map_start_block + i);
		for (uint j = 0; j < BLOCK_SIZE; j++)
		{
			if (block.data[j] != 0xff)
			{
				for (uint k = 0; k < 8; k++)
				{
					if ((block.data[j] & (1 << k)) == 0)
					{
						block.data[j] |= 1 << k;
						block.update();
						int blockid = i * BLOCK_SIZE * 8 + j * 8 + k;
						superblock.block_allocated++;
						superblock.block_free_min = blockid + 1;
						flush_superblock();
						return blockid;
					}
				}
			}
		}
	}
	assert(0); // no space for block
	return -1;
}
void block_free(uint blockid)
{
	assert(blockid < MAX_BLOCK_NUM);
	uint i = blockid / (BLOCK_SIZE * 8);
	uint j = blockid % (BLOCK_SIZE * 8) / 8;
	uint k = blockid % 8;
	Block block(superblock.block_map_start_block + i);
	assert(block.data[j] & (1 << k));
	block.data[j] &= ~(1 << k);
	block.update();
	superblock.block_allocated--;
	if (blockid < superblock.block_free_min)
		superblock.block_free_min = blockid;
	flush_superblock();
}

Inode read_inode(uint ino)
{
	int i = superblock.inode_start_block + ino / INODE_PER_BLOCK;
	Block block(i);
	return ((Inode *)block.data)[ino % INODE_PER_BLOCK];
}

void write_inode(uint ino, const Inode &inode)
{
	int i = superblock.inode_start_block + ino / INODE_PER_BLOCK;
	Block block(i);
	((Inode *)block.data)[ino % INODE_PER_BLOCK] = inode;
	block.update();
}

constexpr size_t RATIO_PER_LEVEL = BLOCK_SIZE / sizeof(uint);
constexpr size_t L0_SIZE = BLOCK_SIZE, L1_SIZE = L0_SIZE * RATIO_PER_LEVEL, L2_SIZE = L1_SIZE * RATIO_PER_LEVEL,
				 L3_SIZE = L2_SIZE * RATIO_PER_LEVEL;
constexpr size_t L0_BOUND = INODE_DATA_DIRECT * BLOCK_SIZE, L1_BOUND = L0_BOUND + INODE_DATA_L1 * L1_SIZE,
				 L2_BOUND = L1_BOUND + INODE_DATA_L2 * L2_SIZE, L3_BOUND = L2_BOUND + INODE_DATA_L3 * L3_SIZE;
bool inode_modify_data_recur(bool is_write, uint &blockid, void *data, uint offset, uint length, uint depth)
{
	bool modified = false;
	bool allocated_new_block = false;
	if (blockid == 0)
	{
		blockid = block_alloc();
		allocated_new_block = true;
		modified = true;
	}
	Block block(blockid);
	if (allocated_new_block)
	{
		memset(block.data, 0, BLOCK_SIZE);
	}
	if (depth == 0)
	{
		if (is_write)
		{
			memcpy(block.data + offset, data, length);
			block.update();
		}
		else
			memcpy(data, block.data + offset, length);
	}
	else
	{
		uint *data = (uint *)block.data;
		size_t size = 0;
		switch (depth)
		{
		case 1:
			size = L0_SIZE;
			break;
		case 2:
			size = L1_SIZE;
			break;
		case 3:
			size = L2_SIZE;
			break;
		default:
			assert(0);
		}
		bool block_modified = false;
		block_modified = inode_modify_data_recur(is_write, data[offset / size], data, offset % size, length, depth - 1);
		if (block_modified)
			block.update();
	}
	return modified;
}
bool inode_modify_data(bool is_write, Inode &inode, void *data, uint offset, uint length)
{
	assert(offset % BLOCK_SIZE + length <= BLOCK_SIZE); // no cross block
	// L3 is actually not used
	bool inode_modified = false;
	if ((offset + length > inode.size) && inode.type == 2)
	{
		inode.size = offset + length;
		inode_modified = true;
	}
	if (offset < L0_BOUND)
	{
		inode_modified |=
			inode_modify_data_recur(is_write, inode.data_direct[offset / L0_SIZE], data, offset % L0_SIZE, length, 0);
	}
	else if (offset < L1_BOUND)
	{
		offset -= L0_BOUND;
		inode_modified |=
			inode_modify_data_recur(is_write, inode.data_l1[offset / L1_SIZE], data, offset % L1_SIZE, length, 1);
	}
	else if (offset < L2_BOUND)
	{
		offset -= L1_BOUND;
		inode_modified |=
			inode_modify_data_recur(is_write, inode.data_l2[offset / L2_SIZE], data, offset % L2_SIZE, length, 2);
	}
	else
		assert(0); // L3 is actually not used

	return inode_modified;
}

int get_inode_by_filename(const char *path, bool create_if_not_existed, uint new_inode_idx)
{
	Inode cwd = read_inode(cwd_node());
	uint free_offset = 0;
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (cwd.data_direct[i])
		{
			Block block(cwd.data_direct[i]);
			DirEntry *dir = (DirEntry *)block.data;
			for (int j = 0; j < DIRENTRY_PER_BLOCK; j++)
			{
				if (dir[j].valid && strcmp(dir[j].name, path) == 0)
					return dir[j].inode;
				else if (!dir[j].valid && free_offset == 0)
					free_offset = i * DIRENTRY_PER_BLOCK + j;
			}
		}
	if (free_offset == 0)
	{
		for (int i = 0; i < INODE_DATA_DIRECT; i++)
			if (cwd.data_direct[i] == 0)
			{
				free_offset = i * DIRENTRY_PER_BLOCK;
				break;
			}
	}
	if (!create_if_not_existed)
		return -1;
	// assert(free_offset != 0); // no space for new file
	if (new_inode_idx == 0)
	{
		new_inode_idx = inode_alloc();
		write_inode(new_inode_idx, Inode{});
	}
	DirEntry dir = {new_inode_idx, 1};
	strcpy(dir.name, path);
	inode_modify_data(true, cwd, &dir, free_offset * sizeof(DirEntry), sizeof(DirEntry));
	cwd.size++;
	write_inode(cwd_node(), cwd);
	return new_inode_idx;
}
void remove_inode_by_filename(const char *path)
{
	Inode cwd = read_inode(cwd_node());
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (cwd.data_direct[i])
		{
			Block block(cwd.data_direct[i]);
			DirEntry *dir = (DirEntry *)block.data;
			for (int j = 0; j < DIRENTRY_PER_BLOCK; j++)
			{
				if (dir[j].valid && strcmp(dir[j].name, path) == 0)
				{
					dir[j].valid = 0;
					block.update();
					cwd.size--;
					write_inode(cwd_node(), cwd);
					return;
				}
			}
		}
	assert(0); // no such file
}

int fs_cd(const char *path)
{
	char tmp[64];
	uint old_cwd = cwd_node();
	const char *p = path;
	while (*p)
	{
		while (*p != '/' && *p)
			p++;
		memcpy(tmp, path, p - path);
		tmp[p - path] = 0;
		cwd_node() = get_inode_by_filename(tmp, false);
		if ((int)cwd_node() < 0)
		{
			cwd_node() = old_cwd;
			printk("No such directory\n");
			return -1;
		}
		Inode inode = read_inode(cwd_node());
		if (inode.type != 1)
		{
			cwd_node() = old_cwd;
			printk("Not a directory\n");
			return -1;
		}
		if (*p == '/')
			p++;
		path = p;
	}
	return 0;
}
int fs_ls(const char *path, int option)
{
	uint old_cwd = cwd_node();
	if (fs_cd(path) == -1)
	{
		cwd_node() = old_cwd;
		return -1;
	}

	Inode inode = read_inode(cwd_node());
	if (inode.type != 1)
	{
		printk("Not a directory\n");
		cwd_node() = old_cwd;
		return -1;
	}

	bool opt_l = option & LS_L;
	if (opt_l)
	{
		printk("| mode | links |   size   |      name    |\n");
	}
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (inode.data_direct[i])
		{
			Block block(inode.data_direct[i]);
			DirEntry *dir = (DirEntry *)block.data;
			for (int j = 0; j < DIRENTRY_PER_BLOCK; j++)
			{
				if (dir[j].valid)
				{
					if (opt_l)
					{
						Inode n = read_inode(dir[j].inode);
						printk("| %4d | %5d | %8d | %12s |\n", n.type, n.link_cnt, n.size, dir[j].name);
					}
					else
					{
						printk("%s ", dir[j].name);
					}
				}
			}
		}
	printk("\n");
	cwd_node() = old_cwd;
	return 0;
}

int fs_mkdir(const char *path)
{
	int inode_idx = get_inode_by_filename(path, true);
	if (inode_idx < 0)
	{
		printk("Failed to create directory\n");
		return -1;
	}
	Inode inode = read_inode(inode_idx);
	if (inode.type != 0)
	{
		printk("Already existed\n");
		return -1;
	}
	inode.type = 1;
	inode.link_cnt = 1;
	DirEntry dir[2] = {DirEntry{(uint)inode_idx, 1, "."}, DirEntry{cwd_node(), 1, ".."}};
	inode_modify_data(true, inode, dir, 0, sizeof(dir));
	write_inode(inode_idx, inode);
	return 0;
}
int fs_touch(const char *path)
{
	int inode_idx = get_inode_by_filename(path, true);
	if (inode_idx < 0)
	{
		printk("Failed to create file\n");
		return -1;
	}
	Inode inode = read_inode(inode_idx);
	if (inode.type != 0)
	{
		printk("Already existed\n");
		return -1;
	}
	inode.type = 2;
	inode.link_cnt = 1;
	write_inode(inode_idx, inode);
	return 0;
}

void free_inode_blocks_recur(uint blockid, size_t depth)
{
	if (depth == 0)
	{
		if (blockid)
			block_free(blockid);
		return;
	}
	Block block(blockid);
	uint *data = (uint *)block.data;
	for (int i = 0; i < BLOCK_SIZE / sizeof(uint); i++)
		if (data[i])
			free_inode_blocks_recur(data[i], depth - 1);
}
void free_inode_blocks(uint ino, Inode &inode)
{
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (inode.data_direct[i])
			free_inode_blocks_recur(inode.data_direct[i], 0);
	for (int i = 0; i < INODE_DATA_L1; i++)
		if (inode.data_l1[i])
			free_inode_blocks_recur(inode.data_l1[i], 1);
	for (int i = 0; i < INODE_DATA_L2; i++)
		if (inode.data_l2[i])
			free_inode_blocks_recur(inode.data_l2[i], 2);
	if (inode.data_l3)
		free_inode_blocks_recur(inode.data_l3, 3);
}
int fs_rm(const char *path)
{
	int inode = get_inode_by_filename(path, false);
	if (inode < 0)
	{
		printk("No such file\n");
		return -1;
	}
	Inode inode_info = read_inode(inode);
	if (inode_info.type != 2)
	{
		printk("Not a file\n");
		return -1;
	}
	remove_inode_by_filename(path);
	inode_info.link_cnt--;
	if (inode_info.link_cnt != 0)
	{
		write_inode(inode, inode_info);
		return 0;
	}
	free_inode_blocks(inode, inode_info);
	inode_free(inode);
	return 0;
}
int fs_rmdir(const char *path)
{
	int inode_idx = get_inode_by_filename(path, false);
	if (inode_idx < 0)
	{
		printk("No such directory\n");
		return -1;
	}
	Inode inode = read_inode(inode_idx);
	if (inode.type != 1)
	{
		printk("Not a directory\n");
		return -1;
	}
	if (inode.size != 0)
	{
		printk("Directory not empty\n");
		return -1;
	}
	remove_inode_by_filename(path);
	inode.link_cnt--;
	if (inode.link_cnt != 0)
	{
		write_inode(inode_idx, inode);
		return 0;
	}
	free_inode_blocks(inode_idx, inode);
	inode_free(inode_idx);
	return 0;
}

int fs_ln(const char *src_path, const char *dst_path)
{
	int src_inode = get_inode_by_filename(src_path, false);
	if (src_inode < 0)
	{
		printk("No such source file\n");
		return -1;
	}
	int dst_inode = get_inode_by_filename(dst_path, true, src_inode);
	if (dst_inode < 0)
	{
		printk("Failed to create link\n");
		return -1;
	}
	Inode inode = read_inode(src_inode);
	inode.link_cnt++;
	write_inode(src_inode, inode);
	return 0;
}

static char cat_buffer[BLOCK_SIZE] __attribute__((aligned(4096)));
int fs_cat(const char *path)
{
	int inode = get_inode_by_filename(path, false);
	if (inode < 0)
	{
		printk("No such file\n");
		return -1;
	}
	Inode inode_info = read_inode(inode);
	if (inode_info.type != 2)
	{
		printk("Not a file\n");
		return -1;
	}
	int offset = 0;
	while (offset < inode_info.size)
	{
		int read_size = std::min(BLOCK_SIZE, (size_t)inode_info.size - offset);
		inode_modify_data(false, inode_info, cat_buffer, offset, read_size);
		putstr((char *)cat_buffer);
		offset += read_size;
	}
	printk("\n");
	return 0;
}

} // namespace FS