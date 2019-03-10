
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
   
  $Id: proto.h,v 1.10 2000/07/01 18:17:39 wuftpd Exp $  
   
****************************************************************************/
#include <sys/types.h>
#include <sys/stat.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <pwd.h>
#include <grp.h>
#include <setjmp.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdarg.h>

/*
   ** access.c
 */
int parsetime(char *);
int validtime(char *);
int hostmatch(char *, char *, char *);
int acl_guestgroup(struct passwd *);
int acl_realgroup(struct passwd *);
void acl_autogroup(struct passwd *);
void acl_setfunctions(void);
int acl_getclass(char *);
int acl_getlimit(char *, char *);
int acl_getnice(char *);
void acl_getdefumask(char *);
void acl_tcpwindow(char *);
void acl_filelimit(char *);
void acl_datalimit(char *);
int acl_deny(char *);
int acl_countusers(char *);
void acl_join(char *);
void acl_remove(void);
void pr_mesg(int, char *);
void access_init(void);
int access_ok(int);

/*
   ** acl.c
 */
struct aclmember *getaclentry(char *, struct aclmember **);
void parseacl(void);
int readacl(char *);

/*
   ** auth.c
 */
#ifdef  BSD_AUTH
char *start_auth(char *, char *, struct passwd *);
#endif
char *check_auth(char *, char *);

/*
   ** authenticate.c
 */
int wu_authenticate(void);

/*
   ** conversions.c
 */
void conv_init(void);

/*
   ** extensions.c
 */
int check_newer(char *, struct stat *, int);
long getSize(char *);
void msg_massage(const char *, char *, size_t);
int cwd_beenhere(int);
void show_banner(int);
void show_message(int, int);
void show_readme(int, int);
int deny_badasciixfer(int, char *);
int is_shutdown(int, int);
void newer(char *date, char *path, int showlots);
int type_match(char *typelist);
int path_compare(char *p1, char *p2);
void expand_id(void);
int fn_check(char *name);
int dir_check(char *name, uid_t * uid, gid_t * gid, int *d_mode, int *valid);
int upl_check(char *name, uid_t * uid, gid_t * gid, int *f_mode, int *valid);
int del_check(char *name);
int regexmatch(char *name, char *rgexp);
int checknoretrieve(char *name);
int path_to_device(char *pathname, char *result);
void get_quota(char *fs, int uid);
char *time_quota(long curstate, long softlimit, long timelimit, char *timeleft);
void fmttime(char *buf, register long time);
int file_compare(char *patterns, char *file);
int remote_compare(char *patterns);
void throughput_calc(char *name, int *bps, double *bpsmult);
void throughput_adjust(char *name);
void SetCheckMethod(const char *method);
void ShowCheckMethod(void);
void CheckSum(char *pathname);
void CheckSumLastFile(void);

/*
   ** ftpcmd.c
 */
char *wu_getline(char *s, int n, register FILE *iop);
int yyparse(void);
void upper(char *s);
char *copy(char *s);
void sizecmd(char *filename);
void site_exec(char *cmd);
void alias(char *s);
void cdpath(void);
void print_groups(void);

/*
   ** ftpd.c
 */

SIGNAL_TYPE randomsig(int sig);
SIGNAL_TYPE lostconn(int sig);
char *mapping_getwd(char *path);
void do_elem(char *dir);
int mapping_chdir(char *orig_path);
char *sgetsave(char *s);
struct passwd *sgetpwnam(char *name);
char *skey_challenge(char *name, struct passwd *pwd, int pwok);
void user(char *name);
int checkuser(char *name);
int denieduid(uid_t uid);
int alloweduid(uid_t uid);
int deniedgid(gid_t gid);
int allowedgid(gid_t gid);
void end_login(void);
int validate_eaddr(char *eaddr);
void pass(char *passwd);
int restricteduid(uid_t uid);
int unrestricteduid(uid_t uid);
int restrictedgid(gid_t gid);
int unrestrictedgid(gid_t gid);
char *opt_string(int options);
void retrieve(char *cmd, char *name);
void store(char *name, char *mode, int unique);
FILE *getdatasock(char *mode);
FILE *dataconn(char *name, off_t size, char *mode);
#ifdef THROUGHPUT
int send_data(char *name, FILE *instr, FILE *outstr, off_t blksize);
#else
int send_data(FILE *instr, FILE *outstr, off_t blksize);
#endif
int receive_data(FILE *instr, FILE *outstr);
void statfilecmd(char *filename);
void statcmd(void);
void fatal(char *s);
void vreply(long flags, int n, char *fmt, va_list ap);
void reply(int, char *fmt,...);
void lreply(int, char *fmt,...);
void ack(char *s);
void nack(char *s);
void yyerror(char *s);
void delete(char *name);
void cwd(char *path);
void makedir(char *name);
void removedir(char *name);
void pwd(void);
char *renamefrom(char *name);
void renamecmd(char *from, char *to);
void dologout(int status);
SIGNAL_TYPE myoob(int sig);
void passive(void);
char *gunique(char *local);
void perror_reply(int code, char *string);
void send_file_list(char *whichfiles);
void initsetproctitle(int argc, char **argv, char **envp);
void setproctitle(const char *fmt,...);
void init_krb(void);
void end_krb(void);
void do_daemon(int argc, char **argv, char **envp);

#ifdef INTERNAL_LS
char *rpad(char *s, unsigned int len);
char *ls_file(const char *file, int nameonly, char remove_path, char classify);
void ls_dir(char *d, char ls_a, char ls_F, char ls_l, char ls_R, char omit_total, FILE *out);
void ls(char *file, char nlst);
#endif

void fixpath(char *path);

/*
   ** glob.c
 */
void blkfree(char **);
char **ftpglob(register char *);

/*
   ** hostacc.c
 */
int rhost_ok(char *pcRuser, char *pcRhost, char *pcRaddr);

/*
   ** loadavg.c
 */
/*
   ** logwtmp.c
 */
void wu_logwtmp(char *line, char *name, char *host, int login);

/*
   ** paths.c
 */
void setup_paths(void);

/*
   ** popen.c
 */
FILE *ftpd_popen(char *program, char *type, int closestderr);
int ftpd_pclose(FILE *iop);

/*
   ** private.c
 */
#ifndef NO_PRIVATE
void parsepriv(void);
void priv_setup(char *path);
void priv_group(char *group);
void priv_gpass(char *gpass);

#endif

/*
   ** rdservers.c
 */
/*
   ** realpath.c
 */
char *fb_realpath(const char *path, char *resolved);
char *wu_realpath(const char *path, char *resolved_path, char *chroot_path);

/*
   ** restrict.c
 */
int restrict_check(char *name);
int test_restriction(char *name);
int restrict_list_check(char *name);

/*
   ** routevector.c
 */
int routevector(void);
void checkports(void);

/*
   ** sigfix.c
 */
int enable_signaling(void);
int delay_signaling(void);

/*
   ** timeout.c
 */
void load_timeouts(void);

/*
   ** support/getusershell.c
 */
char *getusershell(void);
void endusershell(void);

/*
   ** support/strcasestr.c
 */
char *strcasestr(register char *s, register char *find);
