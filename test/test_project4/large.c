#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

const int test_size = 1024 * 1024 * 512 / 4; // 512MB
int main(int argc, char *argv[])
{
	int print_location = 1;
	if (argc > 1)
	{
		print_location = atol(argv[1]);
	}

	int *test_begin = (int *)malloc(test_size * sizeof(int));
	int *p = test_begin;
	for (int i = 0; i < test_size; i++)
	{
		*p = i;
		p++;
		if (i % (1024 * 1024) == 0)
		{
			sys_move_cursor(0, print_location);
			printf("large: filled %dMB\n", i * 4 / (1024 * 1024));
		}
	}
	sys_move_cursor(0, print_location);
	printf("large: fill finished\n");
	p = test_begin;
	for (int i = 0; i < test_size; i++)
	{
		if (*p != i)
		{
			printf("large: Test failed!\n");
			return 0;
		}
		p++;
		if (i % (1024 * 1024) == 0)
		{
			sys_move_cursor(0, print_location + 1);
			printf("large: checked %dMB\n", i * 4 / (1024 * 1024));
		}
	}
	printf("large: Test passed!\n");
	return 0;
}