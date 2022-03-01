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
/* Copyright (c) 1991-2003 Universiteit van Amsterdam                     */
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
 * Layout of memory for interface area between host and
 * IPC.
 * Defines for command numbers
 */

#ifndef _comm_h
#define _comm_h

#define RTX_MAXPARAMS	7
#define RTX_MAXRESPONSE	4
#if RTX_MAXRESPONSE > RTX_MAXPARAMS
#define RTX_MAXIO	RTX_MAXRESPONSE
#else
#define RTX_MAXIO	RTX_MAXPARAMS
#endif

typedef struct rtxio {
    int hostflag;
    int pid;
    int cmd,params[RTX_MAXPARAMS];
    int ipcflag;
    int error,response[RTX_MAXRESPONSE];
    int magic;
} rtxio;

typedef struct libio {
    int what;
    int params[RTX_MAXIO];
} libio;

/*
 * The flag values.
 */

#define RTX_MAGIC		0x66604242

#define CLNT_PRIV		 1
#define CLNT_RDONLY		 2
#define CLNT_INET		 4
#define CLNT_NOT_INITED		 8
#define CLNT_IDLED		16
#define CLNT_TIMEO		32

#define HOST_CMD_NONE		0
#define HOST_CMD_LOADED		1

#define IPC_CMD_IDLE		0
#define IPC_CMD_DONE		1
#define IPC_CMD_BUSY		2

#define RTX_CMD_FIRST		 -1 /* initialize connection */
#define RTX_CMD_QUIT		 -2 /* terminate connection */
#define RTX_CMD_ABORT		 -3 /* abort daemon and driver */
#define RTX_CMD_PRIV		 -4 /* switch to privilige mode */
#define RTX_CMD_GET_NCON	 -5
#define RTX_CMD_GET_CON		 -6
#define RTX_CMD_CLOSE_CON	 -7
#define RTX_CMD_NO_PRIV		 -8
#define RTX_CMD_READ_WRITE	 -9
#define RTX_CMD_READ_ONLY	-10
#define RTX_CMD_GET_PARAMS	-11
#define RTX_CMD_SET_PARAMS	-12

#define RTX_RESP_ACK		 0

#define RTX_CMD_RAW_CMD		 0
#define RTX_CMD_RAW_RESP	 1
#define RTX_CMD_INIT		 2
#define RTX_CMD_VERSION		 3
#define RTX_CMD_RESTART		 4
#define RTX_CMD_DEFINE_HOME	 5
#define RTX_CMD_RELOAD_PIDS	 6
#define RTX_CMD_SET_MODE	 7
#define RTX_CMD_STOP		 8
#define RTX_CMD_GO		 9
#define RTX_CMD_MOTOR_STATUS	10
#define RTX_CMD_GENERAL_STATUS	11
#define RTX_CMD_READ		12
#define RTX_CMD_WRITE		13
#define RTX_CMD_INTERPOLATE	14
#define RTX_CMD_SOAK		15
#define RTX_CMD_RAW		16

#endif /* _comm_h */
