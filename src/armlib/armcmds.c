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
 * The calls to rtxdriver.
 * Simple copy of rtx.pas
 */
#include <stdio.h>
#include "ipcrt.h"
#include "rtx.h"
#include "comm.h"
#include "armraw.h"

static int arm_debug = 0;

int
arm_raw_command(ip,cmdlen,b1,b2,b3)
int ip,cmdlen,b1,b2,b3;
{
    libio rtxparams;

    rtxparams.params[0] = ip;
    rtxparams.params[1] = cmdlen;
    rtxparams.params[2] = b1;
    rtxparams.params[3] = b2;
    rtxparams.params[4] = b3;

    return arm_interrupt(RTX_CMD_RAW_CMD,&rtxparams);
}

int
arm_raw_response(ip,len,resp)
int ip,*len,*resp;
{
    libio rtxparams;
    int err;

    rtxparams.params[0] = ip;
    err = arm_interrupt(RTX_CMD_RAW_RESP,&rtxparams);
    *len = rtxparams.params[0];
    resp[0] = rtxparams.params[1];
    resp[1] = rtxparams.params[2];
    resp[2] = rtxparams.params[3];
    return err;
}

int
arm_init_comms(toggle,debug)
int toggle, debug;
{
    libio rtxparams;
    int err;

    arm_debug = debug;

    if(arm_debug>0)
       printf("======================= ARM version 24 March 2022\n");

    rtxparams.params[0] = toggle;
    rtxparams.params[1] = debug;
    err = arm_interrupt(RTX_CMD_INIT,&rtxparams);
    return err;
}

int
arm_version(version)
int version[];
{
    libio rtxparams;
    int err = arm_interrupt(RTX_CMD_VERSION,&rtxparams);

    if (err == 0) {
	version[0] = rtxparams.params[0];
	version[1] = rtxparams.params[1];
	version[2] = rtxparams.params[2];
    }
    return err;
}



int
arm_restart()
{
    return arm_interrupt(RTX_CMD_RESTART,(libio *) 0);
}

int
arm_define_origin()
{
    return arm_interrupt(RTX_CMD_DEFINE_HOME,(libio *) 0);
}

int
arm_reload_pids()
{
    return arm_interrupt(RTX_CMD_RELOAD_PIDS,(libio *) 0);
}

int
arm_set_mode(motor,mode)
int motor,mode;
{
    libio rtxparams;

    rtxparams.params[0] = motor;
    rtxparams.params[1] = mode;

    return arm_interrupt(RTX_CMD_SET_MODE,&rtxparams);
}

int
arm_stop(mode)
int mode;
{
    libio rtxparams;


    if(arm_debug>0)
        printf("arm_stop(%d)\n",mode);

    rtxparams.params[0] = mode;
    rtxparams.params[1] = 2; /* AVI: added June 2014 */
    return arm_interrupt(RTX_CMD_STOP,&rtxparams);
}

int
arm_go(m,bits)
int m,bits;
{
    libio rtxparams;

    rtxparams.params[0] = m;
    rtxparams.params[1] = bits;
    return arm_interrupt(RTX_CMD_GO,&rtxparams);
}

int
arm_motor_status(motor,status)
int motor, *status;
{
    libio rtxparams;
    int err;

    rtxparams.params[0] = motor;
    err = arm_interrupt(RTX_CMD_MOTOR_STATUS,&rtxparams);
    *status = rtxparams.params[0];
    return err;
}

int
arm_general_status(status)
int *status;
{
    libio rtxparams;
    int err;

    err = arm_interrupt(RTX_CMD_GENERAL_STATUS,&rtxparams);
    *status = rtxparams.params[0];
    return err;
}

int
arm_read(motor,code,result)
int motor, code, *result;
{
    int err;
    libio rtxparams;

    rtxparams.params[0] = motor;
    rtxparams.params[1] = code;
    err = arm_interrupt(RTX_CMD_READ,&rtxparams);
    *result = rtxparams.params[0];
    return err;
}

int
arm_write(motor,code,input)
int motor, code, input;
{
    libio rtxparams;

    rtxparams.params[0] = motor;
    rtxparams.params[1] = code;
    rtxparams.params[2] = input;
    return arm_interrupt(RTX_CMD_WRITE,&rtxparams);
}

int
arm_interpolate(data)
int *data;
{
    libio rtxparams;
    int i;

    for (i = 0; i < ZEDOWN; i++)
	rtxparams.params[i] = data[i];

    return arm_interrupt(RTX_CMD_INTERPOLATE,&rtxparams);
}

int
arm_soak(s)
int s;
{
    libio rtxparams;

    rtxparams.params[0] = s;
    return arm_interrupt(RTX_CMD_SOAK,&rtxparams);
}

int
arm_raw(ip,cmdlen,b1,b2,b3,reslen,resp)
int ip,cmdlen,b1,b2,b3, *reslen, *resp;
{
    libio rtxparams;
    int err;

    rtxparams.params[0] = ip;
    rtxparams.params[1] = cmdlen;
    rtxparams.params[2] = b1;
    rtxparams.params[3] = b2;
    rtxparams.params[4] = b3;

    err = arm_interrupt(RTX_CMD_RAW,&rtxparams);
    if (err == 0) {
	*reslen = rtxparams.params[0];
	resp[0] = rtxparams.params[1];
	resp[1] = rtxparams.params[2];
	resp[2] = rtxparams.params[3];
    }
    return err;
}
