#include <arch/bios_func.h>
#include <drivers/screen.h>
#include <kstdio.h>
#include <spinlock.hpp>
#include <string.h>
#include <thread.hpp>

#define SCREEN_WIDTH 80
#define SCREEN_HEIGHT 50
#define SCREEN_LOC(x, y) ((y) * SCREEN_WIDTH + (x))

#define current_process current_cpu->current_thread->process

/* screen buffer */
char new_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};
char old_screen[SCREEN_HEIGHT * SCREEN_WIDTH] = {0};
SpinLock screen_lock;

/* cursor position */
static void vt100_move_cursor(int x, int y)
{
	// \033[y;xH
	printv("%c[%d;%dH", 27, y, x);
}

/* clear screen */
static void vt100_clear()
{
	// \033[2J
	printv("%c[2J", 27);
}

/* hidden cursor */
static void vt100_hidden_cursor()
{
	// \033[?25l
	printv("%c[?25l", 27);
}

/* write a char */
/* write a char */
void screen_write_ch(char ch)
{
	if (ch == '\n')
	{
		current_process->cursor_x = 0;
		if (current_process->cursor_y < SCREEN_HEIGHT)
			current_process->cursor_y++;
	}
	else if (ch == '\b' || ch == '\177')
	{
		// TODO: [P3] support backspace here
		if (current_process->cursor_x > 0)
		{
			current_process->cursor_x--;
			new_screen[SCREEN_LOC(current_process->cursor_x, current_process->cursor_y)] = ' ';
		}
	}
	else
	{
		new_screen[SCREEN_LOC(current_process->cursor_x, current_process->cursor_y)] = ch;
		if (++current_process->cursor_x >= SCREEN_WIDTH)
		{
			current_process->cursor_x = 0;
			if (current_process->cursor_y < SCREEN_HEIGHT)
				current_process->cursor_y++;
		}
	}
}

void init_screen(void)
{
	vt100_hidden_cursor();
	vt100_clear();
	screen_clear();
}

void screen_clear(void)
{
	screen_lock.lock();
	int i, j;
	for (i = 0; i < SCREEN_HEIGHT; i++)
	{
		for (j = 0; j < SCREEN_WIDTH; j++)
		{
			new_screen[SCREEN_LOC(j, i)] = ' ';
		}
	}
	current_process->cursor_x = 0;
	current_process->cursor_y = 0;
	screen_lock.unlock();
	screen_reflush();
}

void screen_move_cursor(int x, int y)
{
	lock_guard guard(screen_lock);
	if (x >= SCREEN_WIDTH)
		x = SCREEN_WIDTH - 1;
	else if (x < 0)
		x = 0;
	if (y >= SCREEN_HEIGHT)
		y = SCREEN_HEIGHT - 1;
	else if (y < 0)
		y = 0;
	current_process->cursor_x = x;
	current_process->cursor_y = y;
	vt100_move_cursor(x + 1, y + 1);
}

void screen_write(const char *buff)
{
	int i = 0;
	int l = strlen(buff);

	lock_guard guard(screen_lock);
	for (i = 0; i < l; i++)
	{
		screen_write_ch(buff[i]);
	}
}

/*
 * This function is used to print the serial port when the clock
 * interrupt is triggered. However, we need to pay attention to
 * the fact that in order to speed up printing, we only refresh
 * the characters that have been modified since this time.
 */
void screen_reflush(void)
{
	lock_guard guard(screen_lock);
	int i, j;

	/* here to reflush screen buffer to serial port */
	for (i = 0; i < SCREEN_HEIGHT; i++)
	{
		for (j = 0; j < SCREEN_WIDTH; j++)
		{
			/* We only print the data of the modified location. */
			if (new_screen[SCREEN_LOC(j, i)] != old_screen[SCREEN_LOC(j, i)])
			{
				vt100_move_cursor(j + 1, i + 1);
				bios_putchar(new_screen[SCREEN_LOC(j, i)]);
				old_screen[SCREEN_LOC(j, i)] = new_screen[SCREEN_LOC(j, i)];
			}
		}
	}

	/* recover cursor position */
	vt100_move_cursor(current_process->cursor_x, current_process->cursor_y);
}
