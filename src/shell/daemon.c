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
 * Get status info of the daemon
 */

#include "states.h"
#include "globals.h"
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>
#include <stdio.h>
#include <rtx.h>
#include <rtxd.h>
#include <comm.h>
#include <pwd.h>

void dowho(), who_help();
void doclose(), close_help();
void doset(), set_help(), doshow(), show_help();
void doabort(), abort_help();

static builtin daemonbuiltins[] = {
 { "abort",		doabort, 		abort_help },
 { "close",		doclose,		close_help },
 { "show",		doshow,			show_help, },
 { "set",		doset,			set_help, },
 { "who",		dowho,			who_help },
};

commandtable daemonlevel = {
    daemonbuiltins, sizeof(daemonbuiltins)/sizeof(builtin),
    "daemon", "rtxd> ",
};

void
who_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("list all current connections\n");
	break;
    case HELP_LONG:
	printf("%s:\n\tlist all current connections\n",cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	printf("[no arguments]\n");
	break;
    }
}

/* ARGSUSED */
void
dowho(argc,argv)
int argc;
char **argv;
{
    int ncon, i;
    struct passwd *pwd;
    libio tmpio;
    struct hostent *thehost;


    if (argc != 1)
	rtxsh_usage();

    if (arm_interrupt(RTX_CMD_GET_NCON,&tmpio))
	armerror();

    ncon = tmpio.params[0];
    if (ncon > 0)
	printf("Id\tuser\t\thostname\t    time    idle\tflags\n");
    for (i = 0; i < ncon; i++ ) {
	tmpio.what = RTX_CMD_GET_CON;
	tmpio.params[0] = i;
	if (arm_interrupt(RTX_CMD_GET_CON,&tmpio) == -1)
	    armerror();

	pwd = getpwuid(tmpio.params[1]);
	printf("%2d\t%-16.16s",tmpio.params[0],
		pwd ? pwd->pw_name : "<unknown>");
	if (tmpio.params[2] == -1) {
	    printf("%-16.16s","<local>");
	} else {
	    thehost = gethostbyaddr((char*) &tmpio.params[2], 4, AF_INET);
	    if (!thehost) {
		printf("%-16.16s","<unknown>");
	    } else {
		printf("%-16.16s",thehost->h_name);
	    }
	}
	tmpio.params[3] /= 60;
	printf("%4d:%02du", tmpio.params[3]/60, tmpio.params[3] % 60);
	tmpio.params[4] /= 60;
	printf("%4d:%02du", tmpio.params[4]/60, tmpio.params[4] % 60);
	putchar('\t');
	
	if (tmpio.params[5] & CLNT_PRIV)
	    putchar('S');
	if (tmpio.params[5] & CLNT_RDONLY)
	    putchar('R');
	if (tmpio.params[5] & CLNT_IDLED)
	    putchar('I');
	if (tmpio.params[5] & CLNT_TIMEO)
	    putchar('T');
	putchar('\n');
    }
}

void
close_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s [<id>]\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("close a connection to the daemon\n");
	break;
    case HELP_LONG:
	printf("%s <id>:\n\tclose the connection <id> (default: own)\n",
		cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	printf("[<connection>]\n");
	break;
    }
}

void
doclose(argc,argv)
int argc;
char **argv;
{
    libio tmpio;

    if (argc != 2 && argc != 1)
	rtxsh_usage();

    if (argc == 1) {
	if (arm_interrupt(RTX_CMD_QUIT,(libio *) 0) == -1)
	    armerror();
    } else {
	tmpio.params[0] = str2num(argv[1]);
	if (arm_interrupt(RTX_CMD_CLOSE_CON,&tmpio) == -1)
	    armerror();
    }
}

extern int parnumber(),numberofpar();
extern char *parname(), **completepar();

void
set_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <parameter> <value>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("set daemon parameters\n");
	break;
    case HELP_LONG:
	printf("%s <parameter> <value>:\n",cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completepar(complete_arg);
	else if (complete_ind == 2)
	    printf("<seconds>\n");
	break;
    }
}

void
doset(argc,argv)
int argc;
char **argv;
{
    libio tmpio;
    int par, num;

    if (argc != 3)
	rtxsh_usage();

    par = parnumber(argv[1]);
    num = str2num(argv[2]);
    if (arm_interrupt(RTX_CMD_GET_PARAMS,&tmpio) == -1)
	armerror();
    tmpio.params[par] = num;
    if (arm_interrupt(RTX_CMD_SET_PARAMS,&tmpio) == -1)
	armerror();
}

void
show_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <parameter>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("show daemon parameters\n");
	break;
    case HELP_LONG:
	printf("%s <parameter>:\n",cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	complete_result = completepar(complete_arg);
	break;
    }
}

void
doshow(argc,argv)
int argc;
char **argv;
{
    libio tmpio;
    int i;

    if (arm_interrupt(RTX_CMD_GET_PARAMS,&tmpio) == -1)
	armerror();
    if (argc == 1)
	for (i = 0; i < numberofpar(); i++)
	    printf("%-16.16s%10d\n",parname(i),tmpio.params[i]);
    else
	for (i = 1; i < argc; i++)
	    printf("%-16.16s%10d\n",argv[i],tmpio.params[parnumber(argv[i])]);
}

void
abort_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("abort daemon and terminate IPC program\n");
	break;
    case HELP_LONG:
	printf("%s:\n\tabort daemon and terminate IPC program\n",
		cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	printf("[no arguments]\n");
	break;
    }
}

/* ARGSUSED */
void
doabort(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (arm_interrupt(RTX_CMD_ABORT,(libio *) 0) == -1)
	armerror();
}
