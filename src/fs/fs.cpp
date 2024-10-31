#include <assert.h>
#include <common.h>
#include <fs/fs.hpp>
#include <kstdio.h>
#include <string.h>

uint8_t buffer[BLOCK_SIZE]; // caller saved
SuperBlock superblock;
uint cwd_inode;

bool init_filesystem()
{
	SuperBlock *sb = (SuperBlock *)buffer;
	if (sb->magic0 == SUPERBLOCK_MAGIC && sb->magic1 == SUPERBLOCK_MAGIC)
	{
		superblock = *sb;
		cwd_inode = superblock.root_inode;
		return true;
	}
	else
	{
		fs_mkfs();
		cwd_inode = superblock.root_inode;
		return false;
	}
}
void flush_filesystem()
{
	SuperBlock *sb = (SuperBlock *)buffer;
	*sb = superblock;
	write_block(buffer, 0);
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
	superblock.block_free_min = superblock.block_start_block;
	superblock.magic1 = SUPERBLOCK_MAGIC;

	memset(buffer, 0, BLOCK_SIZE);
	for (uint32_t i = 0; i < superblock.inode_map_size / 8; i++)
		write_block(buffer, superblock.inode_map_start_block + i);
	for (uint32_t i = 0; i < superblock.block_map_size / 8; i++)
		write_block(buffer, superblock.block_map_start_block + i);

	superblock.root_inode = inode_alloc();
	cwd_inode = superblock.root_inode;
	flush_filesystem();

	Inode root_inode = {.type = 1, .link_cnt = 65535};
	DirEntry dir[2] = {DirEntry{cwd_inode, 1, "."}, DirEntry{cwd_inode, 1, ".."}};
	inode_modify_data(true, root_inode, dir, 0, sizeof(dir));
	write_inode(superblock.root_inode, Inode{});
	return 0;
}
int fs_statfs()
{
	printk("[statfs]:");
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
		read_block(buffer, superblock.inode_map_start_block + i);
		for (uint j = 0; j < BLOCK_SIZE; j++)
		{
			if (buffer[j] != 0xff)
			{
				for (uint k = 0; k < 8; k++)
				{
					if ((buffer[j] & (1 << k)) == 0)
					{
						buffer[j] |= 1 << k;
						write_block(buffer, superblock.inode_map_start_block + i);
						int inode = i * BLOCK_SIZE * 8 + j * 8 + k;
						superblock.inode_allocated++;
						superblock.inode_free_min = inode + 1;
						flush_filesystem();
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
	read_block(buffer, superblock.inode_map_start_block + i);
	assert(buffer[j] & (1 << k));
	buffer[j] &= ~(1 << k);
	write_block(buffer, superblock.inode_map_start_block + i);
	superblock.inode_allocated--;
	if (ino < superblock.inode_free_min)
		superblock.inode_free_min = ino;
	flush_filesystem();
}

int block_alloc()
{
	for (uint i = superblock.block_free_min / (BLOCK_SIZE * 8); i < superblock.block_map_size / (BLOCK_SIZE * 8); i++)
	{
		read_block(buffer, superblock.block_map_start_block + i);
		for (uint j = 0; j < BLOCK_SIZE; j++)
		{
			if (buffer[j] != 0xff)
			{
				for (uint k = 0; k < 8; k++)
				{
					if ((buffer[j] & (1 << k)) == 0)
					{
						buffer[j] |= 1 << k;
						write_block(buffer, superblock.block_map_start_block + i);
						int blockid = i * BLOCK_SIZE * 8 + j * 8 + k;
						superblock.block_allocated++;
						superblock.block_free_min = blockid + 1;
						flush_filesystem();
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
	read_block(buffer, superblock.block_map_start_block + i);
	assert(buffer[j] & (1 << k));
	buffer[j] &= ~(1 << k);
	write_block(buffer, superblock.block_map_start_block + i);
	superblock.block_allocated--;
	if (blockid < superblock.block_free_min)
		superblock.block_free_min = blockid;
	flush_filesystem();
}

Inode read_inode(uint ino)
{
	int i = superblock.inode_start_block + ino / INODE_PER_BLOCK;
	read_block(buffer, i);
	return ((Inode *)buffer)[ino % INODE_PER_BLOCK];
}

void write_inode(uint ino, const Inode &inode)
{
	int i = superblock.inode_start_block + ino / INODE_PER_BLOCK;
	read_block(buffer, i);
	((Inode *)buffer)[ino % INODE_PER_BLOCK] = inode;
	write_block(buffer, i);
}

constexpr size_t RATIO_PER_LEVEL = BLOCK_SIZE / sizeof(uint);
constexpr size_t L0_SIZE = BLOCK_SIZE, L1_SIZE = L0_SIZE * RATIO_PER_LEVEL, L2_SIZE = L1_SIZE * RATIO_PER_LEVEL,
				 L3_SIZE = L2_SIZE * RATIO_PER_LEVEL;
constexpr size_t L0_BOUND = INODE_DATA_DIRECT * BLOCK_SIZE, L1_BOUND = L0_BOUND + INODE_DATA_L1 * L1_SIZE,
				 L2_BOUND = L1_BOUND + INODE_DATA_L2 * L2_SIZE, L3_BOUND = L2_BOUND + INODE_DATA_L3 * L3_SIZE;
bool inode_modify_data_recur(bool is_write, uint &blockid, void *data, uint offset, uint length, uint depth)
{
	bool modified;
	if (blockid == 0)
	{
		blockid = block_alloc();
		modified = true;
		memset(buffer, 0, BLOCK_SIZE);
		write_block(buffer, blockid);
	}
	else
	{
		read_block(buffer, blockid);
		modified = false;
	}
	if (depth == 0)
	{
		if (is_write)
		{
			memcpy(buffer + offset, data, length);
			write_block(buffer, blockid);
		}
		else
			memcpy(data, buffer + offset, length);
	}
	else
	{
		uint *data = (uint *)buffer;
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
			write_block(buffer, blockid);
	}
	return modified;
}
bool inode_modify_data(bool is_write, Inode &inode, void *data, uint offset, uint length)
{
	assert(offset % BLOCK_SIZE + length <= BLOCK_SIZE); // no cross block
	// L3 is actually not used
	bool inode_modified = false;
	if (offset + length > inode.size)
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

int get_inode_by_filename(const char *path, bool create_if_not_existed)
{
	Inode cwd = read_inode(cwd_inode);
	uint free_offset = 0;
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (cwd.data_direct[i])
		{
			DirEntry *dir = (DirEntry *)buffer;
			read_block(buffer, cwd.data_direct[i]);
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
	assert(free_offset != 0); // no space for new file
	uint new_inode = inode_alloc();
	write_inode(new_inode, Inode{});
	DirEntry dir = {new_inode, 1};
	strcpy(dir.name, path);
	if (inode_modify_data(true, cwd, &dir, free_offset * sizeof(DirEntry), sizeof(DirEntry)))
	{
		write_inode(cwd_inode, cwd);
	}
	return new_inode;
}

int fs_cd(const char *path)
{
	char tmp[64];
	uint old_cwd = cwd_inode;
	const char *p = path;
	while (*p)
	{
		while (*p != '/' && *p)
			p++;
		memcpy(tmp, path, p - path);
		tmp[p - path] = 0;
		cwd_inode = get_inode_by_filename(tmp, false);
		if (cwd_inode < 0)
		{
			cwd_inode = old_cwd;
			printk("No such directory\n");
			return -1;
		}
		Inode inode = read_inode(cwd_inode);
		if (inode.type != 1)
		{
			cwd_inode = old_cwd;
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
	uint old_cwd = cwd_inode;
	if (fs_cd(path) == -1)
	{
		printk("No such directory\n");
		cwd_inode = old_cwd;
		return -1;
	}
	Inode inode = read_inode(cwd_inode);
	if (inode.type != 1)
	{
		printk("Not a directory\n");
		cwd_inode = old_cwd;
		return -1;
	}
	for (int i = 0; i < INODE_DATA_DIRECT; i++)
		if (inode.data_direct[i])
		{
			DirEntry *dir = (DirEntry *)buffer;
			read_block(buffer, inode.data_direct[i]);
			for (int j = 0; j < DIRENTRY_PER_BLOCK; j++)
			{
				if (dir[j].valid)
				{
					printk("%s ", dir[j].name);
				}
			}
		}
	printk("\n");
	cwd_inode = old_cwd;
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
	DirEntry dir[2] = {DirEntry{(uint)inode_idx, 1, "."}, DirEntry{cwd_inode, 1, ".."}};
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
	uint *data = (uint *)buffer;
	read_block(buffer, blockid);
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
	inode_free(ino);
}
/* int fs_rmdir(const char *path){
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

} */
