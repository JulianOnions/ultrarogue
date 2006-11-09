 /*
  * This file has all the code for the option command.
  * I would rather this command were not necessary, but
  * it is the only way to keep the wolves off of my back.
  *
  */
 
#include "mach_dep.h"
#include "curses.h"
#include <ctype.h>
#include "rogue.h"
 
#define	NUM_OPTS	(sizeof optlist / sizeof (OPTION))
 
 
 /*
  * description of an option and what to do with it
  */
 struct optstruct {
     char	*o_name;	/* option name */
     char	*o_prompt;	/* prompt for interactive entry */
     int		*o_opt;		/* pointer to thing to set */
     int		(*o_putfunc)();	/* function to print value */
     int		(*o_getfunc)();	/* function to get value interactively */
 };
 
 typedef struct optstruct	OPTION;
 
 int	put_bool(), get_bool(), put_str(), get_str(), put_abil(), get_abil();
 
 OPTION	optlist[] = {
     {"terse",	 "Terse output (terse): ",
 		 (int *) &terse,	put_bool,	get_bool	},
     {"flush",	 "Flush typeahead during battle (flush): ",
 		 (int *) &fight_flush,	put_bool,	get_bool	},
     {"jump",	 "Show position only at end of run (jump): ",
 		 (int *) &jump,		put_bool,	get_bool	},
     {"step",	"Do inventories one line at a time (step): ",
 		(int *) &slow_invent,	put_bool,	get_bool	},
     {"askme",	"Ask me about unidentified things (askme): ",
 		(int *) &askme,		put_bool,	get_bool	},
     {"name",	 "Name (name): ",
 		(int *) whoami,		put_str,	get_str		},
     {"fruit",	 "Fruit (fruit): ",
 		(int *) fruit,		put_str,	get_str		},
     {"file",	 "Save file (file): ",
 		(int *) file_name,	put_str,	get_str		},
     {"score",	 "Score file (score): ",
 		(int *) score_file,	put_str,	get_str		},
     {"class",	"Character class (class): ",
 		(int *) &char_type,	put_abil,	get_abil	}
 };
 
 /*
  * print and then set options from the terminal
  */
 option()
 {
     register OPTION	*op;
     register int	retval;
 
     wclear(hw);
     touchwin(hw);
     /*
      * Display current values of options
      */
     for (op = optlist; op < &optlist[NUM_OPTS]; op++)
     {
 	waddstr(hw, op->o_prompt);
 	(*op->o_putfunc)(op->o_opt, hw);
 	waddch(hw, '\n');
     }
     /*
      * Set values
      */
     wmove(hw, 0, 0);
     for (op = optlist; op < &optlist[NUM_OPTS]; op++)
     {
 	waddstr(hw, op->o_prompt);
 	if ((retval = (*op->o_getfunc)(op->o_opt, hw)))
 	    if (retval == QUIT)
 		break;
 	    else if (op > optlist) {	/* MINUS */
 		wmove(hw, (op - optlist) - 1, 0);
 		op -= 2;
 	    }
 	    else	/* trying to back up beyond the top */
 	    {
 		putchar('\007');
 		wmove(hw, 0, 0);
 		op--;
 	    }
     }
     /*
      * Switch back to original screen
      */
     mvwaddstr(hw, LINES-1, 0, spacemsg);
     draw(hw);
     wait_for(' ');
     clearok(cw, TRUE);
     touchwin(cw);
     after = FALSE;
 }
 
 /*
  * put out a boolean
  */
 put_bool(b, win)
 bool	*b;
 WINDOW *win;
 {
     waddstr(win, *b ? "True" : "False");
 }
 
 /*
  * put out a string
  */
 put_str(str, win)
 char *str;
 WINDOW *win;
 {
     waddstr(win, str);
 }
 
 /*
  * print the character type
  */
 put_abil(ability, win)
 int *ability;
 WINDOW *win;
 {
     char *abil;
 
     switch (*ability) {
 	case C_FIGHTER:
 	    abil = "Fighter";
 	    break;
 	case C_MAGICIAN:
 	    abil = "Magic User";
 	    break;
 	case C_CLERIC:
 	    abil = "Cleric";
 	    break;
 	case C_THIEF:
 	    abil = "Thief";
 	    break;
 	default:
 	    abil = "??";
     }
     waddstr(win, abil);
 }
 
 
 /*
  * allow changing a boolean option and print it out
  */
 
 get_bool(bp, win)
 bool *bp;
 WINDOW *win;
 {
     register int oy, ox;
     register bool op_bad;
 
     op_bad = TRUE;
     getyx(win, oy, ox);
     waddstr(win, *bp ? "True" : "False");
     while(op_bad)	
     {
 	wmove(win, oy, ox);
 	draw(win);
 	switch (readchar())
 	{
 	    case 't':
 	    case 'T':
 		*bp = TRUE;
 		op_bad = FALSE;
 		break;
 	    case 'f':
 	    case 'F':
 		*bp = FALSE;
 		op_bad = FALSE;
 		break;
 	    case '\n':
 	    case '\r':
 		op_bad = FALSE;
 		break;
 	    case '\033':
 	    case '\007':
 		return QUIT;
 	    case '-':
 		return MINUS;
 	    default:
 		mvwaddstr(win, oy, ox + 10, "(T or F)");
 	}
     }
     wmove(win, oy, ox);
     wclrtoeol(win);
     waddstr(win, *bp ? "True" : "False");
     waddch(win, '\n');
     return NORM;
 }
 
 /*
  * set a string option
  */
 get_str(opt, win)
 register char *opt;
 WINDOW *win;
 {
     register char *sp;
     register int c, oy, ox;
     char buf[LINELEN];
 
     draw(win);
     getyx(win, oy, ox);
     /*
      * loop reading in the string, and put it in a temporary buffer
      */
     for (sp = buf;
 	(c = readchar()) != '\n'	&& 
 	c != '\r'			&& 
 	c != '\033'			&& 
 	c != '\007'			&&
 	sp < &buf[LINELEN-1];
 	wclrtoeol(win), draw(win))
     {
 	if (c == -1)
 	    continue;
#ifndef USGV4
 	else if (c == _tty.c_cc[VERASE]) /* process erase character */
#else
 	else if (c == _tty.sg_erase) /* process erase character */
#endif
 	{
 	    if (sp > buf)
 	    {
 		register int i;
 
 		sp--;
 		for (i = strlen(unctrl(*sp)); i; i--)
 		    waddch(win, '\b');
 	    }
 	    continue;
 	}
#ifndef USGV4
 	else if (c == _tty.c_cc[VKILL])  /* process kill character */
#else
 	else if (c == _tty.sg_kill)  /* process kill character */
#endif
 	{
 	    sp = buf;
 	    wmove(win, oy, ox);
 	    continue;
 	}
 	else if (sp == buf)
 	    if (c == '-' && win == hw)	/* To move back a line in hw */
 		break;
 	    else if (c == '~')
 	    {
 		strcpy(buf, home);
 		waddstr(win, home);
 		sp += strlen(home);
 		continue;
 	    }
 	*sp++ = c;
 	waddstr(win, unctrl(c));
     }
     *sp = '\0';
     if (sp > buf)	/* only change option if something has been typed */
 	strucpy(opt, buf, strlen(buf));
     wmove(win, oy, ox);
     waddstr(win, opt);
     waddch(win, '\n');
     draw(win);
     if (win == cw)
 	mpos += sp - buf;
     if (c == '-')
 	return MINUS;
     else if (c == '\033' || c == '\007')
 	return QUIT;
     else
 	return NORM;
 }
 
 /*
  * The ability field is read-only
  */
 get_abil(abil, win)
 int *abil;
 WINDOW *win;
 {
     register int oy, ox, ny, nx;
     register bool op_bad;
 
     op_bad = TRUE;
     getyx(win, oy, ox);
     put_abil(abil, win);
     getyx(win, ny, nx);
     while(op_bad)	
     {
 	wmove(win, oy, ox);
 	draw(win);
 	switch (readchar())
 	{
 	    case '\n':
 	    case '\r':
 		op_bad = FALSE;
 		break;
 	    case '\033':
 	    case '\007':
 		return QUIT;
 	    case '-':
 		return MINUS;
 	    default:
 		mvwaddstr(win, ny, nx + 5, "(no change allowed)");
 	}
     }
     wmove(win, ny, nx + 5);
     wclrtoeol(win);
     wmove(win, ny, nx);
     waddch(win, '\n');
     return NORM;
 }
 
 
 /*
  * parse options from string, usually taken from the environment.
  * the string is a series of comma seperated values, with booleans
  * being stated as "name" (true) or "noname" (false), and strings
  * being "name=....", with the string being defined up to a comma
  * or the end of the entire option string.
  */
 
 parse_opts(str)
 register char *str;
 {
     register char *sp;
     register OPTION *op;
     register int len;
 
     while (*str)
     {
 	/*
 	 * Get option name
 	 */
 	for (sp = str; isalpha(*sp); sp++)
 	    continue;
 	len = sp - str;
 	/*
 	 * Look it up and deal with it
 	 */
 	for (op = optlist; op < &optlist[NUM_OPTS]; op++)
 	    if (EQSTR(str, op->o_name, len))
 	    {
 		if (op->o_putfunc == put_bool)	/* if option is a boolean */
 		    *(bool *)op->o_opt = TRUE;
 		else				/* string option */
 		{
 		    register char *start;
 		    char value[80];
 
 		    /*
 		     * Skip to start of string value
 		     */
 		    for (str = sp + 1; *str == '='; str++)
 			continue;
 		    if (*str == '~')
 		    {
 			strcpy((char *) value, home);
 			start = (char *) value + strlen(home);
 			while (*++str == '/')
 			    continue;
 		    }
 		    else
 			start = (char *) value;
 		    /*
 		     * Skip to end of string value
 		     */
 		    for (sp = str + 1; *sp && *sp != ','; sp++)
 			continue;
 		    strucpy(start, str, sp - str);
 
 		    /* Put the value into the option field */
 		    if (op->o_putfunc != put_abil) 
 			strcpy(op->o_opt, value);
 
 		    else if (*op->o_opt == -1) { /* Only init ability once */
 			register int len = strlen(value);
 
 			if (isupper(value[0])) value[0] = tolower(value[0]);
 			if (EQSTR(value, "fighter", len))
 				*op->o_opt = C_FIGHTER;
 			else if (EQSTR(value, "magic", min(len, 5)))
 				*op->o_opt = C_MAGICIAN;
 			else if (EQSTR(value, "cleric", len))
 				*op->o_opt = C_CLERIC;
 			else if (EQSTR(value, "thief", len))
 				*op->o_opt = C_THIEF;
 		    }
 		}
 		break;
 	    }
 	    /*
 	     * check for "noname" for booleans
 	     */
 	    else if (op->o_putfunc == put_bool
 	      && EQSTR(str, "no", 2) && EQSTR(str + 2, op->o_name, len - 2))
 	    {
 		*(bool *)op->o_opt = FALSE;
 		break;
 	    }
 
 	/*
 	 * skip to start of next option name
 	 */
 	while (*sp && !isalpha(*sp))
 	    sp++;
 	str = sp;
     }
 }
 
 /*
  * copy string using unctrl for things
  */
 strucpy(s1, s2, len)
 register char *s1, *s2;
 register int len;
 {
     register char *sp;
 
     while (len--)
     {
 	strcpy(s1, (sp = unctrl(*s2++)));
 	s1 += strlen(sp);
     }
     *s1 = '\0';
 }
