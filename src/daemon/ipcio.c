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
 * Io with the arm, using shared memory.
 *
 * interface functions in array ipc_io_ops
 * rtx_ops_t ipc_io_ops = {
 *     ipc_arm_probe,
 *     ipc_arm_test,
 *     ipc_arm_command,
 *     ipc_arm_finish,};
 *
 *
 * int arm_set_pid(); // removed
 *
 * static functions
 *
 * arm_init_lib();
 * arm_command(libio *cmd);
 * void *ipc_arm_probe(char *s) // s has to be "ipc"
 * void *ipc_arm_command(void *data, libio *cmd);
 * int ipc_arm_test(void *data);
 * int ipc_arm_finish(void *data);
 */
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <signal.h>
#include <unistd.h>
#include <stdlib.h>
#include "vrtx.h"
#include "ipcrt.h"
#include "rtx.h"
#include "rtxd.h"
#include "comm.h"
#include "hostlib.h"

static unsigned long armuptime = 0;
unsigned long ticks();

static int library_loaded = 0;
static rtxio *ipcio = 0;

#if 0
/*
 * Must be called after fork()
 */
arm_set_pid()
{
    if (ipcio)
	ipcio->pid = getpid();
}
#endif

/*
 * Initialise the library
 */
static int
arm_init_lib()
{
    if (library_loaded)
	return 0;

    if (initipc() != 0) {
	rtx_log((struct client *) 0, "Can not open IPC");
	return -1;
    }
    if (INDRAM(*(int *)PARAMPTR))
	ipcio = (rtxio *) (ipcbase + *(int *)PARAMPTR);
    else
	ipcio = 0;

    if (!alive() || !ipcio || ipcio->magic != RTX_MAGIC) {
	(void) stopcpu();
	if (download(RTXD_DOWNLOAD) != 0 || runcpu() != 0) {
	    rtx_log((struct client *) 0,"Can't download or run %s on robot",
			    RTXD_DOWNLOAD);
	    exit(1);
	}
	usleep(10000);
	if (!alive()) {
	    rtx_log((struct client *) 0,"robot CPU not responding");
	    stopcpu();
	    exit(1);
	}
    }
    armuptime = ticks();

    if (INDRAM(*(int *)PARAMPTR))
	ipcio = (rtxio *) (ipcbase + *(int *)PARAMPTR);
    else
	ipcio = 0;

    if (ipcio && ipcio->magic == RTX_MAGIC) {
	if (ipcio->pid && ipcio->pid != getpid() &&
		(kill(ipcio->pid,0) == 0 || errno == EPERM)) {
	    rtx_log((struct client *) 0,"Library in use by process %d",ipcio->pid);
	    return -1;
	}
	ipcio->pid = getpid();
	usleep(20000);
	if (ipcio->pid != getpid()) {
	    rtx_log((struct client *) 0,"Library in use by process %d",ipcio->pid);
	    return -1;
	}
	ipcio->hostflag = HOST_CMD_NONE;
	library_loaded = 1;
    }
    return 0;
}

/*
 * Do one command
 */
static void
arm_command(cmd)
libio *cmd;
{
    int i;
    int cnt = 0;

    if (!library_loaded) {
	if (arm_init_lib() != 0) {
	    rtx_log((struct client *) 0,"FATAL - can't connect to robot");
	    exit(1);
	}
    }
    /*
     * No rtxdriver loaded, or rtxdriver restarted.
     * We don't like that, so redownload and restart.
     */
    if (INDRAM(*(int *)PARAMPTR))
	ipcio = (rtxio *) (ipcbase + *(int *)PARAMPTR);
    else
	ipcio = 0;

    if (!ipcio || ipcio->magic != RTX_MAGIC || armuptime >= ticks()) {
	rtx_log((struct client *) 0,
		"IPC restarted or crashed, daemon restarting IPC");
	stopcpu();
	cmd->what = NO_RESPONSE;
	library_loaded = 0;
	if (arm_init_lib() == -1) {
	    rtx_log((struct client *) 0,"FATAL - can't reconnect to robot");
	    exit(1);
	}
	return;
    }
    armuptime = ticks();
    while (ipcio->ipcflag != IPC_CMD_IDLE) {
	if (cnt ++ > 10) {
	    if (!alive()) {
		cmd->what = NO_RESPONSE;
		return;
	    } else {
		cnt = 0;
	    }
        }
	usleep(8000);
    }
    cnt = 0;
    
    /*
     * arm_interpolate could take somewhat more time.
     * Should compute (ntimes * 8ms * 2)
     */
    ipcio->cmd = cmd->what;
    for (i = 0; i < RTX_MAXPARAMS; i++)
	ipcio->params[i] = cmd->params[i];
    ipcio->hostflag = HOST_CMD_LOADED;
    while (ipcio->ipcflag != IPC_CMD_DONE) {
	if (cnt ++ > (cmd->what == RTX_CMD_INTERPOLATE ? 100 : 20)) {
	    if (!alive()) {
		cmd->what = NO_RESPONSE;
		return;
	    } else {
		cnt = 0;
	    }
        }
	usleep(8000);
    }
    for (i = 0; i < RTX_MAXRESPONSE; i++)
	 cmd->params[i] = ipcio->response[i];
    ipcio->hostflag = HOST_CMD_NONE;
    cmd->what = ipcio->error;
    return;
}

/*
 * The new arm operations through the rtx_ops interface
 */

static char cookie;

static void
*ipc_arm_probe(s)
char *s;
{

    if (s == 0 || strcasecmp("ipc", s) != 0)
	return 0;

    if (arm_init_lib() == 0)
	return &cookie;

    return 0;
}

static void
ipc_arm_command(data, cmd)
void *data;
libio *cmd;
{
    arm_command(cmd);
}

static int
ipc_arm_test(data)
void *data;
{
    if (!alive()) {
	(void) arm_init_lib();
	return 1;
    }
    return 0;
}

static int
ipc_arm_finish(data)
void *data;
{
    stopcpu();
    return 0;
}

rtx_ops_t ipc_io_ops = {
    ipc_arm_probe,
    ipc_arm_test,
    ipc_arm_command,
    ipc_arm_finish,
};
