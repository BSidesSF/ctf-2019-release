/****************************************************************************    
  Copyright (c) 1999,2000 WU-FTPD Development Group.  
  All rights reserved.
  
  Portions Copyright (c) 1980, 1985, 1988, 1989, 1990, 1991, 1993, 1994 
    The Regents of the University of California.
  Portions Copyright (c) 1993, 1994 Washington University in Saint Louis. 
  Portions Copyright (c) 1996, 1998 Berkeley Software Design, Inc. 
  Portions Copyright (c) 1989 Massachusetts Institute of Technology. 
  Portions Copyright (c) 1998 Sendmail, Inc. 
  Portions Copyright (c) 1983, 1995, 1996, 1997 Eric P.  Allman. 
  Portions Copyright (c) 1997 by Stan Barber. 
  Portions Copyright (c) 1997 by Kent Landfield. 
  Portions Copyright (c) 1991, 1992, 1993, 1994, 1995, 1996, 1997 
    Free Software Foundation, Inc.   
  
  Use and distribution of this software and its source code are governed  
  by the terms and conditions of the WU-FTPD Software License ("LICENSE"). 
  
  If you did not receive a copy of the license, it may be obtained online 
  at http://www.wu-ftpd.org/license.html. 
  
  $Id: glob.c,v 1.14 2000/07/01 18:17:39 wuftpd Exp $ 
  
****************************************************************************/
/*
 * C-shell glob for random programs.
 */

#include "config.h"

#include <sys/param.h>
#include <sys/stat.h>

#ifdef HAVE_DIRENT_H
#include <dirent.h>
#else
#include <sys/dir.h>
#endif

#include <pwd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>

#include "proto.h"

#define	QUOTE 0200
#define	TRIM 0177
#define	eq(a,b)		(strcmp(a, b)==0)
#define	GAVSIZ		(NCARGS/6)
#define	isdir(d)	((d.st_mode & S_IFMT) == S_IFDIR)

static char **gargv;		/* Pointer to the (stack) arglist */
static int gargc;		/* Number args in gargv */
static size_t gnleft;
static short gflag;
static int tglob(register char);

/* Prototypes */

static char *strend(register char *);
static void addpath(char);
static void ginit(char **);
static void collect(register char *);
static void acollect(register char *);
static void sort(void);
static void expand(char *);
static void matchdir(char *);
static int execbrc(char *, char *);
static int match(char *, char *);
static int amatch(char *, char *);
static void Gcat(register char *, register char *);
static void addpath(char);
static void rscan(register char **, int (*f) (register char));
static int tglob(register char c);

void blkfree(char **);
int letter(register char);
int digit(register char);
int gethdir(char *);
int any(register int, register char *);
int blklen(register char **);
char **ftpglob(register char *);
char *strspl(register char *, register char *);
char **copyblk(register char **);
char **blkcpy(char **, register char **);

char *globerr;
char *home;
extern int errno;

static int globcnt;

char *globchars = "`{[*?";

static char *gpath, *gpathp, *lastgpathp;
static int globbed;
static char *entp;
static char **sortbas;

#ifdef OTHER_PASSWD
#include "getpwnam.h"
extern char _path_passwd[];
#endif

void fatal(char *s)
{
	printf("441. Server error: \"%s\"\n", s);
	exit(1);
}

/*
 * fixpath
 *
 * In principal, this is similar to realpath() or the mapping chdir function.
 * It removes unnecessary path components.  We do this to put a stop to
 * attempts to cause a memory starvation DoS.
 *
 */

void fixpath(char *path)
{
    int abs = 0;
    char *in;
    char *out;

    if (*path == '/') {
	abs = 1;
	path++;
    }
    else if (*path == '~') {
	do
	    path++;
	while ((*path != '\0') && (*path != '/'));
	if (*path == '/')
	    path++;
    }
    in = path;
    out = path;
    while (*in != '\0') {
	if (*in == '/')
	    in++;
	else if ((in[0] == '.') && ((in[1] == '/') || (in[1] == '\0'))) {
	    in++;
	    if (*in == '/')
		in++;
	}
	else if ((in[0] == '.') && (in[1] == '.') && ((in[2] == '/') || (in[2] == '\0'))) {
	    if (out == path) {
		if (abs) {
		    in++;
		    in++;
		    if (*in == '/')
			in++;
		}
		else {
		    *out++ = *in++;
		    *out++ = *in++;
		    if (*in == '/')
			*out++ = *in++;
		    path = out;
		}
	    }
	    else {
		out--;
		while ((out != path) && (*--out != '/'));
		in++;
		in++;
		if (*in == '/')
		    in++;
	    }
	}
	else {
	    do
		*out++ = *in++;
	    while ((*in != '\0') && (*in != '/'));
	    if (*in == '/')
		*out++ = *in++;
	}
    }
    *out = '\0';
}


char **ftpglob(register char *v)
{
    char agpath[BUFSIZ];
    char *agargv[GAVSIZ];
    char *vv[2];

    fixpath(v);
    if (v[0] == '\0')
	v = "*";
    else if ((strlen(v) > 1) && (v[strlen(v) - 1] == '/'))
	v[strlen(v) - 1] = '\0';

    vv[0] = v;
    vv[1] = NULL;
    globerr = NULL;
    gflag = 0;
    rscan(vv, tglob);
    if (gflag == 0) {
	vv[0] = strspl(v, "");
	return (copyblk(vv));
    }

    globerr = NULL;
    gpath = agpath;
    gpathp = gpath;
    *gpathp = 0;
    lastgpathp = &gpath[sizeof agpath - 2];
    ginit(agargv);
    globcnt = 0;
    collect(v);
    if (globcnt == 0 && (gflag & 1)) {
	blkfree(gargv), gargv = 0;
	return (0);
    }
    else
	return (gargv = copyblk(gargv));
}

static void ginit(char **agargv)
{

    agargv[0] = 0;
    gargv = agargv;
    sortbas = agargv;
    gargc = 0;
    gnleft = NCARGS - 4;
}

static void collect(register char *as)
{
    if (eq(as, "{") || eq(as, "{}")) {
	Gcat(as, "");
	sort();
    }
    else
	acollect(as);
}

static void acollect(register char *as)
{
    register int ogargc = gargc;

    gpathp = gpath;
    *gpathp = 0;
    globbed = 0;
    expand(as);
    if (gargc != ogargc)
	sort();
}

static void sort(void)
{
    register char **p1, **p2, *c;
    char **Gvp = &gargv[gargc];

    p1 = sortbas;
    while (p1 < Gvp - 1) {
	p2 = p1;
	while (++p2 < Gvp)
	    if (strcmp(*p1, *p2) > 0)
		c = *p1, *p1 = *p2, *p2 = c;
	p1++;
    }
    sortbas = Gvp;
}

static void expand(char *as)
{
    register char *cs;
    register char *sgpathp, *oldcs;
    struct stat stb;

    if (globerr)
	return;
    sgpathp = gpathp;
    cs = as;
    if (*cs == '~' && gpathp == gpath) {
	addpath('~');
	for (cs++; letter(*cs) || digit(*cs) || *cs == '-';)
	    addpath(*cs++);
	if (!*cs || *cs == '/') {
	    if (gpathp != gpath + 1) {
		*gpathp = 0;
		if (gethdir(gpath + 1))
		    globerr = "Unknown user name after ~";
		(void) strcpy(gpath, gpath + 1);
	    }
	    else
		(void) strcpy(gpath, home);
	    gpathp = strend(gpath);
	}
    }
    while (!any(*cs, globchars)) {
	if (*cs == 0) {
	    if (!globbed)
		Gcat(gpath, "");
	    else if (stat(gpath, &stb) >= 0) {
		Gcat(gpath, "");
		globcnt++;
	    }
	    goto endit;
	}
	addpath(*cs++);
    }
    oldcs = cs;
    while (cs > as && *cs != '/')
	cs--, gpathp--;
    if (*cs == '/')
	cs++, gpathp++;
    *gpathp = 0;
    if (*oldcs == '{') {
	(void) execbrc(cs, ((char *) 0));
	return;
    }
    matchdir(cs);
  endit:
    gpathp = sgpathp;
    *gpathp = 0;
}

static void matchdir(char *pattern)
{
    struct stat stb;

#ifdef HAVE_DIRENT_H
    register struct dirent *dp;
#else
    register struct direct *dp;
#endif

    DIR *dirp;

    dirp = opendir(*gpath == '\0' ? "." : gpath);
    if (dirp == NULL) {
	if (globbed)
	    return;
	goto patherr2;
    }
#ifdef HAVE_DIRFD
    if (fstat(dirfd(dirp), &stb) < 0)
#else /* HAVE_DIRFD */
    if (fstat(dirp->dd_fd, &stb) < 0)
#endif /* HAVE_DIRFD */
	goto patherr1;
    if (!isdir(stb)) {
	errno = ENOTDIR;
	goto patherr1;
    }
    while (!globerr && ((dp = readdir(dirp)) != NULL)) {
	if (dp->d_ino == 0)
	    continue;
	if (match(dp->d_name, pattern)) {
	    Gcat(gpath, dp->d_name);
	    globcnt++;
	}
    }
    closedir(dirp);
    return;

  patherr1:
    closedir(dirp);
  patherr2:
    globerr = "Bad directory components";
}

static int execbrc(char *p, char *s)
{
    char restbuf[BUFSIZ + 2];
    register char *pe, *pm, *pl;
    int brclev = 0;
    char *lm, savec, *sgpathp;

    for (lm = restbuf; *p != '{'; *lm++ = *p++)
	continue;
    for (pe = ++p; *pe; pe++)
	switch (*pe) {

	case '{':
	    brclev++;
	    continue;

	case '}':
	    if (brclev == 0)
		goto pend;
	    brclev--;
	    continue;

	case '[':
	    for (pe++; *pe && *pe != ']'; pe++)
		continue;
	    continue;
	}
  pend:
    brclev = 0;
    for (pl = pm = p; pm <= pe; pm++)
	switch (*pm & (QUOTE | TRIM)) {

	case '{':
	    brclev++;
	    continue;

	case '}':
	    if (brclev) {
		brclev--;
		continue;
	    }
	    goto doit;

	case ',' | QUOTE:
	case ',':
	    if (brclev)
		continue;
	  doit:
	    savec = *pm;
	    *pm = 0;
	    (void) strcpy(lm, pl);
	    (void) strcat(restbuf, pe + 1);
	    *pm = savec;
	    if (s == 0) {
		sgpathp = gpathp;
		expand(restbuf);
		gpathp = sgpathp;
		*gpathp = 0;
	    }
	    else if (amatch(s, restbuf))
		return (1);
	    sort();
	    pl = pm + 1;
	    if (brclev)
		return (0);
	    continue;

	case '[':
	    for (pm++; *pm && *pm != ']'; pm++)
		continue;
	    if (!*pm)
		pm--;
	    continue;
	}
    if (brclev)
	goto doit;
    return (0);
}

static int match(char *s, char *p)
{
    register int c;
    register char *sentp;
    char sglobbed = globbed;

    if (*s == '.' && *p != '.')
	return (0);
    sentp = entp;
    entp = s;
    c = amatch(s, p);
    entp = sentp;
    globbed = sglobbed;
    return (c);
}

static int amatch(char *s, char *p)
{
    register int scc;
    int ok, lc;
    char *sgpathp;
    struct stat stb;
    int c, cc;

    globbed = 1;
    for (;;) {
	scc = *s++ & TRIM;
	switch (c = *p++) {

	case '{':
	    return (execbrc(p - 1, s - 1));

	case '[':
	    ok = 0;
	    lc = 077777;
	    while ((cc = *p++)) {
		if (cc == ']') {
		    if (ok)
			break;
		    return (0);
		}
		if (cc == '-') {
		    if (lc <= scc && scc <= *p++)
			ok++;
		}
		else if (scc == (lc = cc))
		    ok++;
	    }
	    if (cc == 0)
		if (ok)
		    p--;
		else
		    return 0;
	    continue;

	case '*':
	    if (!*p)
		return (1);
	    if (*p == '/') {
		p++;
		goto slash;
	    }
	    s--;
	    do {
		if (amatch(s, p))
		    return (1);
	    } while (*s++);
	    return (0);

	case 0:
	    return (scc == 0);

	default:
	    if (c != scc)
		return (0);
	    continue;

	case '?':
	    if (scc == 0)
		return (0);
	    continue;

	case '/':
	    if (scc)
		return (0);
	  slash:
	    s = entp;
	    sgpathp = gpathp;
	    while (*s)
		addpath(*s++);
	    addpath('/');
	    if (stat(gpath, &stb) == 0 && isdir(stb))
		if (*p == 0) {
		    Gcat(gpath, "");
		    globcnt++;
		}
		else
		    expand(p);
	    gpathp = sgpathp;
	    *gpathp = 0;
	    return (0);
	}
    }
}

/* This function appears to be unused, so why waste time and space on it? */
#if 0 == 1
static int Gmatch(register char *s, register char *p)
{
    register int scc;
    int ok, lc;
    int c, cc;

    for (;;) {
	scc = *s++ & TRIM;
	switch (c = *p++) {

	case '[':
	    ok = 0;
	    lc = 077777;
	    while (cc = *p++) {
		if (cc == ']') {
		    if (ok)
			break;
		    return (0);
		}
		if (cc == '-') {
		    if (lc <= scc && scc <= *p++)
			ok++;
		}
		else if (scc == (lc = cc))
		    ok++;
	    }
	    if (cc == 0)
		if (ok)
		    p--;
		else
		    return 0;
	    continue;

	case '*':
	    if (!*p)
		return (1);
	    for (s--; *s; s++)
		if (Gmatch(s, p))
		    return (1);
	    return (0);

	case 0:
	    return (scc == 0);

	default:
	    if ((c & TRIM) != scc)
		return (0);
	    continue;

	case '?':
	    if (scc == 0)
		return (0);
	    continue;

	}
    }
}
#endif /* Gmatch exclusion */

static void Gcat(register char *s1, register char *s2)
{
    register size_t len = strlen(s1) + strlen(s2) + 1;

    if (len >= gnleft || gargc >= GAVSIZ - 1)
	globerr = "Arguments too long";
    else {
	gargc++;
	gnleft -= len;
	gargv[gargc] = 0;
	gargv[gargc - 1] = strspl(s1, s2);
    }
}

static void addpath(char c)
{

    if (gpathp >= lastgpathp)
	globerr = "Pathname too long";
    else {
	*gpathp++ = c;
	*gpathp = 0;
    }
}

static void rscan(register char **t, int (*f) (register char))
{
    register char *p, c;

    while ((p = *t++)) {
	if (*p == '~')
	    gflag |= 2;
	else if (eq(p, "{") || eq(p, "{}"))
	    continue;
	while ((c = *p++))
	    (*f) (c);
    }
}
static int tglob(register char c)
{
    if (any(c, globchars))
	gflag |= c == '{' ? 2 : 1;
    return (c);
}

int letter(register char c)
{
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z'))
	    || (c == '_'));
}

int digit(register char c)
{
    return (c >= '0' && c <= '9');
}

int any(register int c, register char *s)
{
    while (*s)
	if (*s++ == c)
	    return (1);
    return (0);
}

int blklen(register char **av)
{
    register int i = 0;

    while (*av++)
	i++;
    return (i);
}

char **blkcpy(char **oav, register char **bv)
{
    register char **av = oav;

    while ((*av++ = *bv++))
	continue;
    return (oav);
}

void blkfree(char **av0)
{
    register char **av = av0;

    while (*av)
	free(*av++);
}

char *strspl(register char *cp, register char *dp)
{
    register char *ep =
    (char *) malloc((unsigned) (strlen(cp) + strlen(dp) + 1));

    if (ep == (char *) 0)
	fatal("Out of memory");
    (void) strcpy(ep, cp);
    (void) strcat(ep, dp);
    return (ep);
}

char **copyblk(register char **v)
{
    register char **nv = (char **) malloc((unsigned) ((blklen(v) + 1) *
						      sizeof(char **)));
    if (nv == (char **) 0)
	fatal("Out of memory");

    return (blkcpy(nv, v));
}

static char *strend(register char *cp)
{
    while (*cp)
	cp++;
    return (cp);
}
/*
 * Extract a home directory from the password file
 * The argument points to a buffer where the name of the
 * user whose home directory is sought is currently.
 * We write the home directory of the user back there.
 */
int gethdir(char *home)
{
#ifdef OTHER_PASSWD
    register struct passwd *pp = bero_getpwnam(home, _path_passwd);
#else
    register struct passwd *pp = getpwnam(home);
#endif
    register char *root = NULL;
    if (!pp || home + strlen(pp->pw_dir) >= lastgpathp)
	return (1);
    root = strstr(pp->pw_dir, "/./");
    (void) strcpy(home, root ? (root + 2) : pp->pw_dir);

    return (0);
}





