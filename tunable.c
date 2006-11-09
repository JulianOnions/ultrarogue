/*
 * Machine dependent (tunable) parametesafe.rs.
 */
#define ROGUEPATH(FILE) "/xtel/users/jpo/games/lib/ur/FILE"

/*
 * Wizard's identity.
 */
int	UID		=	97;			/* his user id */
char	LOGIN[]		=	"games";		/* his login name */

/*
 * Special files.
 */

/*
 * File containing uids of people considered authosafe.rs.
 */
char	PERMOK[]	=	ROGUEPATH(permok);

/*
 * File containing wizard's password and password key.
 */ 
char	PASSCTL[]	=	ROGUEPATH(passctl);

/*
 * Directory where the scores are stored.
 */
char	SCOREDIR[]	=	ROGUEPATH(urogue_roll);

/*
 * Location of lav program to print load averages
 */
char	LOADAV_PROG[]	=	"/usr/local/la";

/*
 * Location of utmp
 */
char	UTMP[]		=	"/etc/utmp";

/*
 * Load control constants
 */
int	MAXLOAD		=	50;	/* 10 * max 15 minute load average */
int	MAXUSERS	=	15;	/* max users at start */
int	HOLIDAY		=	0;	/* enable prime shift restrictions */
int	CHECKTIME	=	2000000;	/* minutes between load checks */
