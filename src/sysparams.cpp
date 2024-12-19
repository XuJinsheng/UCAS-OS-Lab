#include <common.h>
#include <fs/fs.hpp>
#include <string>
#include <syscall.hpp>
#include <sysparams.hpp>

int fd_param = 0;
void init_parameters()
{
	fd_param = FS::fs_fopen("sysparams", O_RDWR);
	char buf[16];
	FS::fs_fread(fd_param, buf, 16);
	long policy = 0;
	for (int i = 0; i < 16; i++)
	{
		if (buf[i] == '\0')
			break;
		if (buf[i] < '0' || buf[i] > '9')
			return;
		policy = policy * 10 + buf[i] - '0';
	}
	FS::cache_set_policy(policy);
}

ptr_t set_parameter(long key, long value)
{
	if (key != 0)
		return -1;
	switch (key)
	{
	case 0:
		FS::cache_set_policy(value);
		break;
	}
	std::string str = std::to_string(value);
	FS::fs_lseek(fd_param, 0, SEEK_SET);
	FS::fs_fwrite(fd_param, str.c_str(), str.size());
	return 0;
}