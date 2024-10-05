#include <os/task.h>
#include <os/string.h>
#include <os/kernel.h>
#include <type.h>

uint64_t load_task_img(int taskid)
{
    /* TODO: * [p1-task3] load task from image via task id, and return its entrypoint */
    if (taskid >= task_num)
        return 0;
    uint64_t entry = tasks[taskid].entry_point;
    bios_sd_read(entry, tasks[taskid].sdcard_block_num, tasks[taskid].sdcard_block_id);
    return entry;
}

uint64_t load_task_img_by_name(const char *taskname)
{
    /* TODO: [p1-task4] load task via task name, thus the arg should be 'char *taskname' */
    for (int i = 0; i < task_num; i++)
    {
        if (strcmp(tasks[i].name, taskname) == 0)
        {
            return load_task_img(i);
        }
    }
    return 0;
}

void task_interact()
{
Label:
    bios_putstr("Task list:\n");
    for (int i = 0; i < task_num; i++)
    {
        bios_putchar('0' + i);
        bios_putstr(": ");
        bios_putstr(tasks[i].name);
        bios_putchar('\n');
    }
    bios_putstr("x: batch task\n");
    char str[32];
    bios_putstr("Please select task: ");
    int len = getline(str, 32);
    if (str[0] == 'x')
    {
        uint64_t entry = load_task_img_by_name("xjs1");
        ((void (*)(void))entry)();
        entry = load_task_img_by_name("xjs2");
        ((void (*)(void))entry)();
        entry = load_task_img_by_name("xjs3");
        ((void (*)(void))entry)();
        entry = load_task_img_by_name("xjs4");
        ((void (*)(void))entry)();
        bios_putchar('\n');
        bios_putchar('\n');
    }
    else
    {
        uint64_t entry = 0;
        if (len == 1 && str[0] >= '0' && str[0] < '0' + task_num)
            entry = load_task_img(str[0] - '0');
        else
            entry = load_task_img_by_name(str);
        if (entry == 0)
            bios_putstr("Task not found!\n");
        else
        {
            printnum(entry);
            bios_putchar(' ');
            printnum(*(int *)entry);
            bios_putchar('\n');
            ((void (*)(void))entry)();
            bios_putchar('\n');
            bios_putchar('\n');
        }
    }
    goto Label;
}