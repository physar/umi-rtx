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
 * A shell that lets you send arbitrary commands to the daemon and
 * the robot.
 *
 * Functionality includes:
 * a) Calls to robot arm functions.
 * b) Call to daemon functions. (what connections/status)
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <ctype.h>
#include <signal.h> /* was sys/signal.h */
#include <setjmp.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>

#ifndef SVR4
#define strsignal(sig) sys_siglist[sig]
extern char *sys_siglist[];
#endif

#include "states.h"
#include "globals.h"

#include "rtx.h"
#include "armlib.h"

void init_names();
void top_rtx_restart(), top_rtx_restart_help();
void doinit(), init_help();
void doreadonly(), readonly_help();
void rtx_priv(), rtx_priv_help();
void move_arm_help(), move_arm_unsafe(), move_arm(), move_arm_rel();
void top_rtx_stop(), top_rtx_stop_help();
void rtx_print(), rtx_print_help();
void rtx_set(), rtx_set_help();
void top_rtx_go(), top_rtx_go_help();
void rtx_status(), rtx_status_help();
void vrtx_probe(), vrtx_probe_help();
void vrtx_test(), vrtx_test_help();
void vrtx_command(), vrtx_command_help();
void vrtx_finish(), vrtx_finish_help();

void return_to_toplevel();
static void emergency_stop();
void toplevel_init();
static int toplevel_catchall();

extern commandtable armlevel, daemonlevel;

/*
 * Functions are called with
 * fun(int argc, char ** argv);
 * help functions are called with
 * helpfun(command *cmd, int kind)
 * with kind one of HELP_USAGE, HELP_SHORT or HELP_LONG
 */
static builtin builtins[] = {
 { "arm",		0,			0,		&armlevel },
 { "daemon",		0,			0,		&daemonlevel },
 { "init",		doinit,			init_help },
 { "restart",		top_rtx_restart,	top_rtx_restart_help },
 { "move",		move_arm,		move_arm_help },
 { "rmove",		move_arm_rel,		move_arm_help },
 { "relmove",		move_arm_rel,		move_arm_help },
 { "umove",		move_arm_unsafe,	move_arm_help },
 { "unsafemove",	move_arm_unsafe,	move_arm_help },
 { "stop",		top_rtx_stop,		top_rtx_stop_help },
 { "print",		rtx_print,		rtx_print_help },
 { "setpid",		rtx_set,		rtx_set_help },
 { "go",		top_rtx_go,		top_rtx_go_help },
 { "status",		rtx_status,		rtx_status_help },
 { "probe",		vrtx_probe,		vrtx_probe_help },
 { "test",		vrtx_test,		vrtx_test_help },
 { "command",		vrtx_command,		vrtx_command_help },
 { "finish",		vrtx_finish,		vrtx_finish_help },
 { "su",		rtx_priv,		rtx_priv_help },
 { "readonly",		doreadonly,		readonly_help },
};

#define NBUILTINS (sizeof(builtins)/sizeof(builtin))

commandtable toplevel = {
    builtins, NBUILTINS,
    "", "rtxsh> ",
    toplevel_init,
    0,
    toplevel_catchall,
};

void
toplevel_init(table)
commandtable *table;
{
    char *name, *getenv(), *home, *arg[3], c;
    extern char **global_argv;
    extern int global_argc;
    int readrc = 1, readonly = 0, argc;

    init_names();

    if (global_argc > 1) {
	extern int optind, opterr;
	opterr = 0;
	while ((c = getopt(global_argc,global_argv,"fr")) != -1) {
	    switch(c) {
	    case 'f': readrc = 0; break;
	    case 'r': readonly = 1; break;
	    case '?':
		fprintf(stderr,"Usage: %s [-fr] [cmd [args]]\n",*global_argv);
		exit(1);
	    }
	}
	global_argv += optind - 1;
	global_argc -= optind - 1;
    }
    if (global_argc == 1)
	printf("RTX command line interface $Revision$\n");

    if (readonly) {
	argc = 2;
	arg[1] = "readonly";
    } else
	argc = 1;
    arg[0] = "init";
    arg[argc] = 0;
    docommand(table,argc,arg);

    (void) signal(SIGSEGV,emergency_stop);
    (void) signal(SIGQUIT,emergency_stop);
    (void) signal(SIGINT,emergency_stop);

    if (!readrc)
	return;

    home = getenv("HOME");
    if (home) {
	name = malloc(strlen(home) + sizeof("/.rtxrc"));
	if (name) {
	    strcpy(name,home);
	    strcat(name,"/.rtxrc");
	    readexecutecommands(table,name);
	    free(name);
	}
    }
}

/*
 * On receipt of a signal, perform an emergency stop of the arm.
 * If sig == SEGV quit.
 */
static void
emergency_stop(sig)
int sig;
{
    int err;

    fprintf(stderr,"\nemergency_stop %d\n",sig);
    err = arm_stop(DEAD_STOP);
    fflush(stdout);
    fprintf(stderr,"\n%s\n",strsignal(sig));

    if (sig != SIGSEGV && err == -1 &&
    (armerrno == COMMS_NOT_INITIALISED || armerrno == CONNECTION_READ_ONLY)) {
	return_to_toplevel();
	return;
    }
    if (err == -1)
	armperror("Emergency stop failed");
    else
	fprintf(stderr,"Emergency stop\n");
    if (sig == SIGSEGV) {
	signal(sig,SIG_DFL);
	kill(getpid(),sig);
	return;
    }
    return_to_toplevel();
}

static int
toplevel_catchall(s)
char *s;
{
    if (*s == '!') {
	system(s+1);
	return 1;
    }
    return 0;
}
