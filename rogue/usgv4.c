#include <curses.h>
#include "rogue.h"
#include "mach_dep.h"


#ifndef	flushout
flushout()
{
	int out = 1;
#ifdef TIOCFLUSH
	ioctl(_tty_ch, TIOCFLUSH, (char *)&out);
	#endif
}
#endif
