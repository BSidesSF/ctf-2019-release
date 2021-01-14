/* src/config.h.  Generated automatically by configure.  */
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
 
  $Id: config.h.in,v 1.31 2000/07/01 18:04:21 wuftpd Exp $
 
****************************************************************************/

/*
 * Configuration file for autoconf - will be modified by configure
 */

#define HAVE_FCNTL_H 1
#define HAVE_DIRENT_H 1
#define HAVE_REGEX_H 1
#define TIME_WITH_SYS_TIME 1
/* #undef HAVE_SYS_TIME_H */
/* #undef HAVE_TIME_H */
#define HAVE_MNTENT_H 1
/* #undef HAVE_SYS_MNTENT_H */
/* #undef HAVE_SYS_MNTTAB_H */
/* #undef HAVE_NDIR_H */
#define HAVE_STRING_H 1
#define HAVE_SYS_DIR_H 1
/* #undef HAVE_SYS_NDIR_H */
/* #undef HAVE_SYS_QUOTA_H */
/* #undef HAVE_SYS_FS_UFS_QUOTA_H */
/* #undef HAVE_UFS_QUOTA_H */
/* #undef HAVE_JFS_QUOTA_H */
/* #undef HAVE_UFS_UFS_QUOTA_H */
/* #undef HAVE_LINUX_QUOTA_H */
#define HAVE_STDLIB_H 1
#define HAVE_UNISTD_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_GLOB_H 1
#define HAVE_GRP_H 1
#define HAVE_SHADOW_H 1
/* #undef HAVE_VMSDIR_H */

/* #undef QUOTA_INODE */
/* #undef QUOTA_DEVICE */
#define QSORT_IS_VOID 1

#define HAVE_SIGPROCMASK 1
#define HAVE_VSNPRINTF 1
#define HAVE_DIRFD 1
#define HAVE_FLOCK 1
#define HAVE_FTW 1
#define HAVE_GETCWD 1
#define HAVE_GETDTABLESIZE 1
#define HAVE_GETRLIMIT 1
/* #undef HAVE_PSTAT */
#define HAVE_LSTAT 1
#define HAVE_VPRINTF 1
#define HAVE_SNPRINTF 1
/* #undef HAVE_REGEX */
#define HAVE_REGEXEC 1
#define HAVE_SETSID 1
#define HAVE_MEMMOVE 1
#define HAVE_STRTOUL 1
#define HAVE_SIGLIST 1
#define FACILITY LOG_FTP

#define HAVE_LIMITS_H 1
#define HAVE_VALUES_H 1
/* #undef HAVE_BSD_BSD_H */
#define HAVE_SYS_PARAM_H 1
/* #undef NEED_LIMITS_H */
/* #undef NEED_VALUES_H */
/* #undef NEED_BSD_BSD_H */
#define NEED_SYS_PARAM_H 1
#if defined(HAVE_SYS_PARAM_H) && defined(NEED_SYS_PARAM_H)
#include <sys/param.h>
#endif
#if defined(HAVE_VALUES_H) && defined(NEED_VALUES_H)
#include <values.h>
#endif
#if defined(HAVE_LIMITS_H) && defined(NEED_LIMITS_H)
#include <limits.h>
#endif
#if defined(HAVE_BSD_BSD_H) && defined(NEED_BSD_BSD_H)
#include <bsd/bsd.h>
#endif
/* #undef NBBY */

#define SIGNAL_TYPE void
#define HAVE_SETUID 1
#define HAVE_SETEUID 1
/* #undef HAVE_SETREUID */
/* #undef HAVE_SETRESUID */
#define HAVE_SETEGID 1
/* #undef HAVE_SETREGID */
/* #undef HAVE_SETRESGID */
#define HAVE_ST_BLKSIZE 1
#define HAVE_SYSCONF 1
/* #undef HAVE_SYS_SYSTEMINFO_H */
#define HAVE_PATHS_H 1
#define HAVE_SYSLOG_H 1
#define HAVE_SYS_SYSLOG_H 1
#define HAVE_FCHDIR 1
#define HAVE_QUOTACTL 1
/* #undef HAS_OLDSTYLE_GETMNTENT */
/* #undef HAS_PW_EXPIRE */
#define SHADOW_PASSWORD 1
#define AUTOCONF 1
#define L_FORMAT "qd"
#define T_FORMAT "qd"
#define PW_UID_FORMAT "u"
#define GR_GID_FORMAT "u"

#define HAVE_UT_UT_HOST 1
#define HAVE_UT_UT_EXIT_E_TERMINATION 1

/* Here instead of everywhere: */
#include <stdio.h>
#include <stdlib.h>
#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

/* Newer systems will have seteuid/setegid */
/* Older systems may have the BSD setreuid/setregid */
/* HP/UX has setresuid/setresgid */
/* Some SCO systems appearently have none of this.
   so if HAVE_SETUID is not defined we'll presume it's
   all needed since we're compiling support/sco.c */

#ifdef HAVE_SETUID

#ifndef HAVE_SETEUID
#ifdef HAVE_SETREUID
#define seteuid(euid) setreuid(-1,(euid))
#else
#ifdef HAVE_SETRESUID
#define seteuid(euid) setresuid(-1,(euid),-1)
#else
#error No seteuid() functions.
#endif
#endif
#endif

#ifndef HAVE_SETEGID
#ifdef HAVE_SETREGID
#define setegid(egid) setregid(-1,(egid))
#else
#ifdef HAVE_SETRESGID
#define setegid(egid) setresgid(-1,(egid),-1)
#else
#error No setegid() functions.
#endif
#endif
#endif

#endif /* HAVE_SETUID */

#ifndef HAVE_FCHDIR
#define HAS_NO_FCHDIR 1
#endif
#ifndef HAVE_QUOTACTL
#define HAS_NO_QUOTACTL 1
#endif
#ifdef HAVE_SYS_SYSTEMINFO_H
#define HAVE_SYSINFO 1
#endif
#ifndef HAVE_SETSID
#define NO_SETSID 1
#endif

#ifndef HAVE_MEMMOVE
#define memmove(a,b,c) bcopy(b,a,c)
#endif
#ifndef HAVE_STRTOUL
#define strtoul(a,b,c) (unsigned long)strtol(a,b,c)
#endif

#ifndef RAND_MAX
#define RAND_MAX 2147483647
#endif

/* #undef USE_PAM */
#include "config-root.h"
