/*
 * save and restore routines
 *
 * @(#)save.c	3.9 (Berkeley) 6/16/81
 */

#include "curses.h"
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <signal.h>
#include "rogue.h"

typedef struct stat STAT;

extern char *sys_errlist[], version[];
extern int errno;

char *sbrk();

STAT sbuf;

int sigdie = -1;

save_game()
{
    register FILE *savef;
    register int c;
    char buf[LINELEN];

    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0')
    {
	msg("Save file (%s)? ", file_name);
	do
	{
	    c = getchar() & 0177;
	    if (c == ESCAPE) return(0);
	} while (c != 'n' && c != 'N' && c != 'y' && c != 'Y');
	mpos = 0;
	if (c == 'y' || c == 'Y')
	{
	    msg("File name: %s", file_name);
	    goto gotfile;
	}
    }

    do
    {
	msg("File name: ");
	mpos = 0;
	buf[0] = '\0';
	if (get_str(buf, cw) == QUIT)
	{
	    msg("");
	    return FALSE;
	}
	strcpy(file_name, buf);
gotfile:
	if ((savef = fopen(file_name, "w")) == NULL)
	    msg(sys_errlist[errno]);	/* fake perror() */
    } while (savef == NULL);

    /*
     * write out encrpyted file (after a stat)
     * The fwrite is to force allocation of the buffer before the write
     */
    save_file(savef);
    return TRUE;
}

/*
 * automatically save a file.  This is used if a HUP signal is
 * recieved
 */
auto_save(sig)
int sig;
{
    register FILE *savef;
    FILE *fopen(),*ferrsig;
    register int i;

    /*
     * Code added by Bruce Dautrich on 4/11/84 to
     * increase probability of saving game if two 
     * signals are sent to game. Requires define
     * in rogue.h of a variable called sigdie
     */

    if (sigdie == -1) {
	/*This is first signal*/
	sigdie=sig;
	signal(sig,SIG_IGN);
	goto got1sig;
    } else {
	/*This is second signal*/
	signal(sig,SIG_IGN);
    }

got1sig:
    if (file_name[0] != '\0' && (savef = fopen(file_name, "w")) != NULL)
	save_file(savef);
    exit(1);
}

/*
 * write the saved game on the file
 */
save_file(savef)
register FILE *savef;
{
    wmove(cw, LINES-1, 0);
    draw(cw);
    fstat(fileno(savef), &sbuf);
    fwrite("junk", 1, 5, savef);
    fseek(savef, 0L, 0);
    encwrite(version, sbrk(0) - version, savef);
    fclose(savef);
}

/*This is only until all have used this rogue version 1.2 */
restore(file, envp)
register char *file;
char **envp;
{
    register int inf;
    extern char **environ;
    char buf[LINELEN];
    STAT sbuf2;

    makesure();

    /*
     * Reset the effective uid & gid to the real ones.
     */
    seteuid(getuid());
    setegid(getgid());

    if (strcmp(file, "-r") == 0)
	file = file_name;
    if ((inf = open(file, 0)) < 0)
    {
	perror(file);
	return FALSE;
    }

    fflush(stdout);
    encread(buf, strlen(version) + 1, inf);
    if (strcmp(buf, version) != 0)
    {
	printf("Save Game Version: %s\nReal Game Version: %s\n", buf, version);
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }

    fstat(inf, &sbuf2);
    fflush(stdout);
    brk(version + sbuf2.st_size);
    lseek(inf, 0L, 0);
    encread(version, (unsigned int) sbuf2.st_size, inf);
    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    if (!wizard) {
	if (sbuf2.st_ino != sbuf.st_ino || sbuf2.st_dev != sbuf.st_dev) {
	    printf("Sorry, saved game is not in the same file.\n");
	    return FALSE;
	}
	else if (sbuf2.st_ctime - sbuf.st_ctime > 1800) {
	    printf("(%d): Sorry, file has been touched.\n"
		,sbuf2.st_ctime - sbuf.st_ctime);
	    return FALSE;
	}
    }
    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    /*
     * defeat multiple restarting from the same place
     */
    if (!wizard) {
	if (sbuf2.st_nlink != 1) {
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	}
	else if (unlink(file) < 0) {
	    printf("Cannot unlink file\n");
	    return FALSE;
	}
    }

    environ = envp;
    if (!My_term && isatty(2))
    {
	register char	*sp;

	_tty_ch = 2;
	gettmode();
	if ((sp = getenv("TERM")) == NULL)
	    sp = Def_term;
	setterm(sp);
    }
    else
	setterm(Def_term);
    strcpy(file_name, file);
    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    srand(getpid());
    playit();
    /*NOTREACHED*/
}

/*****************************************************************
 *
 *           encwrite, encread: encoded read/write routines
 *
 * The encwrite/encread routines implement a synchronous stream Vernam
 * cipher using a linear congruential random number generator to
 * simulate a one-time pad.  The random seed is encoded and stored in
 * the file using data diffusion (putword,getword).
 *
 * Usage:
 *	encwrite (start, size, outf);
 *	char *start; int size; FILE *outf;
 *
 *	encread (start, size, infd);
 *	char *start; int size; int infd;
 *
 * HISTORY
 * 03-Mar-85  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Modified for UltraRogue.  Allowed multiple encwrites to a file by
 *	removing lseek calls and computing checksum while reading.
 *
 * 20-Dec-82  Michael Mauldin (mlm) at Carnegie-Mellon University
 *	Created as a replacement for the encwrite/encread in
 *	Rogue 5.2, which are very easily broken.
 *
 *****************************************************************/

# include "stdio.h"

/* Constants for key generation */
# define OFFSET		667818
# define MODULUS	894871
# define MULT		2399
# define ENCHAR		(((seed= ((seed*MULT+OFFSET)%MODULUS)) >> 10) & 0xff)

/* Constants for checksumming */
# define INITCK		1232531
# define CKMOD		6506347

struct wb { int w1, w2, w3, w4; };

/*****************************************************************
 * Encwrite: write a buffer to a file using data encryption
 *****************************************************************/

encwrite (start, size, outf)
register char *start;
int size;
register FILE *outf;
{ register int cksum = INITCK, seed, ch;

  srand (time (0) + getpid () + getuid ());	/* Build a random seed */
  seed = (rand () & 0x7fff);
  
  putword (seed, outf);				/* Write the random seed */
  putword (size, outf);				/* Write the file length */

# ifdef DEBUG
  fprintf (stderr, "Encwrite: size %ld, seed %ld.\n", size, seed);
# endif

  /* Now checksum, encrypt, and write out the buffer. */
  while (size--)
  { ch = *start++ & 0xff;			/* Get the next char */
    cksum = ((cksum << 8) + ch) % CKMOD;	/* Checksum clear text */
    putc (ch ^ ENCHAR, outf);			/* Write ciphertext */
  }

  putword (cksum, outf);			/* Write out the checksum */
  fflush (outf);				/* Flush the output */

# ifdef DEBUG
  fprintf (stderr, "Checksum is %ld.\n", cksum);
# endif
}

/*****************************************************************
 * Encread: read a block of encrypted text from a file descriptor
 *****************************************************************/

encread (start, size, infd)
register char *start;
int size;
int infd;
{ register int cksum = INITCK, seed, length, cnt, ch;
  int stored, getword();

  seed = getword (infd); 
  stored = getword (infd);  

# ifdef DEBUG
  fprintf (stderr, "Encread: size %ld, seed %ld, stored %ld.\n", 
	   size, seed, stored);
# endif
  
  if ((length = read (infd, start, (int) min (size, stored))) > 0)
  { for (cnt = length; cnt--; ) 
    { ch = (*start++ ^= ENCHAR) & 0xff;
      cksum = ((cksum << 8) + ch) % CKMOD;
    }
  
    if ((length == stored) && (getword (infd) != cksum))
    {
# ifdef DEBUG
      fprintf (stderr, "Computed checksum %ld is wrong.\n", cksum);
# else
      fprintf (stderr, "Sorry, file has been touched.\n");
# endif
      while (length--) *--start = '\0';		/* Zero the buffer */
    }
  }
  
  return (length);
}

/*****************************************************************
 * putword: Write out an encoded int word
 *****************************************************************/

putword (word, file)
int word;
FILE *file;
{ struct wb w;

  w.w1 = rand ();  
  w.w2 = rand ();  
  w.w3 = rand ();  
  w.w4 = w.w1 ^ w.w2 ^ w.w3 ^ word;
  
  return (fwrite ((char *) &w, sizeof (struct wb), 1, file));
}

/*****************************************************************
 * getword: Read in an encoded int word
 *****************************************************************/

getword (fd)
int fd;
{ struct wb w;
  
  if (read (fd, (char *) &w, sizeof (struct wb)) == sizeof (struct wb))
    return (w.w1 ^ w.w2 ^ w.w3 ^ w.w4);
  else
    return (0);
}
