#include <kernel.h>
#include "xjs.h"

char *str[] = {
	"cccc",
	"aaaa",
	"bbbb",
	"aaaa",
	"dddd",
	"bbbb",
	"eeee",
	"gafs",
};
int strnum = sizeof(str) / sizeof(str[0]);
int main(void)
{
	*xjs_batch_str_num = strnum;
	for (int i = 0; i < strnum; i++)
	{
		xjs_batch_strs[i] = str[i];
		bios_putstr(str[i]);
		bios_putchar('\n');
	}
	bios_putstr("xjs batch task 1 finished\n");
	return 0;
}