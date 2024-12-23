#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char buff[4096];

int main(int argc, char *argv[])
{
	long start, end;
	long test_KB = 1, loop_count = 0;
	if (argc < 3 || (test_KB = atoi(argv[1])) == 0 || (loop_count = atoi(argv[2])) == 0)
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

	printf("Large file write&read test begin...\n");
	printf("Test size: %ld KB, loop count: %ld\n", test_KB, loop_count);

	long total_start, total_end;
	total_start = sys_get_tick();
	for (long loop = 0; loop < loop_count; loop++)
	{
		sys_move_cursor(0, 2);
		printf("Loop %ld\n", loop);

		// test write
		sys_lseek(fd, 0, SEEK_SET);
		start = sys_get_tick();
		for (int i = 0; i < test_KB / 4; i++)
		{
			sys_fwrite(fd, buff, 4096);
		}
		end = sys_get_tick();
		printf("Write end, Time: %ld\n", (end - start) / sys_get_timebase());

		// test read
		sys_lseek(fd, 0, SEEK_SET);
		start = sys_get_tick();
		for (int i = 0; i < test_KB / 4; i++)
		{
			sys_fread(fd, buff, 4096);
			for (int j = 0; j < 4096; j++)
			{
				if (buff[j] != (j & 0xff))
				{
					printf("Read Error at loop %ld, position at %d bytes\n", loop, i * 4096 + j);
					sys_fclose(fd);
					return 0;
				}
			}
		}
		end = sys_get_tick();
		printf("Read end, Time: %ld\n", (end - start) / sys_get_timebase());
	}
	total_end = sys_get_tick();
	printf("Total Time: %ld\n", (total_end - total_start) / sys_get_timebase());

	sys_fclose(fd);
	return 0;
}