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
 * These routines provide access to the robot arm in a structured way.
 * It redefines all call in calls to rtx_do, which is a wrapper for rtx_raw.
 *
 * Should this run on ythe IPC or should it be used from
 * the host? The IPC would be more efficient.
 * To cater for a standalone enviroment we don't use anything but
 * C. No libc, no Unix.
 * There must be better checking of error returns.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "comm.h"
#include "rtx.h"
#include "rtxd.h"
#include "rtxcmds.h"
#include "raw.h"

#define MOTOR2IP	motor2ip
#define MOTOR2CTL	motor2ctl

extern int rtxerror; /* defined in ttyio.c */
extern int rtxdebug; /* defined in ttyio.c */
static int do_interpolate();

static int
motor2ip(motor)
int motor;
{
    switch(motor) {
    case ELBOW: case SHOULDER: case ZED: case YAW: case GRIP: case ZEDOWN:
	return IP1;
    case WRIST1: case WRIST2:
	return IP0;
    default:
	return 2; /* a value that makes rtx_do/rtx_raw barf */
    }
}

static int
motor2ctl(motor)
int motor;
{
    switch(motor) {
    case ELBOW:
	return 0;
    case SHOULDER:
	return 1;
    case ZED: case ZEDOWN: case WRIST1:
	return 2;
    case YAW: case WRIST2:
	return 3;
    case GRIP:
	return 4;
    default:
	return 5; /* a value that makes the IP barf */
    }
}

static int checksum(sum,b1, b2)
int sum, b1, b2;
{
    return (((b1>>4) ^ b1 ^ (b2>>4) ^ b2) & 0xf) == (sum &=  0xf);
}

/*
 * Do a command with rtx_raw. Check the response. If expect >= 0,
 * Check the response against expect.
 */
#define UBYTE(x)	((unsigned char) ((x) & 0xff))
static int
rtx_do(ip,cmdlen,b1,b2,b3,resplen,resp,expect)
int ip, b1,b2,b3, cmdlen, *resplen, expect;
unsigned char *resp;
{
    rtx_log((client *) 0,"called rtx_do(%d,%d,%d,%d,%d,%d,%u,%d)",ip,cmdlen,b1,b2,b3,*resplen,*resp,expect);
    if (rtx_raw(ip,cmdlen,UBYTE(b1),UBYTE(b2),UBYTE(b3),resplen,resp) == -1)
	return -1;
    /*
     * Error returns
     */
    if (resp[0] <= RESP_IP_RESTART && resp[0] > RESP_ACK) {
	switch(resp[0]) {
	case RESP_PROGRESS:
	    rtxerror = ARM_IN_PROGRESS;
	    return -1;
	case RESP_STORED:
	    rtxerror = ARM_STORED;
	    return -1;
	case RESP_AXIS_BUSY:
	    rtxerror = ARM_AXIS_BUSY;
	    return -1;
	case RESP_IP_BUSY:
	    rtxerror = ARM_DECODER_BUSY;
	    return -1;
	case RESP_PARAM_OOR:
	    rtxerror = ARM_PARAMETER_OOR;
	    return -1;
	case RESP_READ_ONLY:
	    rtxerror = ARM_READ_ONLY;
	    return -1;
	case RESP_READ_ONLY1:
	    rtxerror = ARM_READ_ONLY;
	    return -1;
	case RESP_SELECT_OOR:
	    rtxerror = ARM_SELECTION_OOR;
	    return -1;
	case RESP_CMD_OOR:
	    rtxerror = ARM_COMMAND_OOR;
	    return -1;
	case RESP_CMD_NOT_SUP:
	    rtxerror = ARM_NOT_SUPPORTED;
	    return -1;
	case RESP_FRAME_TIMEO:
	    rtxerror = ARM_FRAME_TIMEOUT;
	    return -1;
	case RESP_FRAME_OVERRUN:
	    rtxerror = ARM_FRAME_OVERRUN;
	    return -1;
	case RESP_PARITY_ERROR:
	    rtxerror = ARM_PARITY;
	    return -1;
	case RESP_IP_RESTART1:
	case RESP_IP_RESTART:
	    rtxerror = ARM_RESTARTED;
	    return -1;
	}
    }

    /*
     * Check if the response is ok
     */
    switch (expect) {
	case -1:
	    return 0;
	case RESP_ACK:
	case RESP_DEF_READ1:
	case RESP_DEF_WRITE1:
	    if (resp[0] != expect) {
		rtxerror = RESPONSE_UNKNOWN;
		return -1;
	   }
	   return 0;
	case RESP_IM_READ:
	case RESP_DEF_READ2:
	    if ((resp[0] & 0xf0) != expect) {
		rtxerror = RESPONSE_UNKNOWN;
		return -1;
	    } else if (!checksum(resp[0],resp[1],resp[2])) {
		rtxerror = CHECKSUM;
		return -1;
	    }
	    return 0;
	case RESP_IM_WRITE:
	case RESP_DEF_WRITE2:
	    if ((resp[0] & 0xf0) != expect) {
		rtxerror = RESPONSE_UNKNOWN;
		return -1;
	    } else if (!checksum(resp[0],b2,b3)) {
		rtxerror = CHECKSUM;
		return -1;
	    }
	    return 0;
	case RESP_STAT_GENERAL:
	case RESP_STAT_CTL_0:
	case RESP_STAT_CTL_1:
	case RESP_STAT_CTL_2:
	case RESP_STAT_CTL_3:
	case RESP_STAT_CTL_4:
	    if (resp[0] != expect) {
		rtxerror = RESPONSE_UNKNOWN;
		return -1;
	    }
	    return 0;
	default:
	    rtxerror = RESPONSE_UNKNOWN;
	    return -1;
    }
}


static unsigned char rtx_response[2][3];
static int rtx_response_len[2], rtx_response_err[2];
int
rtx_raw_command(ip,cmdlen,b1,b2,b3)
int ip, cmdlen, b1, b2, b3;
{
    int err;
    if (ip != IP0 && ip != IP1) {
	rtxerror = SELECTION_OOR;
	return -1;
    }

    err = rtx_raw(ip,cmdlen,UBYTE(b1),UBYTE(b2),UBYTE(b3),
	&rtx_response_len[ip],rtx_response[ip]);
    if (err)
	rtx_response_err[ip] = rtxerror;
    else
	rtx_response_err[ip] = 0;
    return err;
}

int
rtx_raw_response(ip,len,resp)
int ip;
int *len;
unsigned char *resp;
{
    int i;

    if (ip != IP0 && ip != IP1) {
	rtxerror = SELECTION_OOR;
	return -1;
    }

    rtxerror = rtx_response_err[ip];
    *len = rtx_response_len[ip];
    if (rtx_response_err[ip] || rtx_response_len[ip] > 3 ||
	    rtx_response_len[ip] < 1)
	return -1;

    for (i = 0; i < *len; i++)
	resp[i] = rtx_response[ip][i];
    return 0;
}

int
rtx_version(vers)
int vers[];
{
    unsigned char res[3];
    int len;
    char *p;

    if (rtx_do(IP0,1,CMD_VERSION,0,0,&len,res,-1) == -1)
	return -1;

    p = strchr("$Revision$",':');
    if (p) {
	long strtol();
	char *s;

	vers[0] = (int) strtol(p+1,&s,0) & 0xff;
	if (s && *s)
	    vers[1] = (int) strtol(s+1,&s,0) & 0xff;
	else
	    vers[1] = 0;
    } else {
	vers[0] = vers[1] = 0;
    }
    /* assume revision is first byte, version is second */
    vers[2] = res[1] | res[2] << 8;
    return 0;
}

int
rtx_define_origin()
{
    unsigned char res[3];
    int len;

    if (rtx_do(IP0,1,CMD_IP_DEF_HOME,0,0,&len,res,RESP_ACK) == -1)
        return -1;

    if (rtx_do(IP1,1,CMD_IP_DEF_HOME,0,0,&len,res,RESP_ACK) == -1)
	return -1;

    return 0;
}

int
rtx_reload_pids()
{
    int len;
    unsigned char resp[3];

    if (rtx_do(IP0,1,CMD_IP_DEFAULT,0,0,&len,resp,RESP_ACK) == -1)
	return -1;

    if (rtx_do(IP1,1,CMD_IP_DEFAULT,0,0,&len,resp,RESP_ACK) == -1)
	return -1;

    return 0;
}

int
rtx_set_mode(motor,mode)
int motor, mode;
{
    unsigned char res[3];
    int len;

    if (rtx_do(MOTOR2IP(motor),3,CMD_MODE_SET | (int)mode,MOTOR2CTL(motor),0,
		&len,res,RESP_ACK) == -1)
	return -1;

    return 0;
}

int
rtx_stop(mode)
int mode;
{
    unsigned char resp[3];
    int len;

    rtx_log((client *) 0,"called rtx_stop(%d)\n",mode);

    if (mode < 0 || mode >= NUMBER_OF_STOP_MODES) {
	rtxerror = SELECTION_OOR;
	return -1;
    }
    if (rtx_do(IP0,1,CMD_XXXX_STOP | mode,0,0,&len,resp,RESP_ACK) == -1)
	return -1;

    if (rtx_do(IP1,1,CMD_XXXX_STOP | mode,0,0,&len,resp,RESP_ACK) == -1)
	return -1;
    
    return 0;
}

#define NUMGO(x)	((((x)>>1) ^ (x)) & 1)

static int
getbits(motor,bits)
int motor;
int bits;
{
    int b;
    switch (motor) {
    case ELBOW: b = bits; break;
    case SHOULDER: b = bits >> 2; break;
    case ZEDOWN:
    case ZED: b = bits >> 4; break;
    case WRIST1: b = bits >> 6; break;
    case WRIST2: b = bits >> 8; break; /* According to read.me errata */
    case YAW: b = bits >> 10; break;
    case GRIP: b = bits >> 12; break;
    default:
	rtxerror = PARAMETER_OOR;
	return 0;
    }
    return b & 0x3;
}

int
rtx_go(mode,bits)
int mode;
int bits;
{
    rtxerror = 0;
    if (mode == MANUAL_GO) {
	int b[2],b1,b2;
	int len, m;
	unsigned char resp[3];

	b[0] = b[1] = 0;

	for (m = 0; m < NUMBER_OF_MOTORS; m++) {
	    b[MOTOR2IP(m)] |= getbits(m,bits) << (MOTOR2CTL(m)*2);
	}
	if (rtxerror)
	    return -1;

	b1 = b[IP0] & 0xff;
	b2 = (b[IP0] >> 8) & 0xff;
	if (rtx_do(IP0,3,CMD_DIR_MAN,b1,b2,&len,resp,RESP_ACK) == -1)
	    return -1;
	b1 = b[IP1] & 0xff;
	b2 = (b[IP1] >> 8) & 0xff;
	if (rtx_do(IP1,3,CMD_DIR_MAN,b1,b2,&len,resp,RESP_ACK) == -1)
	    return -1;
    } else if (mode == NUMERIC_GO) {
	int cmd[2];
	int len, m;
	unsigned char resp[3];

	cmd[0] = cmd[1] = CMD_DIR_NUM;

	for (m = 0; m < NUMBER_OF_MOTORS; m++) {
	    cmd[MOTOR2IP(m)] |= NUMGO(getbits(m,bits)) << MOTOR2CTL(m);
	}
	if (rtxerror)
	    return -1;

	if (rtx_do(IP0,1,cmd[IP0],0,0,&len,resp,RESP_ACK) == -1)
	    return -1;
	if (rtx_do(IP1,1,cmd[IP1],0,0,&len,resp,RESP_ACK) == -1)
	    return -1;
    } else {
	rtxerror = PARAMETER_OOR;
	return -1;
    }
    return 0;
}

int
rtx_motor_status(motor,status)
int motor, *status;
{
    unsigned char resp[3];
    int len;
    if (rtx_do(MOTOR2IP(motor),1,CMD_STAT_BASE|MOTOR2CTL(motor),0,0,
		&len,resp,RESP_STAT_BASE|MOTOR2CTL(motor)) == -1)
	return -1;

    *status = resp[1];
    return 0;
}

int
rtx_general_status(status)
int *status;
{
    unsigned char resp[3];
    int len;

    if (rtx_do(IP0,1,CMD_STAT_GENERAL,0,0,&len,resp,RESP_STAT_GENERAL) == -1)
	return -1;

    *status = resp[1];
    if (rtx_do(IP1,1,CMD_STAT_GENERAL,0,0,&len,resp,RESP_STAT_GENERAL) == -1)
	return -1;

    *status |= resp[1];
    return 0;
}

int
rtx_read(motor,code,result)
int motor, code, *result;
{
    unsigned char res[3];
    int len,cmd;

    if (code >= NUMBER_OF_DATA_CODES || code < 0) {
	rtxerror = SELECTION_OOR;
	return -1;
    }
    switch (code) {
    case ERROR:
    case CURRENT_POSITION:
	cmd = (code == ERROR) ? CMD_IM_READ_ERR : CMD_IM_READ_CP;
	cmd |= MOTOR2CTL(motor);
	if (rtx_do(MOTOR2IP(motor),1,cmd,0,0,&len,res,RESP_IM_READ) == -1)
	    return -1;
	*result = res[1] + (res[2]<<8);
	if (*result > 0x7fff) *result |= 0xffff0000;
	break;
    case ACTUAL_POSITION:
	{
	    int erdif,cp;
	    if (rtx_read(motor,CURRENT_POSITION,&cp) == -1)
	        return -1;
	    if (rtx_read(motor,ERROR,&erdif) == -1)
		return -1;

	    *result = cp - erdif;
	    return 0;
	}
    default:
	if (rtx_do(MOTOR2IP(motor),3,CMD_DEF_READ1,code,0,&len,res,
			RESP_DEF_READ1) == -1)
	    return -1;
	if (rtx_do(MOTOR2IP(motor),1, CMD_DEF_READ2 | MOTOR2CTL(motor),0,0,
		&len,res,RESP_DEF_READ2) == -1)
	    return -1;
	*result = res[1] + (res[2]<<8);
	if (*result > 0x7fff) *result |= 0xffff0000;
    }
    return 0;
}

int
rtx_write(motor,code,input)
int motor, code, input;
{
    unsigned char res[3];
    int len;

    if (code >= NUMBER_OF_DATA_CODES || code < 0) {
	rtxerror = SELECTION_OOR;
	return -1;
    }
    switch (code) {
    case ACTUAL_POSITION:
    case ERROR:
	rtxerror = ARM_READ_ONLY;
	return -1;
    case CURRENT_POSITION:
	return rtx_do(MOTOR2IP(motor),3,CMD_IM_WRITE_CP | MOTOR2CTL(motor),
		input & 0xff,(input>>8) & 0xff,&len,res,-1);
    default:
	if (rtx_do(MOTOR2IP(motor),3,CMD_DEF_WRITE1,code,0,&len,res,
		RESP_DEF_WRITE1) == -1)
	    return -1;
	if (rtx_do(MOTOR2IP(motor),3,CMD_DEF_WRITE2 | MOTOR2CTL(motor),
		input & 0xff,(input>>8) & 0xff,&len,res,RESP_DEF_WRITE2) == -1)
	    return -1;
    }
    return 0;
}

struct step {
    int decr;
    int cntr;
    int step;
    int sign;
};

/*
 * If the values given to rtx_interpolate are larger that the protocol allows,
 * the request is translated into multiple requests.
 * This is done by finding the minimum number of interpolate commands
 * that have to be given.
 * The division of the increment by the number of steps gives a remainder.
 * This remainder is spread out (by adding one) over all steps.
 * This should be done each times/remainder steps. But we don't want to
 * compute that. (This gives gives rounding errors)
 * That why we simply subtract the remainder from times. When we hit 0,
 * we add times to the counter, and add 1 to steps.
 */
int
rtx_interpolate(data)
int *data;
{
    int m;
    int tmpdata[NUMBER_OF_MOTORS-1];
    int savedata[NUMBER_OF_MOTORS-1];
    struct step step[NUMBER_OF_MOTORS-1];
    int times = 0,i;

    /* NOT zedown */
    for (m = 0; m < ZEDOWN; m++) {
	int t1;
	if (data[m] < 0)
	    t1 = (-data[m]+7)/8;
	else
	    t1 = (data[m]+6)/7;
	if (t1 > times)
	    times = t1;
    }
    if (times == 0)
	return 0;
    for (m = 0; m < ZEDOWN; m++) {
	step[m].sign = data[m] < 0 ? -1 : 1;
	savedata[m] = step[m].sign*data[m];
	step[m].step = savedata[m] / times;
	step[m].decr = savedata[m] % times;
	step[m].cntr = times;
    }
    for (i = 0; i < times; i++) {
	for (m = 0; m < ZEDOWN; m++) {
	    if ((step[m].cntr -= step[m].decr) <= 0) {
		tmpdata[m] = step[m].step + 1;
		step[m].cntr += times;
	    } else
		tmpdata[m] = step[m].step;
		if (tmpdata[m] > savedata[m]) {
#ifdef IPC
		    static int cnt = 0;
		    if (!cnt++)
			putstatus("Interpolation alert: %d\n",&cnt);
		    tmpdata[m] = savedata[m];
#else
		    abort();
#endif
	    }
	    savedata[m] -= tmpdata[m];
	    tmpdata[m] *= step[m].sign;
	}
	if (do_interpolate(tmpdata) == -1)
	    return -1;
    }
    return 0;
}

static int
do_interpolate(data)
int *data;
{
    int m, increments[2], cmd, b1, b2, len;
    unsigned char resp[3];

    /* NOT zedown */
    increments[0] = increments[1] = 0;
    for (m = 0; m < ZEDOWN; m++) {
	if (data[m] > 7 || data[m] < -8) {
	    rtxerror = PARAMETER_OOR;
	    return -1;
	}
	increments[MOTOR2IP(m)] |= (data[m] & 0xf) << (MOTOR2CTL(m) * 4);
    }
    cmd = CMD_INTERP | ((increments[IP0] >> 16) & 0xf);
    b1 = increments[IP0] & 0xff;
    b2 = (increments[IP0] >> 8) & 0xff;
    if (rtx_do(IP0,3,cmd,b1,b2,&len,resp,-1) == -1)
	return -1;
 
    cmd = CMD_INTERP | ((increments[IP1] >> 16) & 0xf);
    b1 = increments[IP1] & 0xff;
    b2 = (increments[IP1] >> 8) & 0xff;
    if (rtx_do(IP1,3,cmd,b1,b2,&len,resp,-1) == -1)
	return -1;

    return 0;
}

int
rtx_soak(s)
int s;
{
    unsigned char resp[3];
    int len;

    if(rtxdebug > 2) 
        rtx_log(NULL, "rtx_soak %d", s);
   
    if (s < 0 || s >= NUMBER_OF_SOAK) {
	rtxerror = SELECTION_OOR;
	return -1;
    }
    if (rtx_do(IP0,1,CMD_INIT+s,0,0,&len,resp,RESP_ACK) == -1)
	return -1;
    if (rtx_do(IP1,1,CMD_INIT+s,0,0,&len,resp,RESP_ACK) == -1)
	return -1;
    return 0;
}

#if 0
int
rtx_template()
{
    unsigned char resp[3];
    int len;
    
    return 0;
}
#endif

static char mess[80];

char*
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
	return "RTX_CMD_VERSION";
    case RTX_CMD_MOTOR_STATUS:
	return "RTX_CMD_MOTOR_STATUS";
    case RTX_CMD_GENERAL_STATUS:
	return "RTX_CMD_GENERAL_STATUS";
    case RTX_CMD_READ:
	return "RTX_CMD_READ";
    default:
        sprintf (mess, "RTX-command %d unknown", what);
	return mess;
    }
}

