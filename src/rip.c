/*
 * File for the fun ends
 * Death or a total win
 *
 * @(#)rip.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <time.h>
#include <signal.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/resource.h>
#include <fcntl.h>
#include "mach_dep.h"
#include "rogue.h"

static char *rip[] = {
"                       __________",
"                      /          \\",
"                     /    REST    \\",
"                    /      IN      \\",
"                   /     PEACE      \\",
"                  /                  \\",
"                  |                  |",
"                  |                  |",
"                  |   killed by a    |",
"                  |                  |",
"                  |       1980       |",
"                 *|     *  *  *      | *",
"         ________)/\\\\_//(\\/(/\\)/\\//\\/|_)_______",
    0
};

char	*killname();

/*
 * death:
 *	Do something really fun when he dies
 */
death(monst)
register char monst;
{
    register char **dp = rip, *killer;
    register struct tm *lt;
    time_t date;
    char buf[80];
    struct tm *localtime();

    file_name[0] = '\0';
    time(&date);
    lt = localtime(&date);
    clear();
    move(8, 0);
    while (*dp)
	printw("%s\n", *dp++);
    mvaddstr(14, 28-((strlen(whoami)+1)/2), whoami);
    purse -= purse/10;
    sprintf(buf, "%d Au", purse);
    mvaddstr(15, 28-((strlen(buf)+1)/2), buf);
    killer = killname(monst);
    mvaddstr(17, 28-((strlen(killer)+1)/2), killer);
    if (*killer == 'a' || *killer == 'e' || *killer == 'i' ||
	*killer == 'o' || *killer == 'u')
	    mvaddch(16, 33, 'n');
    sprintf(prbuf, "%4d", 1900 + lt->tm_year);
    mvaddstr(18, 26, prbuf);
    move(LINES-1, 0);
    draw(stdscr);
    score(purse, 0, monst);
    endwin();
    exit(0);
}

/*
 * score -- figure score and post it.
 */
/* VARARGS2 */
score(amount, flags, monst)
char monst;
{
    struct sc_ent {
	int sc_score;
	char sc_name[80];
	int sc_flags;
	int sc_level;
	char sc_monster;
    } top_ten[10];
    register int i, j, got;
    register FILE *outf;
    register char *killer;
    struct rlimit lim;
    sigset_t set, oset;

    /*
     * Open file and read list
     */

    {
    static char *reason[] = {
	"killed",
	"quit",
	"A total winner",
    };
    if (fd < 0)
	return;
    outf = fdopen(fd, "w");

    memset((char *) top_ten, 0, sizeof top_ten);
    for (i = 0; i < 10; i++)
    {
	top_ten[i].sc_score = 0;
	for (j = 0; j < 80; j++)
	    top_ten[i].sc_name[j] = rnd(255);
	top_ten[i].sc_flags = RN;
	top_ten[i].sc_level = RN;
	top_ten[i].sc_monster = RN;
    }

    file_name[0] = '\0';
    signal(SIGINT, SIG_DFL);
#ifdef SIGWINCH
    signal(SIGWINCH, SIG_IGN);
#endif
#ifdef SIGTSTP
    signal(SIGTSTP, SIG_DFL);
#endif
    if (!waswizard)
	hold_sc();
    got = encread((char *) top_ten, sizeof top_ten, fd);
    for (i = j = 0; i < 10; i++)
	if ((int) ((i + 1) * sizeof top_ten[0]) <= got && top_ten[i].sc_score > 0
	    && top_ten[i].sc_flags >= 0 && top_ten[i].sc_flags <= 2)
		top_ten[j++] = top_ten[i];
    while (j < 10)
	top_ten[j++].sc_score = 0;
    /*
     * Insert her in list if need be
     */
    if (!waswizard)
    {
	for (i = 0; i < 10; i++)
	    if (amount > top_ten[i].sc_score)
		break;
	if (i < 10)
	{
	    for (j = 9; j > i; j--)
		top_ten[j] = top_ten[j-1];
	    top_ten[i].sc_score = amount;
	    strcpy(top_ten[i].sc_name, whoami);
	    top_ten[i].sc_flags = flags;
	    if (flags == 2)
		top_ten[i].sc_level = max_level;
	    else
		top_ten[i].sc_level = level;
	    top_ten[i].sc_monster = monst;
	}
    }
    /*
     * Update the list file, and let go of it, before waiting for the
     * player, who may never come back
     */
    sigemptyset(&set);
    sigaddset(&set, SIGHUP);
    sigaddset(&set, SIGINT);
    sigaddset(&set, SIGQUIT);
    sigaddset(&set, SIGTERM);
    sigprocmask(SIG_BLOCK, &set, &oset);
    if (!waswizard
	&& !(getrlimit(RLIMIT_FSIZE, &lim) == 0 && lim.rlim_cur < sizeof top_ten))
    {
	fseek(outf, 0L, 0);
	encwrite((char *) top_ten, sizeof top_ten, outf);
    }
    fclose(outf);
    fd = -1;
    sigprocmask(SIG_SETMASK, &oset, (sigset_t *) NULL);
    if (flags != -1)
    {
	endwin();
	printf("[Press return to continue]");
	wait_for('\n');
    }
    /*
     * Print the list
     */
    printf("\nTop Ten Adventurers:\nRank\tScore\tName\n");
    for (i = 0; i < 10; i++) {
	if (top_ten[i].sc_score
	    && top_ten[i].sc_flags >= 0 && top_ten[i].sc_flags <= 2) {
	    top_ten[i].sc_name[79] = '\0';
	    for (j = 0; top_ten[i].sc_name[j] != '\0'; j++)
		if (top_ten[i].sc_name[j] < ' ' || top_ten[i].sc_name[j] > '~')
		    top_ten[i].sc_name[j] = '?';
	    printf("%d\t%d\t%s: %s on level %d", i + 1,
		top_ten[i].sc_score, top_ten[i].sc_name, reason[top_ten[i].sc_flags],
		top_ten[i].sc_level);
	    if (top_ten[i].sc_flags == 0) {
		printf(" by a");
		killer = killname(top_ten[i].sc_monster);
		if (*killer == 'a' || *killer == 'e' || *killer == 'i' ||
		    *killer == 'o' || *killer == 'u')
			putchar('n');
		printf(" %s", killer);
	    }
	    printf(".\n");
	}
    }
    }
}

/*
 * hold_sc:
 *	Lock the score file.  The lock goes when the file is closed.
 *	Return TRUE if the file could be locked.
 */
hold_sc()
{
    register int cnt;
    static struct flock fl;

    fl.l_type = F_WRLCK;
    for (cnt = 0; fcntl(fd, F_SETLK, &fl) < 0; cnt++)
    {
	if ((errno != EAGAIN && errno != EACCES) || cnt == 5)
	    return FALSE;
	sleep(1);
    }
    return TRUE;
}

total_winner()
{
    register struct linked_list *item;
    register struct object *obj;
    register int worth;

    file_name[0] = '\0';
    for (item = pack; item != NULL; item = next(item))
    {
	obj = (struct object *) ldata(item);
	switch (obj->o_type)
	{
	    when FOOD:
		purse += 2;
	    case WEAPON:
		switch (obj->o_which)
		{
		    when MACE: worth = 8;
		    when SWORD: worth = 15;
		    when BOW: worth = 75;
		    when ARROW: worth = 1;
		    when DAGGER: worth = 2;
		    when ROCK: worth = 1;
		    when TWOSWORD: worth = 30;
		    when SLING: worth = 1;
		    when DART: worth = 1;
		    when CROSSBOW: worth = 15;
		    when BOLT: worth = 1;
		    when SPEAR: worth = 2;
		    otherwise: worth = 0;
		}
		purse += worth * (1 + (10 * (obj->o_dplus + obj->o_hplus)));
	    when ARMOR:
		switch (obj->o_which)
		{
		    when LEATHER: worth = 5;
		    when RING_MAIL: worth = 30;
		    when STUDDED_LEATHER: worth = 15;
		    when SCALE_MAIL: worth = 3;
		    when CHAIN_MAIL: worth = 75;
		    when SPLINT_MAIL: worth = 80;
		    when BANDED_MAIL: worth = 90;
		    when PLATE_MAIL: worth = 400;
		    otherwise: worth = 0;
		}
		purse += worth * (1 + (10 * (a_class[obj->o_which] - obj->o_ac)));
	    when POTION:
	    case SCROLL:
		purse += rnd(100) + 50;
	    when AMULET:
		purse += 1000;
	}
    }
    wclear(cw);
    wmove(cw, LINES-1, 0);
    draw(cw);
    endwin();
    printf("Congratulations, you have made it to the light of day!\n");
    printf("You have joined the elite ranks of those who have escaped\n");
    printf("the Dungeons of Doom alive.  You journey home and sell all your\n");
    printf("loot at a great profit and are admitted to the fighters guild.\n");
    score(purse, 2, 0);
    exit(0);
}

char *
killname(monst)
register char monst;
{
    if (isupper((unsigned char) monst))
	return monsters[monst-'A'].m_name;
    else if (monst == 'a')
	return "arrow";
    else
	return "dart";
}
