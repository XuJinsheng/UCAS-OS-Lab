/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode.
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this
 * software and associated documentation files (the "Software"), to deal in the Software
 * without restriction, including without limitation the rights to use, copy, modify,
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SHELL_BEGIN 10
char buffer[100];
char *argv[20];
int spilt()
{
	int i = 0;
	char *p = buffer;
	while (*p)
	{
		while (isspace(*p))
			++p;
		if (*p)
			argv[i++] = p;
		while (*p && !isspace(*p))
			++p;
		if (*p)
			*p++ = 0;
	}
	return i;
}
char cwd[256] = "";
int cwd_idx = 0;
void truncate_cwd()
{
	cwd_idx = strlen(cwd);
	cwd[cwd_idx] = '/';
	cwd[cwd_idx + 1] = 0;
	int i = 0;
	for (int j = 0; cwd[j]; j++)
	{
		if (j > 0 && cwd[j] == '/')
		{
			int dash = 0;
			if (cwd[j - 1] == '.')
			{
				dash++;
				if (cwd[j - 2] == '.')
					dash++;
			}
			while (dash)
			{
				i--;
				if (i < 0)
				{
					i = 0;
					break;
				}
				dash -= cwd[i] == '/';
			}
		}
		cwd[i++] = cwd[j];
	}
	i--;
	cwd[i] = 0;
	cwd_idx = i;
}
int main(void)
{
	sys_move_cursor(0, SHELL_BEGIN);
	printf("------------------- COMMAND -------------------\n");

	while (1)
	{
		printf("> root@UCAS_OS:%s$ ", cwd_idx == 0 ? "/" : cwd);
		// call syscall to read UART port
		int buffer_index = 0;
		for (char ch = sys_getchar(); ch != '\n' && ch != '\r'; ch = sys_getchar())
		{
			if (ch == '\b' || ch == 127)
			{
				if (buffer_index > 0)
				{
					buffer_index--;
					printf("%c", ch);
				}
			}
			else
			{
				buffer[buffer_index++] = ch;
				printf("%c", ch);
			}
		}
		sys_write("\n");
		buffer[buffer_index] = '\0';
		// ps, exec, kill, clear
		int argc = spilt();
		if (strcmp(argv[0], "ps") == 0)
		{
			int process = 1;
			int killed = 0;
			for (int i = 1; i < argc; i++)
			{
				if (strcmp(argv[i], "-t") == 0)
					process = 0;
				else if (strcmp(argv[i], "-k") == 0)
					killed = 1;
			}
			sys_ps(process, killed);
		}
		else if (strcmp(argv[0], "exec") == 0)
		{
			if (argc < 2)
				printf("exec: lack of arguments\n");
			else
			{
				int pid = sys_exec(argv[1], argc - 1, argv + 1);
				if (pid == 0)
					printf("exec: command not found\n");
				else
				{
					if (argv[argc - 1][0] != '&')
						sys_waitpid(pid);
				}
			}
		}
		else if (strcmp(argv[0], "taskset") == 0)
		{
			if (strcmp(argv[1], "-p") == 0)
			{
				if (argc < 4)
				{
					printf("taskset -p: lack of arguments\n");
				}
				else
				{
					long mask = atoi(argv[2]);
					int pid = atoi(argv[3]);
					sys_task_set(pid, mask);
				}
			}
			else
			{
				if (argc < 3)
				{
					printf("taskset: lack of arguments\n");
				}
				else
				{
					long mask = atoi(argv[1]);
					if (mask == 0)
					{
						printf("taskset: mask should not be zero\n");
					}
					else
					{
						sys_task_set(0, mask);
						int pid = sys_exec(argv[2], argc - 2, argv + 2);
						if (pid == 0)
							printf("exec: command not found\n");
						sys_task_set(0, -1);
					}
				}
			}
		}
		else if (strcmp(argv[0], "kill") == 0)
		{
			if (argc < 2)
				printf("kill: lack of arguments\n");
			else
			{
				int pid = atoi(argv[1]);
				if (sys_kill(pid) == 0)
					printf("kill: process failed\n");
				else
					printf("kill: process %d has been killed\n", pid);
			}
		}
		else if (strcmp(argv[0], "waitpid") == 0)
		{
			if (argc < 2)
				printf("waitpid: lack of arguments\n");
			else
			{
				int pid = atoi(argv[1]);
				if (sys_waitpid(pid) == 0)
					printf("waitpid: process failed\n");
				else
					printf("waitpid: process %d end\n", pid);
			}
		}
		else if (strcmp(argv[0], "clear") == 0)
		{
			sys_clear();
			sys_move_cursor(0, SHELL_BEGIN);
			printf("------------------- COMMAND -------------------\n");
		}
		else if (strcmp(argv[0], "mkfs") == 0)
		{
			sys_mkfs();
			cwd_idx = 0;
		}
		else if (strcmp(argv[0], "statfs") == 0)
		{
			sys_statfs();
		}
		else if (strcmp(argv[0], "cd") == 0)
		{
			if (argc < 2)
				printf("cd: lack of arguments\n");
			else if (sys_cd(argv[1]) == 0)
			{
				cwd[cwd_idx++] = '/';
				strcpy(cwd + cwd_idx, argv[1]);
				truncate_cwd();
			}
		}
		else if (strcmp(argv[0], "ls") == 0)
		{
			const char *path = ".";
			int option = 0;
			for (int i = 1; i < argc; i++)
				if (argv[i][0] != '-')
					path = argv[i];
				else
				{
					switch (argv[i][1])
					{
					case 'l':
						option |= LS_L;
						break;
					default:
						printf("ls: unknown argument %s", argv[i]);
					}
				}
			sys_ls(path, option);
		}
		else if (strcmp(argv[0], "cat") == 0)
		{
			if (argc < 2)
				printf("cat: lack of arguments\n");
			else
				sys_cat(argv[1]);
		}
		else if (strcmp(argv[0], "mkdir") == 0)
		{
			if (argc < 2)
				printf("mkdir: lack of arguments\n");
			else
				sys_mkdir(argv[1]);
		}
		else if (strcmp(argv[0], "rmdir") == 0)
		{
			if (argc < 2)
				printf("rmdir: lack of arguments\n");
			else
				sys_rmdir(argv[1]);
		}
		else if (strcmp(argv[0], "touch") == 0)
		{
			if (argc < 2)
				printf("touch: lack of arguments\n");
			else
				sys_touch(argv[1]);
		}
		else if (strcmp(argv[0], "rm") == 0)
		{
			if (argc < 2)
				printf("rm: lack of arguments\n");
			else
				sys_rm(argv[1]);
		}
		// ln
		else if (strcmp(argv[0], "ln") == 0)
		{
			if (argc < 3)
				printf("ln: lack of arguments\n");
			else
				sys_ln(argv[1], argv[2]);
		}
		else if (strcmp(argv[0], "echo") == 0)
		{
			if (argc < 2)
				printf("echo: lack of arguments\n");
			else
				printf("%s\n", argv[1]);
		}
		else
		{
			printf("Unknown command: [%s]\n", argv[0]);
		}
	}
	return 0;
}