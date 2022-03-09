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
 * Raw interface with the robot arm.
 *
 * This are (should be, anyway) the only functions that
 * now about RTX and the internal state maintained by this module.
 *
 * (minimal) error checking currently.
 *
 * rtx_init_comms
 *
 * rtx_raw
 *
 * rtx_resync
 *
 * rtx_set_toggle_mode
 *
 * rtx_restart
 *
 */

#include <stdio.h>
#include "rtxcmds.h"
#include "rtx.h"
#include "ipcrt.h"
#include "raw.h"

static int current_ip;
static int toggle_mode;
static unsigned long lasttime = 0;
static int inited = 0,initing = 0;

/* debug */
static char *status = 0;
static int lastbyte,ncmds = 0;
static int rtxdebug;

int rtxerr;

/*
 * Start the communication with the robot.
 * Send the first commands.
 */
static int rerrs = 0, nreads = 0, nid = 0, svsim = 0, errresp = 0;
static int lasterr = 0, errcmd = 0, resync = 0;


int
rtx_init_comms(tmode,debug)
int tmode,debug;
{
    static int first = 1;
    int len,ip;
    unsigned char resp[3];

    printf("called rtx_init_comms(%d,%d)\n",tmode, debug);
    rtxdebug = debug;

    if (first) {
	putstatus("$Id$\n\n");
	putstatus("RTX inited, %d resync\n", &resync);
	putstatus("Last = %x, lerr = %x, ncmds = %d, ip = %d, stat = %s\n",
	    &lastbyte,&rtxerr,&ncmds,&current_ip,&status);
	putstatus("Rerr = %d, nread = %d, nid = %d, svsim = %d\n",
		&rerrs,&nreads,&nid,&svsim);
	putstatus("Error responses = %d, Last err = %x (cmd = %x)\n",
		&errresp, &lasterr, &errcmd);
	first = 0;
    }

    initing = 1; inited = 0;
    lasttime = mtime();
    status = "raw1";
    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1)
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1)
	    return -1;

    if (len != 1 || (resp[0] != RESP_ACK && resp[0] != RESP_IP_RESTART &&
		     resp[0] != RESP_IP_RESTART1)) {
	    rtxerr = COMMS_FAULT;
	    return -1;
    }

    status = "raw2";
    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1)
	return -1;

    if (len != 1) {
	rtxerr = RESPONSE_OVERRUN;
	return -1;
    }

    toggle_mode = 0;

    switch(resp[0]) {
    case RESP_IP_RESTART:
    case RESP_IP_RESTART1:
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,resp) == -1)
	    return -1;
	if (len != 1 || resp[0] != RESP_ACK) {
	    rtxerr = COMMS_FAULT;
	    return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1)
	    return -1;
	if (len != 1 ||
	    (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1)) {
		rtxerr = COMMS_FAULT;
		return -1;
	}
	/* FALL THROUGH */
    case RESP_IDENTIFY_0:
    case RESP_IDENTIFY_1:
	ip = resp[0] & RESP_IDENTIFY_MASK;
	break;
    default:
	rtxerr = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK) {
	rtxerr = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 || len != 1
	    || (resp[0] != RESP_ACK && resp[0] != RESP_IP_RESTART &&
		resp[0] != RESP_IP_RESTART1)) {
	rtxerr = COMMS_FAULT;
	return -1;
    }

    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1) {
	rtxerr = COMMS_FAULT;
	return -1;
    }

    switch(resp[0]) {
    case RESP_IP_RESTART:
    case RESP_IP_RESTART1:
	if (rtx_raw(IPDONTCARE,1,CMD_GO,0,0,&len,resp) == -1 || len != 1
		|| resp[0] != RESP_ACK) {
	    rtxerr = COMMS_FAULT;
	    return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1 || len != 1
		||
	    (resp[0] != RESP_IDENTIFY_0 && resp[0] != RESP_IDENTIFY_1)) {
		rtxerr = COMMS_FAULT;
		return -1;
	}
	/* FALL THROUGH */
    case RESP_IDENTIFY_0:
    case RESP_IDENTIFY_1:
	current_ip = resp[0] & RESP_IDENTIFY_MASK;
	break;
    default:
	rtxerr = COMMS_FAULT;
	return -1;
    }
    if (ip == current_ip) {
	rtxerr = COMMS_FAULT;
	return -1;
    }

    if (tmode) {
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK ||
	    rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ON,0,0,&len,resp) == -1 ||
	    len != 1 || resp[0] != RESP_ACK) {
		rtxerr = COMMS_FAULT;
		return -1;
	}
	toggle_mode = 1;
    }
    initing = 0;
    inited = 1;
    return 0;
}

static int getcharbefore(port,time)
unsigned long time;
int port;
{
    while (mtime() < time)
	if (inputpending(port))
	    return inputc(port);
    return -1;
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
    int ioerr = 0, byte, len, expect;

    printf("called rtx_raw(%d,%d,%c,%c,%c,%d,%c)\n",ip,cmdlen,b1,b2,b3,*resplen,*resp);
    if (!(inited ^ initing) ) {
	rtxerr = COMMS_NOT_INITIALISED;
	return -1;
    }

    if (ip != IP0 && ip != IP1 && ip != IPDONTCARE) {
	rtxerr = ARM_SELECTION_OOR;
	return -1;
    }

    if (ip != IPDONTCARE && ip != current_ip) {
	unsigned char tmp[3];

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

    status = "timesync";
    msleep(lasttime+8-mtime());
    lasttime = mtime();
    status = "outputc";
    (void) outputc(PORTB,b1);
    if (cmdlen >= 2) {
	(void) outputc(PORTB,b2);
	if (cmdlen == 3)
	    (void) outputc(PORTB,b3);
    }

    if (!initing && toggle_mode)
	current_ip = 1 - current_ip;

    status = "inputc";

    for (len = 0; len < 3; len++) {
	lastbyte = byte = getcharbefore(PORTB,lasttime+8);
	if (byte < 0)
	    break;
	resp[len] = (unsigned char) byte;
    }
    if (getcharbefore(PORTB,lasttime+8) != -1) {
	rtx_resync(1);
	rtxerr = RESPONSE_OVERRUN;
	return -1;
    }
    if (len == 0) {
	rtx_resync(1);
	rtxerr = NO_RESPONSE;
	return -1;
    }
    status = "doresponse";
    expect = 1;
    switch(*resp & RESP_TYPE_MASK) {
	case RESP_TYPE_SV_ID:
	    nid++;
	    ioerr = b1 == CMD_VERSION ? len != 3 : len != 1;
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
	    ioerr = len != 3;
	    expect = 3;
	    break;
	default:
	    *resplen = 0;
	    rtxerr = RESPONSE_UNKNOWN;
	    return -1;
    }
    if (ioerr) {
	rtx_resync(1);
	if (expect > len)
	    rtxerr = RESPONSE_INCOMPLETE;
	else if (expect < len)
	    rtxerr = RESPONSE_OVERRUN;
	else
	    rtxerr = RESPONSE_UNKNOWN;
	*resplen = 0;
	return -1;
    }
    *resplen = len;
    status = "command completed";
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
    rtxerr = COMMS_FAULT;
    if (toggle_mode) {
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerr = COMMS_FAULT; return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_ONCE,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerr = COMMS_FAULT; return -1;
	}
	if (rtx_raw(IPDONTCARE,1,CMD_TOGGLE_OFF,0,0,&len,resp) == -1 ||
		len != 1 || resp[0] != RESP_ACK) {
	    rtxerr = COMMS_FAULT; return -1;
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
	if (ip1 == ip2) { rtxerr = COMMS_FAULT; return -1; }
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
	if (ip1 == ip2) { rtxerr = COMMS_FAULT; return -1; }
	current_ip = ip1;
    }
    toggle_mode = m;
    return old;
}

int
rtx_resync(err)
int err;
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

    while (inputpending(PORTB))
	(void) inputc(PORTB);
    (void) rtx_set_toggle_mode(rtx_set_toggle_mode(1-m));
    if (ip != current_ip) {
	if (toggle_mode) {
	    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,resp) == -1)
		return lock=0,-1;
		if (resp[0] & RESP_IDENTIFY_MASK != 1 - current_ip) {
		    rtxerr = COMMS_NOT_INITIALISED;
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
    static int first = 1;
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

    if (rtxdebug >= 2 && first) {
	first = 0;
	putstatus("Done with restart: (%d) %x %x %x %x %x %x",&i,
	    resps, resps+1, resps+2, resps+3, resps+4, resps+5);
	putstatus(" %x %x %x %x\n", resps+6, resps+7, resps+8, resps+9);
    }
    toggle_mode = 0;
    if (rtx_raw(IPDONTCARE,1,CMD_IDENTIFY,0,0,&len,res) == -1)
	return -1;
    resps[i++] = res[0];
    current_ip = res[0] & RESP_IDENTIFY_MASK;
    (void) rtx_set_toggle_mode(t);
    return 0;
}
