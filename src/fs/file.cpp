#include <assert.h>
#include <common.h>
#include <container.hpp>
#include <fs/fs.hpp>
#include <kstdio.h>
#include <string.h>
#include <thread.hpp>
namespace FS
{

static char file_buffer[BLOCK_SIZE] __attribute__((aligned(4096)));
class File : public IdObject
{
public:
	uint inode_idx;
	uint mode;
	uint file_ptr = 0;
	Inode inode;
	File()
	{
	}
	int read(char *buff, int length)
	{
		if ((mode & O_RDONLY) == 0)
			return -1;
		int read_length = 0;
		while (length)
		{
			uint block_offset = file_ptr % BLOCK_SIZE;
			uint read_size = std::min(BLOCK_SIZE - block_offset, (size_t)length);
			inode_modify_data(false, inode, file_buffer, file_ptr, read_size);
			memcpy(buff, file_buffer, read_size); // buff is in user space
			buff += read_size;
			file_ptr += read_size;
			length -= read_size;
			read_length += read_size;
		}
		write_inode(inode_idx, inode);
		return read_length;
	}
	int write(const char *buff, int length)
	{
		if ((mode & O_WRONLY) == 0)
			return -1;
		int write_length = 0;
		while (length)
		{
			uint block_offset = file_ptr % BLOCK_SIZE;
			uint write_size = std::min(BLOCK_SIZE - block_offset, (size_t)length);
			memcpy(file_buffer, buff, write_size); // buff is in user space
			inode_modify_data(true, inode, file_buffer, file_ptr, write_size);
			buff += write_size;
			file_ptr += write_size;
			length -= write_size;
			write_length += write_size;
		}
		write_inode(inode_idx, inode);
		return write_length;
	}
	int lseek(int offset, int whence)
	{
		switch (whence)
		{
		case SEEK_SET:
			file_ptr = offset;
			break;
		case SEEK_CUR:
			file_ptr += offset;
			break;
		case SEEK_END:
			file_ptr = inode.size + offset;
			break;
		default:
			return -1;
		}
		return file_ptr;
	}

	inline static IdPool pool;
	virtual ~File()
	{
		pool.remove(this);
	}
};

int fs_fopen(const char *path, int mode)
{
	uint inode_idx = get_inode_by_filename(path, true);
	if (inode_idx < 0)
		return -1;
	Inode inode = read_inode(inode_idx);
	if (inode.type == 1)
		return -1;
	if (inode.type == 0)
	{
		inode.type = 2;
		inode.link_cnt = 1;
		write_inode(inode_idx, inode);
	}
	return File::pool.init(inode_idx, [mode, inode_idx, inode]() {
		File *file = new File();
		file->inode_idx = inode_idx;
		file->mode = mode;
		file->inode = inode;
		return file;
	});
}
int fs_fread(int fd, char *buff, int length)
{
	File *file = (File *)File::pool.get(fd);
	if (file)
		return file->read(buff, length);
	return -1;
}
int fs_fwrite(int fd, const char *buff, int length)
{
	File *file = (File *)File::pool.get(fd);
	if (file)
		return file->write(buff, length);
	return -1;
}
int fs_lseek(int fd, int offset, int whence)
{
	File *file = (File *)File::pool.get(fd);
	if (file)
		return file->lseek(offset, whence);
	return -1;
}
int fs_fclose(int fd)
{
	File::pool.close(fd);
	return 0;
}

} // namespace FS