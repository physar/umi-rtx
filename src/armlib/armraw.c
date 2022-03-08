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
 * The raw interface to the RTX daemon over the network.
 *
 * Release 1.10: enable change of debug-level when library-loaded
 * Release 1.8: htonl commands moved to _arm_raw_trans, for all params and com
 */
#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/fcntl.h>
#include <sys/un.h>
#include <sys/uio.h>
#include <sys/file.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <fcntl.h>

#include <rtxd.h>
#include <rtx.h>
#include <comm.h>
#include <signal.h>
#include "armraw.h"
#include "comm.h"



static int debug = 0;
static int library_loaded = 0;
static int lib_socket = -1;
static int sock_fam;

static int _arm_raw_transaction(libio *cmd);
static char *rtxstrcommand(int what);

int armerrno = 0;
int _arm_read_only = 0;

/* ARGSUSED */
#ifdef SVR4
static void freertx(void)
#else
static void
freertx(status,arg)
int status;
char *arg;
#endif
{
    if (lib_socket != -1) {
	(void) arm_interrupt(RTX_CMD_QUIT,(libio *) 0);
	close(lib_socket);
    }
}

/*
 * Try to open a connection to the (possibly) remote host.
 * If the there is no host in the portinfo file, if it is not possible to
 * open a conenction, or if the portinfo file seems corrupted, it is unlinked.
 * This is, of course, only possible if you have the permission to do that.
 */

static int
downloadlib()
{
    char hostname[MAXHOSTNAMELEN];
    char buf[1024];
    libio tmpio;
    int pid;
    FILE *pinfo;
    int port;

    if (library_loaded)
	return 0;

    /*
     * The default errno
     */
    armerrno = NO_CONNECTION;
    if (gethostname(hostname,MAXHOSTNAMELEN) != 0) {
	perror("gethostname failed");
	goto fatal;
    }

    if (chdir(RTXD_DIR) != 0) {
         perror("chdir failed");
         goto fatal;
    }
    pinfo = fopen(RTXD_PORTINFO,"r"); /* relative to RTX_DIR */

    if (pinfo)
	if (!fgets(buf,1024,pinfo)) {
            perror("downloadlib has problems reading portinfo");
	    fclose(pinfo); pinfo = 0;
	}
    
    buf[strlen(buf)-1] = '\0';
    if (!pinfo || strcmp(hostname,buf) == 0) {
	struct sockaddr_un un;

	if (pinfo) 
            fclose(pinfo);
	pid = getpid();
	strcpy(un.sun_path,RTXD_SOCKET);
	sock_fam = un.sun_family = PF_UNIX;
	lib_socket = socket(PF_UNIX,SOCK_STREAM,0);
	if (lib_socket == -1) {
	perror("downloadlib has problems opening UNIX socket");
	    goto fatal;
        }
	fcntl(lib_socket,F_SETFD,1);
	if (connect(lib_socket,(struct sockaddr *)&un,
		strlen(un.sun_path)+sizeof(un.sun_family)) == -1) {
#if 0
	    if (system(RTXD_PATH) != 0)
		goto fatal;
	    if (connect(lib_socket,(struct sockaddr *)&un,
		    strlen(un.sun_path)+sizeof(un.sun_family)) == -1)
		goto fatal;
#else
	perror("downloadlib has problems connecting to UNIX socket");
	    goto fatal;
#endif
	}
    } else {
	struct sockaddr_in in;
	struct hostent *gethostbyname(), *host = gethostbyname(buf);

	if (fscanf(pinfo,"%d",&port) != 1) {
	    fprintf(stderr,"No port number in `%s'.\n",RTXD_PORTINFO);
	    fclose(pinfo);
	    unlink(RTXD_PORTINFO);
	    goto fatal;
	}
	fclose(pinfo);
	pid = -1;
	if (!host) {
	    fprintf(stderr,"No such host: `%s'.\n",buf);
	    (void) unlink(RTXD_PORTINFO);
	    goto fatal;
	}
#if 1
	if (htons(port) != port && debug > 0)
	    printf("Connecting to daemon running on `%s':%d\n",buf,port); 
	/* htons changes integer to network byte order (big-endian) */
	in.sin_port = htons(port); /* not needed, converted via pinfo-file */
#else
        in.sin_port = port;
#endif
	printf("Connecting to daemon running on `%s':%d\n",buf,in.sin_port); 

	sock_fam = in.sin_family = PF_INET;
	memcpy((char*)&in.sin_addr,(char*)host->h_addr_list[0],host->h_length);
	lib_socket = socket(PF_INET,SOCK_STREAM,0);
	if (lib_socket == -1) {
	    perror("downloadlib has problems opening INET socket");
	    goto fatal;
        }
	if (connect(lib_socket,(struct sockaddr *)&in,sizeof(in)) == -1) {
		perror("downloadlib via internet socket");
	    fprintf(stderr,"Cannot connect to daemon\n"); 
	    fprintf(stderr,"Make sure it is running on `%s':%d\n",buf,port); 
	    /* unlink(RTXD_PORTINFO); */
	    goto fatal;
	}
    }
    tmpio.what = RTX_CMD_FIRST;
    /* arnoud expects: better in arm_raw_transaction ???? */
    tmpio.params[0] = pid;
    tmpio.params[1] = _arm_read_only;
    tmpio.params[2] = getuid(); /* htonl(getuid()) */
    library_loaded = 1;
    if (_arm_raw_transaction(&tmpio) == -1) {
	perror("downloadlib received no acknowledgment");
	goto fatal;
    }
    if (tmpio.what == RTX_RESP_ACK) {
	library_loaded = 1;
	armerrno = 0;
#ifdef SVR4
	atexit(freertx);
#else
	on_exit(freertx,(char *)0);
#endif
	return 0;
    }
    armerrno = tmpio.what;
fatal:
    /*
     * Lots of cleanup to do
     */
    fprintf(stderr,"[armlib:downloadlib] failed\n"); 
    library_loaded = 0;
    close(lib_socket);
    lib_socket = -1;
    return -1;
}

/*
 * Do one command with parameters.
 *
 * Set armerrno, if appropriate.
 */
int
arm_interrupt(cmd,rtxparams)
int cmd;
libio *rtxparams;
{
    libio tmpio;

    if ((armerrno == COMMS_NOT_INITIALISED || armerrno == LOST_CONNECTION) && cmd != RTX_CMD_INIT)
       return -1;

    if (!rtxparams)
	rtxparams = &tmpio;

    if (cmd == RTX_CMD_INIT) /* change debug also when library is loaded */
            debug = rtxparams->params[1];

    if (!library_loaded) {
	if (cmd == RTX_CMD_INIT) {
	    if (downloadlib() == -1) {
		return -1;
	    }
	} else {
	    armerrno = COMMS_NOT_INITIALISED;
	    return -1;
	}
    }
    rtxparams->what = cmd;

    if (debug > 3)
        printf( "arm_interrupt: %s\n", rtxstrcommand(rtxparams->what));

    if (_arm_raw_transaction(rtxparams) == -1)
	return -1;

    /* preserve errno */
    if (rtxparams->what) {
	armerrno = rtxparams->what;
	return -1;
    } else
	return 0;
}

/*
 * Do one transaction.
 * Only returns an error on a comunnications failure.
 * if the command is PRIV, send the authorisation info
 */
#if !defined(_KERNEL)
/* this is the msghdr as used by sendmsg and recvmsg
   for older XOPEN versions (< 4.1) */
struct omsghdr {
        caddr_t msg_name;               /* optional address */
        int     msg_namelen;            /* size of address */
        struct  iovec *msg_iov;         /* scatter/gather array */
        int     msg_iovlen;             /* # elements in msg_iov */
        caddr_t msg_accrights;          /* access rights sent/received */
        int     msg_accrightslen;
};
#endif  /* !defined(_KERNEL) */

static int
_arm_raw_transaction(cmd)
libio *cmd;
{
    void (*fun)();
    struct iovec thevec;
    int theright = -1;
    struct msghdr themsg;
    static int sending = 0;
    int msgSize, ackSize = -123;


    if (lib_socket == -1) {
	armerrno = COMMS_NOT_INITIALISED;
	return -1;
    }
    /* linux manual says:
     * struct msghdr {
     *  void         * msg_name;      * optional address *
     *  socklen_t    msg_namelen;     * size of address *
     *	struct iovec * msg_iov;       * scatter/gather array *
     *	size_t       msg_iovlen;      * # elements in msg_iov *
     *  void         * msg_control;   * ancillary data, see below *
     *  socklen_t    msg_controllen;  * ancillary data buffer len *
     *  int          msg_flags;       * flags on received message *
     *	};
     *	The prototypes given above follow the Single Unix Specifi-
     *	cation,  as glibc2 also does; the flags argument was `int'
     *	in BSD 4.*, but `unsigned int' in libc4 and libc5; the len
     *	argument  was  `int' in BSD 4.* and libc4, but `size_t' in
     *	libc5; the tolen argument was `int' in BSD 4.*  and  libc4
     *	and libc5.
     */

    themsg.msg_name = 0;
    themsg.msg_namelen = 0;
    themsg.msg_iov = &thevec;
    themsg.msg_iovlen = 1;

    cmd->what = htonl(cmd->what);
    cmd->params[0] = htonl(cmd->params[0]);
    cmd->params[1] = htonl(cmd->params[1]);
    cmd->params[2] = htonl(cmd->params[2]);
#if defined(_XPG4_2)
/*
 * application writers wishing to use any functions specified
 * as X/Open UNIX Extension must define _XOPEN_SOURCE and
 * _XOPEN_SOURCE_EXTENDED=1.  The Sun internal macro _XPG4_2
 * should not be used in its place as unexpected results may
 * occur.
 * 
 * linux/socket.h says:
 **      As we do 4.4BSD message passing we use a 4.4BSD message passing
 **      system, not 4.3. Thus msg_accrights(len) are now missing. They
 **      belong in an obscure libc emulation or the bin.
 */

    if (sock_fam == PF_UNIX && cmd->what == RTX_CMD_PRIV
       && (theright = open(RTXD_LOCK,O_RDWR)) != -1) {
	themsg.msg_accrights = (caddr_t) &theright;
	themsg.msg_accrightslen = sizeof(int);
    } else {
	themsg.msg_accrights = 0;
	themsg.msg_accrightslen = 0;
    }
    thevec.iov_base = (caddr_t) cmd;
    thevec.iov_len = sizeof(libio);
#else
    thevec.iov_base = (void *) cmd;
#if _XOPEN_VERSION > 4 || (_XOPEN_VERSION == 4 && defined (_XOPEN_UNIX))
	themsg.msg_control = 0;
	themsg.msg_controllen = 0;

    thevec.iov_len = sizeof(libio);
#else
    /* XOPEN without Unix extentions */
	themsg.msg_accrights = 0;
	themsg.msg_accrightslen = 0;

    thevec.iov_len = (size_t )sizeof(libio);
#endif
#endif
    fun = signal(SIGPIPE,SIG_IGN);
    /*
     * Protect this section of code
     * If this is interrupted and not restarted, we would be
     * in serious trouble. (Daemon and arm would loose sync)
     */
    if (sending) {
	int flags;
	flags = fcntl(lib_socket,F_GETFL);
	if (flags > 0 && fcntl(lib_socket,F_SETFL,flags & ~FNDELAY)) {
	    char buf[100];
	    while (read(lib_socket,buf,100) > 0)
                ;
            fcntl(lib_socket,F_SETFL,flags);
        }
    }

    sending = 1;
    if ((msgSize = sendmsg(lib_socket,(struct msghdr *)&themsg,0)) != sizeof(libio) ||
        (ackSize = recv(lib_socket,(char *)cmd, sizeof(libio),0)) != sizeof(libio)) {
	    if (msgSize != sizeof(libio) && debug > 0)
		fprintf(stderr, "arm_raw_transaction: send %d bytes, expected %ld bytes, errno %d\n", msgSize, sizeof(libio), errno);

	    if (ackSize != sizeof(libio) && debug > 0)
		fprintf(stderr,"arm_raw_transction: recv ack %d bytes, expected %ld bytes, errno %d\n", ackSize, sizeof(libio), errno);
	    signal(SIGPIPE,fun);
	    library_loaded = 0;
	    if (lib_socket != -1) {
		close(lib_socket);
		lib_socket = -1;
	    }
	    if (theright != -1)
		close(theright);
	    armerrno = LOST_CONNECTION;
	    sending = 0;
	    return -1;
    }
    cmd->what = ntohl(cmd->what);
    cmd->params[0] = ntohl(cmd->params[0]);
    cmd->params[1] = ntohl(cmd->params[1]);
    cmd->params[2] = ntohl(cmd->params[2]);
    sending = 0;
    (void) signal(SIGPIPE,fun);
    if (theright != -1)
	close(theright);
    return 0;
}
static char mess[80];

static char*
rtxstrcommand(what)
int what;
{

    switch(what) {
    case RTX_CMD_FIRST:
        return "RTX_CMD_FIRST";
    case RTX_CMD_QUIT:
        return "RTX_CMD_QUIT";
    case RTX_CMD_ABORT:
        return "RTX_CMD_ABORT";
    case RTX_CMD_GET_NCON:
        return "RTX_CMD_GET_NCON";
    case RTX_CMD_GET_CON:
        return "RTX_CMD_GET_CON";
    case RTX_CMD_CLOSE_CON:
        return "RTX_CMD_GET_CON";
    case RTX_CMD_PRIV:
        return "RTX_CMD_PRIV";
    case RTX_CMD_READ_WRITE:
        return "RTX_CMD_READ_WRITE";
    case RTX_CMD_READ_ONLY:
        return "RTX_CMD_READ_ONLY";
    case RTX_CMD_GET_PARAMS:
        return "RTX_CMD_GET_PARAMS";
    case RTX_CMD_SET_PARAMS:
        return "RTX_CMD_SET_PARAMS";
    case RTX_CMD_RAW_CMD:
    case RTX_CMD_RAW_RESP:
    case RTX_CMD_RAW:
        return "RTX_CMD_RAW";
    case RTX_CMD_RESTART:
        return "RTX_CMD_RESTART";
    case RTX_CMD_DEFINE_HOME:
        return "RTX_CMD_DEFINE_HOME";
    case RTX_CMD_RELOAD_PIDS:
        return "RTX_CMD_RELOAD_PIDS";
    case RTX_CMD_SET_MODE:
        return "RTX_CMD_SET_MODE";
    case RTX_CMD_STOP:
        return "RTX_CMD_STOP";
    case RTX_CMD_GO:
        return "RTX_CMD_GO";
    case RTX_CMD_WRITE:
        return "RTX_CMD_WRITE";
    case RTX_CMD_INTERPOLATE:
        return "RTX_CMD_INTERPOLATE";
    case RTX_CMD_SOAK:
        return "RTX_CMD_SOAK";
    case RTX_CMD_INIT:
        return "RTX_CMD_INIT";
    case RTX_CMD_VERSION:
    case RTX_CMD_MOTOR_STATUS:
    case RTX_CMD_GENERAL_STATUS:
    case RTX_CMD_READ:
        return "RTX_CMD_STATUS";
    default:
        sprintf (mess, "RTX-command %d unknown", what);
        return mess;
    }
}
