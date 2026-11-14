/*
 * machine dependicies
 *
 * @(#)mach_dep.h	X.XX (Berkeley) XX/XX/XX
 */
/*
 * where scorefile should live
 */
#ifndef SCOREFILE
#define SCOREFILE	"/usr/public/.rogue_roll"
#endif

/*
 * Variables for checking to make sure the system isn't too loaded
 * for people to play.  Either can be set to 0 to leave out the check.
 */

#ifndef MAXUSERS
#define	MAXUSERS	25	/* max number of users for this game */
#endif
#ifndef MAXLOAD
#define	MAXLOAD		55	/* 10 * max 15 minute load average */
#endif

#if MAXUSERS|MAXLOAD
#define	CHECKTIME	15	/* number of minutes between load checks */
				/* if not defined checks are only on startup */
#endif

