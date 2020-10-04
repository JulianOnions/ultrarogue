 /*
 * Rogue
 * Exploring the dungeons of doom
 * Copyright (C) 1980 by Michael Toy and Glenn Wichman
 * All rights reserved
 *
 */

#include <stdio.h>
#include <curses.h>
#include <signal.h>
#include <pwd.h>
#include <sys/time.h>
#include "mach_dep.h"
#include "rogue.h"
#include <time.h>

static int num_checks;		/* times we've gone over in checkout() */
static double avec[3];		/* load average vector */

int fd_score = -1;		/* file descriptor the score file */

main(argc, argv, envp)
char **argv;
char **envp;
{
    register char *env;
    register struct passwd *pw;
    register struct linked_list *item;
    register struct object *obj;
    struct passwd *getpwuid();
    char *getpass(), *crypt();
    int quit(), lowtime, wpt, i, j, hpadd, dmadd;
    bool alldone, predef;
    long now;

    areuok();				/***another void here ***/
    /*
     * get home and options from environment
     */

    if ((env = getenv("HOME")) != NULL)
	strcpy(home, env);
    else if ((pw = getpwuid(getuid())) != NULL)
	strcpy(home, pw->pw_dir);
    else
	home[0] = '\0';
    strcat(home, "/");

    /* Get default save file */
    strcpy(file_name, home);
    strcat(file_name, "rogue.save");

    /* Get default score file */
    strcpy(score_file, SCOREDIR);

    /*
     * Grab a file descriptor to the score file before the
     * effective uid and gid are reset to the real ones.
     */
    fd_score = open(score_file, 2);

    if ((env = getenv("SROGUEOPTS")) != NULL)
	parse_opts(env);
    if (env == NULL || whoami[0] == '\0')
	if ((pw = getpwuid(getuid())) == NULL)
	{
	    printf("Say, who the hell are you?\n");
	    printf("You can't run without a name.\n");
	    sleep(4);
	    exit(1);
	}
	else
	    strucpy(whoami, pw->pw_name, strlen(pw->pw_name));
    if (env == NULL || fruit[0] == '\0') {
	static char *funfruit[] = {
		"candleberry", "caprifig", "dewberry", "elderberry",
		"gooseberry", "guanabana", "hagberry", "ilama", "imbu",
		"jaboticaba", "jujube", "litchi", "mombin", "pitanga",
		"prickly pear", "rambutan", "sapodilla", "soursop",
		"sweetsop", "whortleberry"
	};

	srand48((long) (getpid()+255));
	strcpy(fruit, funfruit[rnd(sizeof(funfruit)/sizeof(funfruit[0]))]);
     }

     /* put a copy of fruit in the right place */
     fd_data[1].mi_name = strdup(fruit);

    /*
     * check for print-score option
     */
    if (argc == 2 && strcmp(argv[1], "-s") == 0)
    {
	waswizard = TRUE;
	score(0, SCOREIT, 0);
	exit(0);
    }
    /*
     * Check to see if he is a wizard
     */
    areuok(PERMOK);
    makesure();
    if (argc >= 2 && argv[1][0] == '\0')
	if (strcmp(PASSWD, crypt(getpass("Wizard's password: "), PASSWDSEED)) == 0)
	{
		if (canwizard)
		{
		    wizard = TRUE;
		    argv++;
		    argc--;
		}
		else
			printf("Well that's odd...\n");
	}

    if( !author() && !wizard && !holiday())
	{
	static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	srand48((long) (getpid()+rnd(127)));
	printf("Sorry, %s, but you can't play during working hours.\n", whoami);
	printf("Meanwhile, why not enjoy a %s %s?\n",niceties[rnd(4)], fruit);
	sleep(4);
	exit(1);
	}
    if (too_much() && !wizard)
    {
	if(!author())
	{
	    static char *niceties[] = { "juicy", "fresh", "nice" ,"sweet" };

	    srand48((long) (getpid()+rnd(127)));
	    printf("Sorry, %s, but the system is too loaded now.\n", whoami);
	    printf("Try again later.  Meanwhile, why not enjoy a %s %s?\n",
	        niceties[rnd(4)], fruit);
	    sleep(4);
	    exit(1);
	}
	else
	{
		printf("The system load is rather heavy, %s...\n",whoami);
		printf("However I'll let you play anyway!\n");
		printf("Please note that there are %d users!\n",ucount());
		sleep(2);
	}
    }

    if (argc == 2 && argv[1][0] != '\0') {
	if (!restore(argv[1], envp))/* Note: restore returns on error only */
	    exit(1);
    }
    lowtime = (int) time(&now);
    dnum = (wizard && getenv("SEED") != NULL ?
	atoi(getenv("SEED")) :
	lowtime + getpid());
    if (wizard)
	printf("Hello %s, welcome to dungeon #%d", whoami, dnum);
    else
	printf("Hello %s, just a moment while I dig the dungeon...", whoami);
    fflush(stdout);
    seed = dnum;
    srand48(seed);

    init_things();			/* Set up probabilities of things */
    init_fd();				/* Set up food probabilities */
    init_colors();			/* Set up colors of potions */
    init_stones();			/* Set up stone settings of rings */
    init_materials();			/* Set up materials of wands */
    initscr();				/* Start up cursor package */
    init_names();			/* Set up names of scrolls */
    setup();

    /*
     * Reset the effective uid & gid to the real ones.
     */
    seteuid(getuid());
    setegid(getgid());

    /*
     * Set up windows
     */
    cw = newwin(LINES, COLS, 0, 0);
    mw = newwin(LINES, COLS, 0, 0);
    hw = newwin(LINES, COLS, 0, 0);

    predef = geta_player();
    waswizard = wizard;
re_roll:
    if(!predef)
	init_player();			/* Roll up the rogue */
    else
	goto get_food;			/* Using a pre-rolled rogue */
    /*
     * Give the rogue his weaponry.  
     */
    alldone = FALSE;
    do {
	int ch;
	i = rnd(16);	/* number of acceptable weapons */
	switch(i) {
	    case 0: ch = 25; wpt = MACE;
	    when 1: ch = 25; wpt = SWORD;
	    when 2: ch = 15; wpt = TWOSWORD;
	    when 3: ch = 10; wpt = SPEAR;
	    when 4: ch = 20; wpt = TRIDENT;
	    when 5: ch = 20; wpt = SPETUM;
	    when 6: ch = 20; wpt = BARDICHE;
	    when 7: ch = 15; wpt = SPIKE;
	    when 8: ch = 15; wpt = BASWORD;
	    when 9: ch = 20; wpt = HALBERD;
	    when 10:ch = 20; wpt = BATTLEAXE;
	    when 11:ch = 20; wpt = GLAIVE;
	    when 12:ch = 20; wpt = LPIKE;
	    when 13:ch = 20; wpt = BRSWORD;
	    when 14:ch = 20; wpt = CRYSKNIFE;
	    when 15:ch = 20; wpt = CLAYMORE;
	}
	if(rnd(100) < ch) {		/* create this weapon */
	    alldone = TRUE;
	}
    } while(!alldone);
    hpadd = rnd(2) + 1;
    dmadd = rnd(2) + 1;
    if (player.t_ctype == C_FIGHTER) {
	if (rnd(100) > 50)
	    wpt = TWOSWORD;
	else
	    wpt = CLAYMORE;
	hpadd = hpadd - 1;
    }
    /*
     * Find out what the armor is.
     */
    i = rnd(100) + 1;
    j = 0;
    while (armors[j].a_prob < i)
	j++;
    /*
     * See if this rogue is acceptable to the player.
     */
    if(!puta_player(j,wpt,hpadd,dmadd))
 	goto re_roll;
    /*
     * It's OK. Add this stuff to the rogue's pack.
     */
    item = spec_item(WEAPON, wpt, hpadd, dmadd);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    add_pack(item, TRUE);
    cur_weapon = obj;
    /*
     * And his suit of armor
     */
    item = spec_item(ARMOR, j, 0, 0);
    obj = (struct object *) ldata(item);
    obj->o_flags |= ISKNOW;
    obj->o_weight = armors[j].a_wght;
    add_pack(item, TRUE);
    cur_armor = obj;
    /*
     * Give him some food too
     */
get_food:
    item = spec_item(FOOD, 0, 0, 0);
    obj = OBJPTR(item);
    obj->o_weight = things[TYP_FOOD].mi_wght;
    add_pack(item, TRUE);
    new_level(FALSE);			/* Draw current level */
    /*
     * Start up daemons and fuses
     */
    daemon(doctor, &player, AFTER);
    fuse(swander, 0, WANDERTIME, AFTER);
    daemon(stomach, 0, AFTER);
    daemon(runners, 0, AFTER);
    playit();
}

/*
 * endit:
 *	Exit the program abnormally.
 */

endit()
{
    fatal("Ok, if you want to exit that badly, I'll have to allow it\n");
}

/*
 * fatal:
 *	Exit the program, printing a message.
 */

fatal(s)
char *s;
{
    clear();
    move(LINES-2, 0);
    printw("%s", s);
    draw(stdscr);
    endwin();
    printf("\n");	/* So the cursor doesn't stop at the end of the line */
    exit(0);
}

/*
 * rnd:
 *	Pick a very random number.
 */

rnd(range)
register int range;
{
	return (range == 0 ? 0 : (lrand48() & 0x7fffffff) % range);
}

/*
 * roll:
 *	roll a number of dice
 */

roll(number, sides)
register int number, sides;
{
    register int dtotal = 0;

    while(number--)
	dtotal += rnd(sides)+1;
    return dtotal;
}


/*
 * handle stop and start signals
tstp()
{
    mvcur(0, COLS - 1, LINES - 1, 0);
    endwin();
    fflush(stdout);
    signal(SIGTSTP, SIG_IGN);
    kill(0, SIGTSTP);
    signal(SIGTSTP, tstp);
    crmode();
    noecho();
    clearok(curscr, TRUE);
    touchwin(cw);
    draw(cw);
    flushout();
}  */

setup()
{
    int  checkout();
//    int  tstp();

    areuok();      /**void agin **/
    signal(SIGHUP, auto_save);
    signal(SIGINT, quit);
//    signal(SIGTSTP, tstp);

    if (!author())
    {
	signal(SIGALRM, checkout);
	alarm(CHECKTIME * 60);
	num_checks = 0;
    }
    crmode();				/* Cbreak mode */
    noecho();				/* Echo off */
}

/*
 * playit:
 *	The main loop of the program.  Loop until the game is over,
 * refreshing things and looking at the proper times.
 */

playit()
{
    register char *opts;

    /*
     * set up defaults for slow terminals
     */

#ifndef USGV4
	if((_tty.c_cflag & CBAUD) < B1200)
#else
	if(baudrate() < 1200)
#endif
    {
	terse = TRUE;
	jump = TRUE;
    }

    char_type = player.t_ctype;

    player.t_oldpos = hero;
    oldrp = roomin(&hero);
    after = TRUE;
    while (playing)
	command();			/* Command execution */
    endit();
}

/*
 * see if the system is being used too much for this game
 */
too_much()
{
	return 0;
	loadav(avec);
	return (avec[2] > (MAXLOAD / 10.0)) || (ucount() > MAXUSERS);
}

/*
 * author:
 *	See if a user is an author of the program
 *	or a user with the author's permission.
 */
author()
{
	int	id;
	int	i;

	id = getuid();

	for(i=0;i<MAXAUTH;i++)
		if(id == auth_or[i])
			return TRUE;
	return FALSE;
}

makesure()
{

	int	fd,i;

	canwizard = FALSE;
	if ((fd = open(PASSCTL, 0)) < 0) return (FALSE);

	for ( i=0; i<20;i++)
		rpass.rp_pass[i] = 0;
	for ( i=0; i<2;i++)
		rpass.rp_pkey[i] = 0;

	encread(rpass.rp_pkey, sizeof(rpass.rp_pkey), fd);
	encread(rpass.rp_pass, sizeof(rpass.rp_pass), fd);

	if((*(rpass.rp_pkey) != 0) && (*(rpass.rp_pass) != 0))
	    canwizard = TRUE;
	return(FALSE);
}
/**void**/
areuok(file)
char	*
file;
{
	register int	fd;
	char	idbuf[20];
	int	i;

	if((fd = open(file,0)) < 0)
		return ;

	encread(auth_or, sizeof(auth_or), fd);
}

checkout()
{
	static char *msgs[] = {
	"The system is too loaded for games. Please leave in %d minutes.",
	"Please save your game.  You have %d minutes.",
	"This is your last chance. You had better leave in %d minutes.",
	};
	int checktime;

	signal(SIGALRM, checkout);
	if (too_much() || !holiday()) {
	    if (num_checks >= 3) {
		register FILE *savef;
		FILE *fopen();

		clear();
		move(LINES-2, 0);
		printw("You didn't listen, so now you are DEAD!!\n");
		draw(stdscr);
		endwin();
		printf("\n");	/* now do a save if possible */
		if (file_name[0] != '\0' 
			&& (savef = fopen(file_name, "w")) != NULL)
		    save_file(savef);
		exit(1);
	    }
	    checktime = CHECKTIME / (2 * (num_checks + 1));
	    chmsg(msgs[num_checks++], checktime);
	    alarm(checktime * 60);
	}
	else {
	    if (num_checks) {
		chmsg("The load has dropped. You have a reprieve.");
		num_checks = 0;
	    }
	    alarm(CHECKTIME * 60);
	}
}

/*
 * checkout()'s version of msg.  If we are in the middle of a shell, do a
 * printf instead of a msg to avoid the refresh.
 */
chmsg(fmt, arg)
char *fmt;
int arg;
{
	if (in_shell) {
		printf(fmt, arg);
		putchar('\n');
		fflush(stdout);
	}
	else
		msg(fmt, arg);
}

/*
 * loadav - find current load average
 */
loadav(avg)
reg double *avg;
{
	FILE * pp;
	FILE * popen ();
	pp = popen (LOADAV_PROG, "r");
	fscanf (pp, "%lf%lf%lf", &avg[0], &avg[1], &avg[2]);
	pclose (pp);
	return;
}

/*
 * ucount:
 *	Count the number of people on the system
 */
#ifndef USGV4		/*Must be a 5.0 UNIX system */
#include <sys/types.h>
#include <utmp.h>
struct utmp buf;
ucount()
{
	reg struct utmp *up;
	reg FILE *utmp;
	reg int count;

	if ((utmp = fopen(UTMP, "r")) == NULL)
	    return 0;

	up = &buf;
	count = 0;
	while (fread(up, 1, sizeof (*up), utmp) > 0)
		if (buf.ut_type == USER_PROCESS)
			count++;
	fclose(utmp);
	return count;
}
#else		/* Otherwise it's a USG 3.0 or 4.0 UNIX */
#include <utmp.h>

struct utmp buf;

ucount()
{
    register struct utmp *up;
    register FILE *utmp;
    register int count;

    if ((utmp = fopen(UTMP, "r")) == NULL)
	return 0;

    up = &buf;
    count = 0;

    while (fread(up, 1, sizeof (*up), utmp) > 0)
	if (buf.ut_name[0] != '\0')
	    count++;
    fclose(utmp);
    return count;
}
#endif		/* end ifndef USGV4 */

/*
 * holiday:
 *	Returns TRUE when it is a good time to play rogue
 */
holiday()
{
	long now;
	struct tm *localtime();
	reg struct tm *ntime;

	if (!HOLIDAY)
		return TRUE;

	time(&now);			/* get the current time */
	ntime = localtime(&now);
	if(ntime->tm_wday == 0 || ntime->tm_wday == 6)
		return TRUE;		/* OK on Sat & Sun */
	if(ntime->tm_hour < 9 || ntime->tm_hour >= 17)
		return TRUE;		/* OK before 9AM & after 5PM */
	return FALSE;			/* All other times are bad */
}
