#include <curses.h>
#include "rogue.h"
#include "mach_dep.h"

#ifndef	flushout
flushout()
{
	int out = 1;

	ioctl(_tty_ch, TIOCFLUSH, (char *)&out);
}
#endif
