#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char buff[4096];

int main(int argc, char *argv[])
{

	long test_MB = 1;
	if (argc > 1)
	{
		test_MB = atoi(argv[1]);
		if (test_MB <= 0)
		{
			printf("Invalid argument\n");
			return 0;
		}
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

	printf("Large file write&read test begin...\n");
	printf("Test size: %d MB\n", test_MB);

	// test write
	long start = sys_get_tick();
	for (int i = 0; i < test_MB * 256; i++)
	{
		sys_fwrite(fd, buff, 4096);
	}
	long end = sys_get_tick();
	printf("Write end, Time: %ld\n", (end - start) / sys_get_timebase());

	// test read
	start = sys_get_tick();
	for (int i = 0; i < test_MB * 256; i++)
	{
		sys_fread(fd, buff, 4096);
		for (int j = 0; j < 4096; j++)
		{
			if (buff[j] != (j & 0xff))
			{
				printf("Read Error\n");
				sys_fclose(fd);
				return 0;
			}
		}
	}
	end = sys_get_tick();
	printf("Read end, Time: %ld\n", (end - start) / sys_get_timebase());

	sys_fclose(fd);
	return 0;
}