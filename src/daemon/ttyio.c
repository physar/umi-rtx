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
 * $Id: ttyio.c,v 1.7 2006/06/21 11:21:07 arnoud Exp $
 *
 * Control the robot through serial I/O.
 *
 * Release 1.6: added logs
 *
 */

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/types.h>
#ifndef __USE_BSD /*  4.3BSD things */
#ifdef LINUX
#include "sys/filio.h"
#else
#include <sys/filio.h>
#endif
#endif
#include <sys/time.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <netinet/in.h>
#include <string.h>

#include "comm.h"
#include "rtxcmds.h"
#include "rtx.h"
#include "rtxd.h"
#include "ttyio.h"
#include "vrtx.h"

/*
 * The tty arm operations through the rtx_ops interface
 */

static int tty_fd = -1;

static int current_ip;
static int toggle_mode;
static int inited = 0,initing = 0;

struct timeval lasttime;
#define gettime(tvp) gettimeofday((tvp), (struct timezone *)0)
#define msleep(i) usleep(1000*i)

/* debug */
static int ncmds = 0;
       int rtxdebug = 0;

static int rtx_resync(int err);

int rtxerror;

/* sleep until 8 milliseconds after lasttime */
static void
sleepltp8()
{
    struct timeval tz;

    gettime(&tz);
    if (tz.tv_sec - 1 <= lasttime.tv_sec) {
	long diff = (tz.tv_sec - lasttime.tv_sec) * 1000000;
	diff += (tz.tv_usec - lasttime.tv_usec);
	/* diff should be positive, not expecting timewarp, though */
	if (diff < 8000)
	    usleep(8000 - diff);
    }
}

static void
flush_input(fd)
int fd;
{
    char c;
    while (read(tty_fd, &c, 1) == 1)
	;
}

static void
read_from_rtx(len, resp, want)
int *len;
unsigned char *resp;
int want;
{
    int retry = 0;

    while (*len < want) {
	int cnt = read(tty_fd, &resp[*len], want - *len);
	if (cnt <= 0) {
	    if (retry)
		break;
	    retry = 1;
	    continue;
	}
	*len += cnt;
    }
}

/*
 * Start the communication with the robot.
 * Send the first commands.
 */
static int nreads = 0, nid = 0, svsim = 0, errresp = 0;
static int lasterr = 0, errcmd = 0, resync = 0;

static int
rtx_init_comms(tmode,debug)
int tmode,debug;
{
    int len,ip;
    unsigned char resp[3];

    rtxdebug = debug;

    initing = 1; inited = 0;
    gettime(&lasttime);

    flush_input(tty_fd);

    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1)
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1) {
            if (rtxdebug > 1)
                 rtx_log(NULL, "'%s' fails on CMD_TOGGLE_OFF", "rtx_init_comms");
	    return -1;
    }
    if (len != 1 || (resp[0] != RESP_ACK && resp[0] != RESP_IP_RESTART &&
		     resp[0] != RESP_IP_RESTART1)) {
	    rtxerror = COMMS_FAULT;
            if (rtxdebug > 1)
                 rtx_log(NULL, "'%s' CMD_TOGGLE_OFF gives wrong response", "rtx_init_comms");
	    return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1) {
        if (rtxdebug > 1)
            rtx_log(NULL, "'%s' fails on CMD_IDENTIFY", "rtx_init_comms");
	return -1;
    }

    if (len != 1) {
	rtxerror = RESPONSE_OVERRUN;
        if (rtxdebug > 1)
           rtx_log(NULL, "'%s' CMD_IDENTIFY gives multiple responses", "rtx_init_comms");
	return -1;
    }

    toggle_mode = 0;

    switch(resp[0]) {
    case RESP_IP_RESTART:
    case RESP_IP_RESTART1:
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,resp) == -1)
	    return -1;
	if (len != 1 || resp[0] != RESP_ACK) {
	    rtxerror = COMMS_FAULT;
	    return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1)
	    return -1;
	if (len != 1 ||
	    (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1)) {
		rtxerror = COMMS_FAULT;
		return -1;
	}
	/* FALL THROUGH */
    case RESP_IDENTIFY_0:
    case RESP_IDENTIFY_1:
	ip = resp[0] & RESP_IDENTIFY_MASK;
        if (rtxdebug > 2)
           rtx_log(NULL, "'%s' CMD_IDENTIFY gives correct response", "rtx_init_comms");
	break;
    default:
	rtxerror = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK) {
	rtxerror = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 || len != 1
	    || (resp[0] != RESP_ACK && resp[0] != RESP_IP_RESTART &&
		resp[0] != RESP_IP_RESTART1)) {
	rtxerror = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1) {
	rtxerror = COMMS_FAULT;
	return -1;
    }

    switch(resp[0]) {
    case RESP_IP_RESTART:
    case RESP_IP_RESTART1:
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,resp) == -1 || len != 1
		|| resp[0] != RESP_ACK) {
	    rtxerror = COMMS_FAULT;
	    return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		||
	    (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1)) {
		rtxerror = COMMS_FAULT;
		return -1;
	}
	/* FALL THROUGH */
    case RESP_IDENTIFY_0:
    case RESP_IDENTIFY_1:
	current_ip = resp[0] & RESP_IDENTIFY_MASK;
        if (rtxdebug > 2)
           rtx_log(NULL, "'%s' CMD_IDENTIFY gives 2nd correct response", "rtx_init_comms");
	break;
    default:
	rtxerror = COMMS_FAULT;
	return -1;
    }
    if (ip == current_ip) {
	rtxerror = COMMS_FAULT;
	return -1;
    }

    if (tmode) {
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK ||
	    rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK) {
		rtxerror = COMMS_FAULT;
        if (rtxdebug > 1)
           rtx_log(NULL, "'%s' CMD_TOGGLE_ON fails at the end", "rtx_init_comms");
		return -1;
	}
	toggle_mode = 1;
    }
    initing = 0;
    inited = 1;
    return 0;
}

/* based on MARIE/src/ad_server.c */
static void print_bits(char *annot, unsigned char mask)
{
/* Print high-order bits first */
    int i;
    char  iut[10];

    printf("\r");

    i = 8 * sizeof (unsigned char);
    iut[i] = '\0';
    while(i)
    {
        i--;
        iut[i] = (1 & (mask >> i)) ? ('0' + (char) i%10) : '.';
    }
    if (rtxdebug > 3)
       rtx_log(NULL, "%s %x  %s", iut, mask, annot);
}

/*
 * Arm raw. Does one command - response frame.
 * Execute cmd on ip, yielding resp.
 * Resp should have enough room for the longest possible response. 3 bytes
 */

int
rtx_raw (ip,cmdlen,b1,b2,b3,resplen,resp)
int ip, cmdlen, *resplen;
unsigned char b1,b2,b3, *resp;
{
    int ioerr = 0, len, expect;
    unsigned char cmd[3];

    if (rtxdebug > 3)
	rtx_log((client *) 0,"called rtx_raw(%d,%d,%u,%u,%u,%d,%u)",
			ip,cmdlen,b1,b2,b3,*resplen,*resp);

    if (!(inited ^ initing) ) {
        rtx_log((client *) 0,"rtx_raw fails on inited^initing (%d,%d)\n",
		inited,initing);
	rtxerror = COMMS_NOT_INITIALISED;
	return -1;
    }

    if (ip != IP0 && ip != IP1 && ip != IPDONTCARE) {
        rtx_log((client *) 0,"rtx_raw fails on ip (%d)\n",ip);
	rtxerror = ARM_SELECTION_OOR;
	return -1;
    }

    if (ip != IPDONTCARE && ip != current_ip) {
	unsigned char tmp[3];

        if (rtxdebug > 2)
            rtx_log((client *) 0,"rtx_raw expects another ip (%d) than current ip (%d)",ip, current_ip);

	if (toggle_mode) {
	    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,tmp) == -1)
		return -1;
	    if (tmp[0] != RESP_IDENTIFY_0 + (1-current_ip)) {
		if (rtx_resync(1) == -1)
		    return -1;
	    }
	} else {
	    current_ip = 1 - current_ip;
	    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,tmp) == -1)
		return -1;
	}
    }

    switch(cmdlen) {
    case 3:
        print_bits("rtx_raw: b3", b3);
        /* no break; */
    case 2:
        print_bits("rtx_raw: b2", b2);
        /* no break; */
    case 1:
        print_bits("rtx_raw: b1", b1);
        break;
    default:
	if (rtxdebug > 3)
            rtx_log(NULL, "rtx_raw: cmdlen %d",cmdlen);
    }
#if 0
    cmd[0] = htons(b1); /* nonsense, we already guarantee the right order */
    cmd[1] = htons(b2); /* by making a array of bytes. */
    cmd[2] = htons(b3); /* htonl(cmd) switches cmd[0] <-> cmd[4] */
#else
    cmd[0] = b1;
    cmd[1] = b2;
    cmd[2] = b3;
#endif

    if (cmd[0] != b1)
    {
        switch(cmdlen) {
        case 3:
            print_bits("rtx_raw: cmd[2]", cmd[2]);
            /* no break; */
        case 2:
            print_bits("rtx_raw: cmd[1]", cmd[1]);
            /* no break; */
        case 1:
            print_bits("rtx_raw: cmd[0]", cmd[0]);
            break;
        default:
	    if (rtxdebug > 3)
                rtx_log(NULL, "rtx_raw (big endian): cmdlen %d",cmdlen);
        }
    }

    /*
     * Under SunOS, the clock is usually accurate enough.
     */
    sleepltp8();
    gettime(&lasttime);
    /* should check errors here ? */
    (void) write(tty_fd, cmd, cmdlen);

    if (!initing && toggle_mode)
	current_ip = 1 - current_ip;

    /* msleep(8); */
    len = 0;
    read_from_rtx(&len, resp, 1);
#if 0
    {
    char x;

    for (len = 0; len < 3;) {
	int cnt = read(tty_fd, &resp[len], 3 - len);
	if (cnt <= 0)
	    break;
	len += cnt;
    }
    if (len == 3 && read(tty_fd, &x , 1) == 1) {
	rtx_resync(1);
#if 0
	printf("read too many: %02x\n", x & 0xff);
#endif
	rtxerror = RESPONSE_OVERRUN;
	return -1;
    }
    }
#endif


    if (len == 0) {
	rtx_resync(1);
	rtxerror = NO_RESPONSE;
	return -1;
    }

    print_bits("rtx_raw: resp", *resp);
#if 0
    *resp = ntohs(*resp);
    if (*resp != ntohs(*resp))
        print_bits("rtx_raw (big endian): resp", *resp);
#endif
    expect = 1;
    switch(*resp & RESP_TYPE_MASK) {
	case RESP_TYPE_SV_ID:
	    nid++;
	    if (b1 == CMD_VERSION) {
		read_from_rtx(&len, resp, 3);
		ioerr = len != 3;
	    } else
		ioerr = len != 1;
	    break;
	case RESP_TYPE_SV_SIMPLE:
	    svsim++;
	    if ((*resp >= 0x01 && *resp <= 0x0f)) {
		errresp ++;
		lasterr = *resp;
		errcmd = (ip << 24) | (b1 << 16) | (b2 << 8) | b3;
	    }
	    ioerr = len != 1;
	    break;
	case RESP_TYPE_IM_WRITE:
	case RESP_TYPE_DEF_W1:
	case RESP_TYPE_DEF_W2:
	    ioerr = len != 1;
	    break;
	case RESP_TYPE_DEF_R1:
	    nreads ++;
	    ioerr = len != 1;
	    break;
	case RESP_TYPE_SV_STAT:
	case RESP_TYPE_IM_READ:
	case RESP_TYPE_DEF_R2:
	    nreads ++;
	    read_from_rtx(&len, resp, 3);
	    ioerr = len != 3;
	    expect = 3;
	    break;
	default:
	    *resplen = 0;
	    rtxerror = RESPONSE_UNKNOWN;
	    return -1;
    }
    if (ioerr) {
	rtx_resync(1);
	if (expect > len)
	    rtxerror = RESPONSE_INCOMPLETE;
	else if (expect < len)
	    rtxerror = RESPONSE_OVERRUN;
	else
	    rtxerror = RESPONSE_UNKNOWN;
	*resplen = 0;
	return -1;
    }
    *resplen = len;
    ncmds ++;
    return 0;
}

int
rtx_set_toggle_mode(m)
int m;
{
    int old = toggle_mode;
    unsigned char resp[3];
    int len,ip1,ip2;

    if (m == toggle_mode)
	return old;
    rtxerror = COMMS_FAULT;
    if (toggle_mode) {
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerror = COMMS_FAULT; return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerror = COMMS_FAULT; return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerror = COMMS_FAULT; return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		|| (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1))
	    return -1;
	ip1 = resp[0] & RESP_IDENTIFY_MASK;
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1)
	    return -1;
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		|| (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1))
	    return -1;
	ip2 = resp[0] & RESP_IDENTIFY_MASK;
	if (ip1 == ip2) { rtxerror = COMMS_FAULT; return -1; }
	current_ip = ip2;
    } else {
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1)
	    return -1;
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1)
	    return -1;
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		|| (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1))
	    return -1;
	ip1 = resp[0] & RESP_IDENTIFY_MASK;
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		|| (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1))
	    return -1;
	ip2 = resp[0] & RESP_IDENTIFY_MASK;
	if (ip1 == ip2) { rtxerror = COMMS_FAULT; return -1; }
	current_ip = ip1;
    }
    toggle_mode = m;
    return old;
}

static int
rtx_resync(int err)
{
    int m = toggle_mode;
    int ip = current_ip;
    int len;
    unsigned char resp[3];
    static int lock = 0;

    if (err)
	resync++;

    if (lock) return -1;
    lock++;

    msleep(8);

    flush_input(tty_fd);

    (void) rtx_set_toggle_mode(rtx_set_toggle_mode(1-m));
    if (ip != current_ip) {
	if (toggle_mode) {
	    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1) {
		return lock=0,-1;
            }
	    if (resp[0] & (RESP_IDENTIFY_MASK != 1 - current_ip)) {
	        rtxerror = COMMS_NOT_INITIALISED;
		return lock=0,-1;
	    }
	} else {
	    current_ip = 1 - current_ip;
	    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1)
		return lock=0,-1;
	}
    }
    lock = 0;
    return 0;
}

/*
 * Restart the IPCs. They can be running, or they can be dead.
 */
int
rtx_restart()
{
    unsigned char res[3];
    static int resps[10];
    int len;
    static int i;
    int t = toggle_mode;

    i = 0;
    if (rtx_raw(IPDONTCARE,1,CMD_IP_RESET,0,0,&len,res) == -1)
	return -1;
    resps[i++] = res[0];

    /*
     * It was observed that after resetting one IP, a toggle can
     * occur. Therefor we send a command IDENTIFY. If the response is
     * not RESP_IDENTIFY_X, a toggle occured or the other IP was already
     * in restarted mode. But first we must make sure that it doesn't toggle.
     */
    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,res) == -1)
	return -1;
    resps[i++] = res[0];

    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,res) == -1)
	return -1;
    resps[i++] = res[0];

    if (*res != RESP_IDENTIFY_0 && *res != RESP_IDENTIFY_1) {
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (rtx_raw(IPDONTCARE,1,CMD_IP_RESET,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (*res == RESP_IDENTIFY_0 || *res == RESP_IDENTIFY_1) {
	    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,res) == -1)
		return -1;
	    resps[i++] = res[0];
	}

	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];
    } else {
	if (rtx_raw(IPDONTCARE,1,CMD_IP_RESET,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	/*
	 * A toggle could have occured. But both must get the GO.
	 */
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];

	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,res) == -1)
	    return -1;
	resps[i++] = res[0];
    }

    toggle_mode = 0;
    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,res) == -1)
	return -1;
    resps[i++] = res[0];
    current_ip = res[0] & RESP_IDENTIFY_MASK;
    (void) rtx_set_toggle_mode(t);
    return 0;
}
static void
*tty_arm_probe(s)
char *s;
{
    struct termios t;

    if (s == 0 || strncmp("/dev/", s, 5) != 0)
    {
	rtx_log((void *) 0,"'%s' should start with '/dev/'\n", s);
	return 0;
    }

    tty_fd = open(s, O_RDWR|O_EXCL|O_NOCTTY);

    if (tty_fd == -1)
    {
	rtx_log((void *) 0,"Couldn't open '%s'\n", s);
	return 0;
    }

    memset(&t, 0, sizeof(t));
    t.c_iflag = 0;
    t.c_oflag = 0;
    t.c_cflag = B9600 | CS8 | CLOCAL | CREAD;
    t.c_lflag = 0;
    /* Read one character, must arrive within 100 ms */
    t.c_cc[VMIN] = 0;
    t.c_cc[VTIME] = 1;
    if (tcsetattr(tty_fd, TCSANOW, &t) == -1) {
	rtx_log((void *) 0,"Couldn't set attributes of '%s'\n", s);
	close(tty_fd);
	return 0;
    }
    return &tty_fd;
}

#define P(x) (cmd->params[x])
#define R(x) (cmd->params[x])

static void
tty_arm_command(d, cmd)
void *d;
libio *cmd;
{
    int err, i;
    unsigned char resp[3];
    int data[7];
#if 0
    char buf[100];


    sprintf(buf, "P %02x (%02x %02x %02x) ", cmd->what,
	cmd->params[0] & 0xff, cmd->params[1] & 0xff, cmd->params[2] & 0xff);
#endif
    switch (cmd->what) {
    case RTX_CMD_RAW_CMD:
	err = rtx_raw_command(P(0),P(1),P(2),P(3),P(4));
	break;
    case RTX_CMD_RAW_RESP:
	err = rtx_raw_response(P(0),&R(0),resp);
	if (err == 0) {
	    R(1) = resp[0] & 0xff;
	    R(2) = resp[1] & 0xff;
	    R(3) = resp[2] & 0xff;
	}
	break;
    case RTX_CMD_INIT:
	err = rtx_init_comms(P(0),P(1));
	break;
    case RTX_CMD_VERSION:
	err = rtx_version(cmd->params);
	break;
    case RTX_CMD_RESTART:
	err = rtx_restart();
	break;
    case RTX_CMD_DEFINE_HOME:
	err = rtx_define_origin();
	break;
    case RTX_CMD_RELOAD_PIDS:
	err = rtx_reload_pids();
	break;
    case RTX_CMD_SET_MODE:
	err = rtx_set_mode(P(0),P(1));
	break;
    case RTX_CMD_STOP:
	err = rtx_stop(P(0));
	break;
    case RTX_CMD_GO:
	err = rtx_go(P(0),P(1));
	break;
    case RTX_CMD_MOTOR_STATUS:
	err = rtx_motor_status(P(0),&R(0));
	break;
    case RTX_CMD_GENERAL_STATUS:
	err = rtx_general_status(&R(0));
	break;
    case RTX_CMD_READ:
	err = rtx_read(P(0),P(1),&R(0));
	break;
    case RTX_CMD_WRITE:
	err = rtx_write(P(0),P(1),P(2));
	break;
    case RTX_CMD_INTERPOLATE:
	for (i = 0; i < 7; i ++)
	    data[i] = P(i);
	err = rtx_interpolate(data);
	break;
    case RTX_CMD_SOAK:
	err = rtx_soak(P(0));
	break;
    case RTX_CMD_RAW:
	err = rtx_raw(P(0),P(1),P(2),P(3),P(4),&R(0),resp);
	if (err == 0) {
	    R(1) = resp[0] & 0xff;
	    R(2) = resp[1] & 0xff;
	    R(3) = resp[2] & 0xff;
	}
	break;
    default:
	err = -1;
	rtxerror = ARM_NOT_SUPPORTED;
	break;
    }
    if (err) {
	cmd->what = rtxerror;
    } else
	cmd->what = 0;
#if 0
    printf("%s = %02x %02x %02x %02x\n", buf, cmd->what,
	cmd->params[0] & 0xff, cmd->params[1] & 0xff, cmd->params[2] & 0xff);
#endif
}

static int
tty_arm_test(data)
void *data;
{
    return 0;
}

static int
tty_arm_finish(data)
void *data;
{
    close(tty_fd);
    return 0;
}

rtx_ops_t tty_io_ops = {
    tty_arm_probe,
    tty_arm_test,
    tty_arm_command,
    tty_arm_finish,
};
