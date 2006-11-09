/*
 * machine dependencies
 *
 */

#define	USGV4	1		/* define if 3.0 or 4.0 UNIX */
				/* if not defined or undef USGV4, UNIX is 5.0 */

#ifdef	USGV4
#define	strchr	index
#define srand48	srandom
#define lrand48	random
#endif
