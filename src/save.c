/*
 * save and restore routines
 *
 * @(#)save.c	X.XX (Berkeley) XX/XX/XX
 */

#include <curses.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <signal.h>
#include "rogue.h"

typedef struct stat STAT;

extern char version[], encstr[], *strerror();
extern bool _endwin;
#include <errno.h>

STAT sbuf;

save_game()
{
    register FILE *savef;
    register int c;
    char buf[80];

    /*
     * get file name
     */
    mpos = 0;
    if (file_name[0] != '\0')
    {
	msg("Save file (\"%s\")? ", file_name);
	do
	{
	    c = getchar();
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
	    msg(strerror(errno));	/* fake perror() */
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
auto_save()
{
    register FILE *savef;

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
    long now;

    wmove(cw, LINES-1, 0);
    draw(cw);
    fwrite("junk", 1, 5, savef);
    fseek(savef, 0L, 0);
    _endwin = TRUE;
    fstat(fileno(savef), &sbuf);
    encwrite(version, (char *) sbrk(0) - version, savef);
    fseek(savef, (long)((char *)&sbuf.st_mtime - version), 0);
    now = time(0);
    fwrite(&now, sizeof now, 1, savef);
    fclose(savef);
}

restore(file, envp)
register char *file;
char **envp;
{
    register int inf;
    extern char **environ;
    char buf[80];
    STAT sbuf2;
    time_t saved_mtime;

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
	printf("Sorry, saved game is out of date.\n");
	return FALSE;
    }

    fstat(inf, &sbuf2);
    fflush(stdout);
    if (brk(version + sbuf2.st_size) < 0)
	perror("brk");
    lseek(inf, (long)((char *)&sbuf.st_mtime - version), 0);
    read(inf, &saved_mtime, sizeof saved_mtime);
    lseek(inf, 0L, 0);
    encread(version, (unsigned int) sbuf2.st_size, inf);
    sbuf2.st_mtime = saved_mtime;
    /*
     * we do not close the file so that we will have a hold of the
     * inode for as long as possible
     */

    mpos = 0;
    mvwprintw(cw, 0, 0, "%s: %s", file, ctime(&sbuf2.st_mtime));

    /*
     * defeat multiple restarting from the same place
     */
    if (!wizard)
	if (sbuf2.st_nlink != 1)
	{
	    printf("Cannot restore from a linked file\n");
	    return FALSE;
	}
	else if (unlink(file) < 0)
	{
	    printf("Cannot unlink file\n");
	    return FALSE;
	}

    environ = envp;
    setterm(getenv("TERM"));
    strcpy(file_name, file);
    setup();
    clearok(curscr, TRUE);
    touchwin(cw);
    srand(getpid());
    playit();
    /*NOTREACHED*/
}

/*
 * perform an encrypted write
 */
encwrite(start, size, outf)
register char *start;
unsigned int size;
register FILE *outf;
{
    register char *ep;

    ep = encstr;

    while (size--)
    {
	if (*ep == '\0')
	    ep = encstr;
	putc(*start++ ^ *ep++, outf);
    }
}

/*
 * perform an encrypted read
 */
encread(start, size, inf)
register char *start;
unsigned int size;
register int inf;
{
    register char *ep;
    register int read_size, got = 0;

    for (read_size = 0; read_size < size; read_size += got)
	if ((got = read(inf, start + read_size, size - read_size)) <= 0)
	    break;
    if (read_size == 0)
	return got;

    ep = encstr;

    while (size--)
    {
	if (*ep == '\0')
	    ep = encstr;
	*start++ ^= *ep++;
    }
    return read_size;
}
