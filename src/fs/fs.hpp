#pragma once

#include <arch/bios_func.h>
#include <common.h>

typedef uint32_t uint;
constexpr size_t KiB = 1024;
constexpr size_t MiB = 1024 * KiB;
constexpr size_t GiB = 1024 * MiB;
constexpr size_t BLOCK_SIZE = 4096;
constexpr size_t BLOCK_START = 512 * MiB; // 512MB
constexpr size_t BLOCK_END = 1024 * MiB;  // 1GB
constexpr uint32_t SUPERBLOCK_MAGIC = 0x20221205;
constexpr size_t MAX_BLOCK_NUM = (BLOCK_END - BLOCK_START) / BLOCK_SIZE;
constexpr size_t MAX_INODE_NUM = 32 * KiB;

struct SuperBlock // start is counted in block
{
	uint32_t magic0;
	uint32_t fs_start;
	uint32_t inode_map_start_block;
	uint32_t inode_map_size;
	uint32_t inode_allocated;
	uint32_t inode_free_min;
	uint32_t block_map_start_block;
	uint32_t block_map_size;
	uint32_t block_allocated;
	uint32_t block_free_min;
	uint32_t inode_start_block;
	uint32_t block_start_block;
	uint32_t root_inode;
	uint32_t magic1;
};

constexpr size_t INODE_DATA_DIRECT = 8;
constexpr size_t INODE_DATA_L1 = 3;
constexpr size_t INODE_DATA_L2 = 2;
constexpr size_t INODE_DATA_L3 = 1;
struct Inode
{
	uint8_t type; // 0: empty, 1: dir, 2: file
	uint8_t mode;
	uint16_t link_cnt;
	uint size;
	uint data_direct[INODE_DATA_DIRECT];
	uint data_l1[INODE_DATA_L1];
	uint data_l2[INODE_DATA_L2];
	uint data_l3;
};
static_assert(BLOCK_SIZE % sizeof(Inode) == 0);
constexpr size_t INODE_PER_BLOCK = BLOCK_SIZE / sizeof(Inode);

struct DirEntry
{
	uint inode;
	uint valid;
	char name[24];
};
static_assert(BLOCK_SIZE % sizeof(DirEntry) == 0);
constexpr size_t DIRENTRY_PER_BLOCK = BLOCK_SIZE / sizeof(DirEntry);

/* modes of do_fopen */
#define O_RDONLY 1 /* read only open */
#define O_WRONLY 2 /* write only open */
#define O_RDWR 3   /* read/write open */

/* whence of do_lseek */
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

extern bool init_filesystem();
extern void flush_filesystem();

extern void read_block(void *dest, uint32_t blockid, uint32_t blocks = 1); // do not use user space pointer
extern void write_block(void *src, uint32_t blockid, uint32_t blocks = 1); // do not use user space pointer

extern int inode_alloc();
extern void inode_free(uint ino);
extern int block_alloc();
extern void block_free(uint blockid);
// inode is rw by function, block is rw directly
extern Inode read_inode(uint ino);
extern void write_inode(uint ino, const Inode &inode);
// do not exceed one page, will alloc new block if needed
extern bool inode_modify_data(bool is_write, Inode &inode, void *data, uint offset, uint length);
extern int get_inode_by_filename(const char *path, bool create_if_not_existed, uint new_inode_idx = 0);

extern int fs_mkfs(void);
extern int fs_statfs(void);
extern int fs_cd(const char *path);
extern int fs_ls(const char *path, int option);
extern int fs_mkdir(const char *path);
extern int fs_touch(const char *path);
extern int fs_rmdir(const char *path);
extern int fs_rm(const char *path);
extern int fs_ln(const char *src_path, const char *dst_path);
extern int fs_cat(const char *path);
extern int fs_fopen(const char *path, int mode);
extern int fs_fread(int fd, char *buff, int length);
extern int fs_fwrite(int fd, const char *buff, int length);
extern int fs_fclose(int fd);
extern int fs_lseek(int fd, int offset, int whence);

extern uint8_t buffer[BLOCK_SIZE]; // caller saved
extern SuperBlock superblock;
extern uint cwd_inode;
