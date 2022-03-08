/**************************************************************************/
/*                                                              /\/\      */
/*                                                              \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam, Casper Dik          _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Dept. of Computer Systems                        | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*                                                          | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/*                                                              \/\/      */
/* This file is copyright protected.                                      */
/* Copyright (c) 1991-2022 Universiteit van Amsterdam                     */
/* This software or any part thereof may only be used for non-commercial  */
/* research or purposes, as long as the author and University are         */
/* mentioned. Commercial use without explicit prior written consent by    */
/* the Universiteit van Amsterdam is strictly prohibited. This copyright  */
/* notice must be included with any copy of this software or any part     */
/* thereof.                                                               */
/*                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/**************************************************************************/
/*
 * $Id$
 *
 * The RTX daemon.
 *
 * It provides unprivileged users with access to the program running
 * on the IPC.
 *
 * It checks/downloads/restarts the IPC, if this is appropriate.
 *
 * arnoud, Dec 2003
 * Converted the old BSD4.3 access-rights
 *
 * Release 20: htons of in.sin_port for linux-daemon
 *
 */

#include <sys/types.h>
#include <sys/uio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/fcntl.h>
#include <sys/file.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/un.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <malloc.h>
#include <stdarg.h>
#include <pwd.h>
#include <stdlib.h>
#include <netdb.h>
#include <time.h>
#include <fcntl.h>

#ifndef SVR4
#include <termios.h>
#else
#include <termio.h>
#endif

#include "comm.h"
#include "rtxd.h"
#include "rtx.h"
#include "rtxcmds.h"
#include "armlib.h"
#include "vrtx.h"
#include "ipcio.h"

int maxidle = 300;
int maxtime = 900;

client *clients = 0, *newclient();
FILE *logfile = (FILE *)2; /* stderr */

static struct stat privbuf;
static int priv_enabled;
static rtx_t *rtx;

static int errexit(char *s,int stat);
static void sweepclients();
static void onetransaction(client *cp);

extern int rtxdebug;

int
main(argc, argv)
int argc;
char **argv;
{
    int i, lockfd, s_un, s_in;
    FILE *portinfo;
    struct sockaddr_un un;
    struct sockaddr_in in;
    int isize;
    struct timeval timeo;
    int maxfd, oldmaxfd = 0;
    char hostname[MAXHOSTNAMELEN];
    char cwd[PATH_MAX];
    int one = 1;
    char *which;
    struct flock lock;
#ifndef SVR4
    int ttfd;
#endif
#ifndef NOLOCKING
    char tmp[10];
#endif

    (void) umask(002);

    /* Select the RTX controller */
    switch (argc) {
    case 2:
	which = argv[1];
	break;
    case 1:
	which = "IPC";
	break;
    default:
	fprintf(stderr,"Usage: %s [type]\n", argv[0]);
	exit(1);
    }

    /* close all files */
    for (i = 3; i < NOFILE; i++)
	close(i);

    if (chdir(RTXD_DIR) != 0) errexit(RTXD_DIR,1);
    /*
     * Am I the one and only daemon.
     *
    if(access(strcat(RTXD_DIR,RTXD_LOCK), F_OK )){
        printf("The File %s\t was not Found\n",strcat(RTXD_DIR,RTXD_LOCK));
    }
    if(access(strcat(RTXD_DIR,RTXD_LOCK), R_OK )){
        printf("The File %s\t cannot be read\n",strcat(RTXD_DIR,RTXD_LOCK));
    }
    lockfd = open(strcat(RTXD_DIR,RTXD_LOCK),O_RDWR|O_CREAT,0660);
    */
    if(getcwd(cwd, sizeof(cwd) ) != NULL){
        printf("Current working dir %s\n",cwd);
    }
    if(access(RTXD_LOCK, F_OK )){
        printf("The File %s\t was not Found\n",RTXD_LOCK);
    }
    lockfd = open(RTXD_LOCK,O_RDWR|O_CREAT,0660);
    if (lockfd == -1) errexit(RTXD_LOCK,1);

    fchmod(lockfd,0660);
    priv_enabled = fstat(lockfd,&privbuf) != -1 &&
			(S_IRWXO & privbuf.st_mode) == 0;

    /*
     * Daemonize goes here. So we can give a meaningfull exit status.
     */
#ifndef NOFORK
#ifdef SVR4
    setsid();
#else
    ttfd = open("/dev/tty",O_RDWR);
    ioctl(ttfd,TIOCNOTTY,0);
    close(ttfd);
#if 1
    /* New programs should use 'setpgid' to be POSIX-compatible */
    setpgid(0,0);
#else
    /* Both System V en BSD have setpgrp function, but with different
       calling conventions. */
#ifndef __FAVOR_BSD
    setpgrp(); /* no arguments according to /usr/include/unistd.h */
#else
    setpgrp(0,0);
#endif /* __FAVOR_BSD */
#endif /* new */
#endif /* SVR4 */
    close(0); close(1);
    (void) open("/dev/null",O_RDWR);
    (void) open("/dev/null",O_RDWR);
    switch(fork()) {
    case -1: errexit("fork",1);
    case 0: break;
    default: _exit(0);
    }
#endif /* !NOFORK */
    /* must lock after fork() */
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    lock.l_type = F_WRLCK;
#ifndef NOLOCKING
    if (fcntl(lockfd, F_SETLK, &lock) == -1)
#ifdef SVR4
	if (errno == EAGAIN)
#else
	if (errno == EACCES || errno ==EAGAIN)
#endif
	    exit(1);
	else
	    errexit("lock",1);
#endif


    s_un = socket(PF_UNIX,SOCK_STREAM,0);
    if (s_un == -1) errexit("unix socket",1);
    s_in = socket(PF_INET,SOCK_STREAM,0);
    if (s_in == -1) errexit("in socket",1);

    if (unlink(RTXD_SOCKET) == -1 && errno == EPERM) errexit("unlink",1);

    strcpy(un.sun_path,RTXD_SOCKET);
    un.sun_family = PF_UNIX;

    if (bind(s_un,(struct sockaddr*) &un,strlen(RTXD_SOCKET) + sizeof(un.sun_family)) == -1)
	errexit("bind unix socket",1);

    if (setsockopt(s_in,SOL_SOCKET,SO_REUSEADDR,(const void*)&one,sizeof(one)) == -1)
	perror ("inet setsockopt REUSEADDR");

    if (s_un >= oldmaxfd) oldmaxfd = s_un+1;
    if (s_in >= oldmaxfd) oldmaxfd = s_in+1;

    if (listen(s_un,5) == -1) errexit("unix listen",1);
    if (listen(s_in,5) == -1) errexit("inet listen",1);

    /* not critical - shouldn't setsockopt take a void * as arg #4? */
    if (setsockopt(s_un,SOL_SOCKET,SO_KEEPALIVE,(char*)&one,sizeof(one)) == -1)
	perror ("unix setsockopt");
    if (setsockopt(s_in,SOL_SOCKET,SO_KEEPALIVE,(char*)&one,sizeof(one)) == -1)
	perror ("inet setsockopt KEEPALIVE");

    isize = sizeof(in);
    if (getsockname(s_in, (struct sockaddr *) &in, (socklen_t *)&isize) != 0)
	errexit("getsockname",1);

    if (gethostname(hostname,MAXHOSTNAMELEN) != 0)
	errexit("gethostname",1);

    in.sin_port = htons(in.sin_port);
    portinfo = fopen(RTXD_PORTINFO,"w");
    if (!portinfo) errexit(RTXD_PORTINFO,1);
    fchmod(fileno(portinfo),0664);
    fprintf(portinfo,"%s\n",hostname);
    fprintf(portinfo,"%d\n",in.sin_port);
    fclose(portinfo);

    logfile = freopen(RTXD_LOG,"a",stderr);

    if (!logfile)
	exit(2);

    fchmod(fileno(logfile),0664);
#ifdef SVR4
    setvbuf(logfile, 0, _IOLBF, 0);
#else
    setlinebuf(logfile);
#endif
    rtx_log((client *) 0,"Restarting, listening to port %d on %s",
       in.sin_port, hostname);
#ifndef NOLOCKING
    ftruncate(lockfd,0);
    sprintf(tmp,"%d\n",getpid());
    write(lockfd,tmp,strlen(tmp));
#endif

    if ((rtx = rtx_probe(which)) == 0) {
	rtx_log((client *) 0,"Can not talk to RTX");
	exit(1);
    }

    timeo.tv_sec = 60;
    timeo.tv_usec = 0;
    (void) signal(SIGPIPE,SIG_IGN);
    while(1) {
	int cnt;
	fd_set readfds;
	client *cp;

	maxfd = oldmaxfd;

	FD_ZERO(&readfds);
	FD_SET(s_un,&readfds);
	FD_SET(s_in,&readfds);
	for (cp = clients; cp ; cp = cp->c_next) {
	    FD_SET(cp->c_fd,&readfds);
	    if (maxfd <= cp->c_fd)
		maxfd = cp->c_fd + 1;
	}

	cnt = select(maxfd,&readfds,(fd_set *) 0,(fd_set *) 0, &timeo);

	if (timeo.tv_sec != 60) {
		/* some selects change the struct timeo to indicate 
		 * the time they have waited. On Linux they apparently
		 * do, on Solaris not */
    		timeo.tv_sec = 60;
	}
	sweepclients();

	if (cnt == -1)
	    continue;
	if (cnt == 0) {
	    if (rtx_test(rtx))
		rtx_log((client *) 0,"RTX died - reinited");
	    continue;
	}
	if (cnt && FD_ISSET(s_un,&readfds)) {
	    struct sockaddr_un aun;
	    int news, len = sizeof(aun);

	    news = accept(s_un,(struct sockaddr*) &aun,(socklen_t *)&len);
	    (void) newclient(news,0,(char *) &aun,len);
	    cnt--;
	}
	if (cnt && FD_ISSET(s_in,&readfds)) {
	    struct sockaddr_in ain;
	    int news, len = sizeof(ain);

	    news = accept(s_in,(struct sockaddr*) &ain,(socklen_t *)&len);
	    (void) newclient(news,CLNT_INET,(char *) &ain,len);
	    cnt--;
	}
	for (cp = clients;cp && cnt; cp = cp->c_next) {
	    if (!FD_ISSET(cp->c_fd,&readfds))
		continue;
	    onetransaction(cp);
	    cnt --;
	}
	if (cnt)
	    rtx_log((client *) 0,"Select, unexpected fd/event");
    }
}

client *getclient(s)
int s;
{
    client *cp = clients;
    while (cp && cp->c_fd != s)
	cp = cp->c_next;

    return cp;
}

/*
 * If a client has been idle too long, or has been working a long time,
 * mark it for deletion.
 * It will be marked read-only when a new connection is opened.
 */
static void
sweepclients()
{
    time_t t = time ((time_t *) 0);
    client *cp = clients;

    for (;cp;cp = cp->c_next) {
	if (cp->c_flags & CLNT_PRIV)
	    continue;
	if (t - cp->c_last > maxidle && !(cp->c_flags & CLNT_IDLED)) {
	    cp->c_flags |= CLNT_IDLED;
	    rtx_log(cp,"idle too long");
	}
	if (t - cp->c_first > maxtime && !(cp->c_flags & CLNT_TIMEO)) {
	    cp->c_flags |= CLNT_TIMEO;
	    rtx_log(cp,"working too long");
	}
    }
}

client *
newclient(s,flags,socket,len)
int s,flags,len;
char *socket;
{
    client *p,*cp = (client *) malloc(sizeof(struct client));
    int r = 0;
    char *ctime();

    sweepclients();
    for (p = clients; p; p=p->c_next) {
	if (p->c_flags & (CLNT_IDLED|CLNT_TIMEO))
	    continue;

	if (!(p->c_flags & CLNT_RDONLY))
	    r = CLNT_RDONLY;
    }
    cp->c_next = clients;
    cp->c_fd = s;
    cp->c_last = cp->c_first = time((time_t *) 0);
    rtx_log(cp,"New Connection (read%s)",r ? "only" : "/write");
    memcpy((char *)&cp->c_un,socket,len);
    cp->c_flags = r | flags | CLNT_NOT_INITED;
    clients = cp;
    return cp;
}

void
doneclient(oldcp)
client *oldcp;
{
    client *cp = clients, *p = 0;
    time_t t;
    char *ctime();

    t = time((time_t *) 0);
    rtx_log(oldcp,"Closed");
    close(oldcp->c_fd);
    while (cp && cp != oldcp) {
	p = cp;
	cp = cp->c_next;
    }
    if (!cp) {
	rtx_log((client *) 0,"OOPS: unknown client disposed of -- not freed");
	return;	/* OOPS */
    }
    if (p)
	p->c_next = cp->c_next;
    else
	clients = cp->c_next;
    free((char *) oldcp);
}

#if !defined(_KERNEL)
/* this is the msghdr as used by sendmsg an recvmsg
   for older XOPEN versions (< 4.1) */
struct omsghdr {
    caddr_t msg_name;
    int msg_namelen;
    struct iovec *msg_iov; /* scatter/gather array */
    int msg_iovlen;
    caddr_t msg_accrights;
    int msg_accrightslen;
};
#endif /* !defined (_KERNEL) */

static struct libio theio;
static struct iovec thevec = { (caddr_t) &theio, sizeof(theio) };
static int theright;
#if defined (SVR4)
/*
#if defined(_XPG4_2)
 *  * linux/socket.h says:
 *   **      As we do 4.4BSD message passing we use a 4.4BSD message passing
 *    **      system, not 4.3. Thus msg_accrights(len) are now missing. They
 *     **      belong in an obscure libc emulation or the bin.
 *      */

static struct msghdr themsg = 
	{ 0, 0, &thevec, 1, (caddr_t) &theright, sizeof(int)};
#else
static struct msghdr themsg = 
	{ 0, 0, &thevec, 1, (caddr_t) &theright, sizeof(int), 0};
#endif

/*
 * Do one transaction with a client.
 * Care must be taken that rogue sends (by students) do not
 * result in filedescriptors gathering the daemon
 */
static void
onetransaction(cp)
client *cp;
{
    int doquit = 0, doabort = 0;
    int cc, i;
    struct passwd *pwd, *getpwuid();
    client *c;

    thevec.iov_len = sizeof(theio);
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined(_XOPEN_UNIX))
    themsg.msg_controllen = sizeof(int);
#else
    themsg.msg_accrightslen = sizeof(int);
#endif
    cc = recvmsg(cp->c_fd,(struct msghdr *)&themsg,0);
    if (cc != sizeof(struct libio)) {
        rtx_log(cp,"Unknown size msg, %d instead of %d", cc, sizeof(struct libio));
	doneclient(cp);
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined(_XOPEN_UNIX))
	if (cc != -1 && themsg.msg_controllen == sizeof(int))
	    close(theright);
	return;
#else
	if (cc != -1 && themsg.msg_accrightslen == sizeof(int))
	    close(theright);
	return;
#endif
    }
    theio.what = ntohl (theio.what);
    theio.params[0] = ntohl (theio.params[0]);
    theio.params[1] = ntohl (theio.params[1]);
    theio.params[2] = ntohl (theio.params[2]);

    if (!!(cp->c_flags & CLNT_NOT_INITED) ^ (theio.what == RTX_CMD_FIRST)) {
        rtx_log(cp,"Client not inited");
	doneclient(cp);
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined(_XOPEN_UNIX))
	if (themsg.msg_controllen == sizeof(int))
	    close(theright);
	return;
#else
	if (themsg.msg_accrightslen == sizeof(int))
	    close(theright);
	return;
#endif
    }

    if (rtxdebug > 1)
        rtx_log(cp, "onetransaction: %s", rtxstrcommand(theio.what));
    switch (theio.what) {
    case RTX_CMD_FIRST:
	cp->c_flags &= ~CLNT_NOT_INITED;
	theio.what = RTX_RESP_ACK;
	cp->c_pid = theio.params[0];
	if (theio.params[1])
	    cp->c_flags |= CLNT_RDONLY;
	else if (cp->c_flags & CLNT_RDONLY)
	    theio.what = ARM_IN_USE;
	else {
	    for (c = clients; c ; c = c->c_next)
		if (c->c_flags & (CLNT_IDLED|CLNT_TIMEO))
		    c->c_flags |= CLNT_RDONLY;

	}

	cp->c_uid = theio.params[2];

	if (rtxdebug > 2)
           rtx_log(cp, "%s: pid %d rw %d uid %d",
		rtxstrcommand(theio.what), cp->c_pid, theio.params[1], cp->c_uid);

	pwd = getpwuid(cp->c_uid);
	if (!pwd) {
	    rtx_log(cp,"Unknown user %d (expected %d)", cp->c_uid, getuid());
	    doquit = 1;
	    theio.what = PRIVILEGE_VIOLATION;
	    break;
	}
	rtx_log(cp,"New client (read%s), user %s",
		(cp->c_flags & CLNT_RDONLY) ? "only" : "/write", pwd->pw_name);
	cp->c_last = time((time_t *) 0);
	break;
    case RTX_CMD_QUIT:
	doquit = 1;
	theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_ABORT:
	if (cp->c_flags & CLNT_PRIV) {
	    doabort = 1;
	    theio.what = RTX_RESP_ACK;
	} else
	    theio.what = PRIVILEGE_VIOLATION;
	break;
    case RTX_CMD_GET_NCON:
	i = 0;
	c = clients;
	while (c) {
	    i ++;
	    c = c->c_next;
	}
	theio.what = RTX_RESP_ACK;
	theio.params[0] = i;
	break;
    case RTX_CMD_GET_CON:
	{
	    time_t t;
	    i = theio.params[0];
	    c = clients;

	    (void) time(&t);
	    while (c && i) {
		i --;
		c = c->c_next;
	    }
	    if (!c) {
		theio.what = SELECTION_OOR;
		break;
	    }
	    theio.what = RTX_RESP_ACK;
	    theio.params[0] = c->c_fd;
	    theio.params[1] = c->c_uid;
	    theio.params[2] = (c->c_flags & CLNT_INET) ? 
			c->c_in.sin_addr.s_addr : -1;
	    theio.params[3] = t - c->c_first;
	    theio.params[4] = t - c->c_last;
	    theio.params[5] = c->c_flags;
	}
	break;
    case RTX_CMD_CLOSE_CON:
	i = theio.params[0];
        rtx_log(cp, "%s: fd %d",
		rtxstrcommand(theio.what), i);
	c = clients;
	while (c && c->c_fd != i)
	    c = c->c_next;
	if (!c) {
	    theio.what = SELECTION_OOR;
	    break;
	}
	if (c == cp) {
	    doquit = 1;
	} else if (cp->c_flags & CLNT_PRIV) {
            rtx_log(cp,"RTX command: close connection");
	    doneclient(c);
	} else {
	    theio.what = PRIVILEGE_VIOLATION;
	    break;
	}
	theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_PRIV:
	{
	    struct stat buf;
	    if (!priv_enabled ||
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined(_XOPEN_UNIX))
		    themsg.msg_controllen != sizeof(int) ||
#else
		    themsg.msg_accrightslen != sizeof(int) ||
#endif
		    fstat(theright,&buf) == -1 ||
		    buf.st_ino != privbuf.st_ino ||
		    buf.st_dev != privbuf.st_dev ||
		    (buf.st_mode & S_IRWXO) != 0) {
		theio.what = PRIVILEGE_VIOLATION;
		rtx_log(cp,"PRIVILEGE VIOLATION");
	    } else {
		theio.what = RTX_RESP_ACK;
		cp->c_flags |= CLNT_PRIV;
		cp->c_flags &= ~(CLNT_RDONLY|CLNT_IDLED|CLNT_TIMEO);
		rtx_log(cp,"Privilege turned on");
		cp->c_last = time((time_t *) 0);
	    }
	}
	break;
    case RTX_CMD_NO_PRIV:
	cp->c_flags &= ~CLNT_PRIV;
	theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_READ_WRITE:
	if (cp->c_flags & CLNT_RDONLY) {
	    int flag = ~CLNT_RDONLY;
	    theio.what = RTX_RESP_ACK;
	    for (c = clients; c; c = c->c_next) {
		if (!(c->c_flags & CLNT_RDONLY)) {
		    flag = ~0;
		    theio.what = ARM_IN_USE;
		    break;
		}
	    }
	    cp->c_flags &= flag;
	} else 
	    theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_READ_ONLY:
	if (!(cp->c_flags & CLNT_PRIV))
	    cp->c_flags |= CLNT_RDONLY;
	theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_GET_PARAMS:
	theio.params[0] = maxidle;
	theio.params[1] = maxtime;
	theio.what = RTX_RESP_ACK;
	break;
    case RTX_CMD_SET_PARAMS:
	if (!(cp->c_flags & CLNT_PRIV))
	    theio.what = PRIVILEGE_VIOLATION;
	else {
	    maxidle = theio.params[0];
	    maxtime = theio.params[1];
            rtx_log(cp, "%s: maxidle %d maxtime %d",
		rtxstrcommand(theio.what), maxidle, maxtime);
	    theio.what = RTX_RESP_ACK;
	}
	break;
    /* privileged */
    case RTX_CMD_RAW_CMD:
    case RTX_CMD_RAW_RESP:
    case RTX_CMD_RAW:
	if (!(cp->c_flags & CLNT_PRIV)) {
	    theio.what = PRIVILEGE_VIOLATION;
	    break;
	}
	/* FALL THROUGH */
    /* read / write */
    case RTX_CMD_RESTART:
    case RTX_CMD_DEFINE_HOME:
    case RTX_CMD_RELOAD_PIDS:
    case RTX_CMD_SET_MODE:
    case RTX_CMD_STOP:
    case RTX_CMD_GO:
    case RTX_CMD_WRITE:
    case RTX_CMD_INTERPOLATE:
    case RTX_CMD_SOAK:
	if (cp->c_flags & CLNT_RDONLY)
	    theio.what = CONNECTION_READ_ONLY;
	else
	    rtx_command(rtx, &theio);
	cp->c_last = time((time_t *) 0);
	break;
    /* assume already inited */
    case RTX_CMD_INIT:
	if (!(cp->c_flags & CLNT_RDONLY))
	    rtx_command(rtx, &theio);
	else
	    theio.what = RTX_RESP_ACK;
	break;
    /* read only */
    case RTX_CMD_VERSION:
    case RTX_CMD_MOTOR_STATUS:
    case RTX_CMD_GENERAL_STATUS:
    case RTX_CMD_READ:
	rtx_command(rtx, &theio);
	break;
    default:
	theio.what = COMMAND_OOR;
	break;
    }
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined(_XOPEN_UNIX))
    if (themsg.msg_controllen == sizeof(int))
    {
        rtx_log(cp,"closing the right");
	close(theright);
    }
#else
    if (themsg.msg_accrightslen == sizeof(int))
	close(theright);
#endif
#if 1
    theio.what = htonl (theio.what);
    theio.params[0] = htonl (theio.params[0]);
    theio.params[1] = htonl (theio.params[1]);
    theio.params[2] = htonl (theio.params[2]);

    if (send(cp->c_fd,(void *) &theio, sizeof(theio),0) != sizeof(theio)) {
#else
    if (rtxdebug > 2)
       rtx_log(cp,"sending acknowledgement of size %d", sizeof(theio));
    
    if (sendmsg(cp->c_fd,(void *) &theio, sizeof(theio),0) != sizeof(theio)) {
#endif
        rtx_log(cp,"couldn't sent acknowledgement");
	doneclient(cp);
	return;
    }
    if (doabort) {
	rtx_log(cp,"CPU halted - daemon aborted");
	stopcpu();
	exit(0);
    }
    if (doquit) {
        rtx_log(cp,"Do quit");
	doneclient(cp);
    }
}

static int
errexit(s,stat)
char *s;
int stat;
{
    perror(s);
    exit(stat);
}

/* VARARGS */
void
rtx_log(client *clnt, ...)
{
    va_list args;
    char *fmt;
    time_t t;
    char tbuf[32];
    struct tm *localtime();

    if (!logfile)
	return;

    va_start(args, clnt);
    fmt = va_arg(args,char *);
    (void) time(&t);
    strftime(tbuf,32,"%F %T",localtime(&t));
    if (clnt)
	fprintf(logfile,"%s (%d) ",tbuf, clnt->c_fd);
    else
	fprintf(logfile,"%s (RTXD) ", tbuf);
    vfprintf(logfile,fmt,args);
    putc('\n',logfile);
    va_end(args);
}
