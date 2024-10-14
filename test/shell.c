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
int main(void)
{
	sys_move_cursor(0, SHELL_BEGIN);
	printf("------------------- COMMAND -------------------\n");

	while (1)
	{
		printf("> root@UCAS_OS: ");
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
			sys_ps();
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
		else
		{
			printf("Unknown command: [%s]\n", argv[0]);
		}
		/************************************************************/
		/* Do not touch this comment. Reserved for future projects. */
		/************************************************************/
	}

	return 0;
}
