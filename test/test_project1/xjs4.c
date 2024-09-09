#include <kernel.h>
#include "xjs.h"

int main(void)
{
    for (int i = 0; i < *xjs_batch_str_num; i++)
    {
        bios_putchar('1' + i);
        bios_putchar(':');
        bios_putstr(xjs_batch_strs[i]);
        bios_putchar('\n');
    }
    bios_putstr("xjs batch task 4 finished\n");
    return 0;
}