#include <kernel.h>
#include "xjs.h"
int strcmp(const char *str1, const char *str2)
{
    while (*str1 && *str2)
    {
        if (*str1 != *str2)
        {
            return (*str1) - (*str2);
        }
        ++str1;
        ++str2;
    }
    return (*str1) - (*str2);
}
int unique(char *arr[], int length)
{
    if (length == 0)
        return 0;

    int writeIndex = 1;
    for (int readIndex = 1; readIndex < length; readIndex++)
    {
        if (strcmp(arr[readIndex], arr[writeIndex - 1]))
        {
            arr[writeIndex] = arr[readIndex];
            writeIndex++;
        }
    }

    return writeIndex;
}
int main(void)
{
    *xjs_batch_str_num = unique(xjs_batch_strs, *xjs_batch_str_num);
    bios_putstr("xjs batch task 3 finished\n");
    return 0;
}