#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char buff[4096];

int main(int argc, char *argv[])
{
	long start, end;
	long test_MB = 1;
	if (argc < 2 || (test_MB = atoi(argv[1])) == 0)
	{
		printf("Invalid argument\n");
		return 0;
	}

	int fd = sys_fopen("test.dat", O_RDWR);
	if (fd == -1)
	{
		printf("Open file failed\n");
		return 0;
	}

	for (int i = 0; i < 4096; i++)
	{
		buff[i] = i;
	}

	printf("File hole test begin\n");
	printf("Test size: %ld MB\n", test_MB);

	sys_fwrite(fd, buff, 4096);
	sys_lseek(fd, 0, SEEK_SET);
	sys_fread(fd, buff, 4096);

	for (int j = 0; j < 4096; j++)
	{
		if (buff[j] != (j & 0xff))
		{
			printf("Read Error");
			sys_fclose(fd);
			return 0;
		}
	}
	sys_lseek(fd, test_MB * 1024 * 1024, SEEK_SET);
	sys_fwrite(fd, buff, 4096);
	sys_lseek(fd, test_MB * 1024 * 1024, SEEK_SET);
	sys_fread(fd, buff, 4096);

	for (int j = 0; j < 4096; j++)
	{
		if (buff[j] != (j & 0xff))
		{
			printf("Read Error");
			sys_fclose(fd);
			return 0;
		}
	}
	printf("Read OK\n");

	sys_fclose(fd);
	return 0;
}