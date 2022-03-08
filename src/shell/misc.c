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
 * Miscellaneous functions to control the arm/daemon.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <comm.h>
#include <sys/signal.h>
#include <arpa/inet.h>
#include "rtx.h"
#include "states.h"
#include "globals.h"
#include "armlib.h"
#include "armraw.h"
#include "vrtx.h"
#include "names.h"
char *motorname();

void
init_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s [readonly]\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("initialise communication with RTX\n");
	break;
    case HELP_LONG:
	printf("%s:\n\tinitialise communication with RTX\n",cmd->cmd_string);
	break;
    }
}

void
doinit(argc,argv)
int argc;
char **argv;
{
    extern int _arm_read_only;

    if (argc != 1 &&
	(argc != 2 || strncmp(argv[1],"readonly",strlen(argv[1])) != 0))
	rtxsh_usage();

    _arm_read_only = argc == 2;
    if (arm_init_comms(1,2) != -1)
	return;

    switch(armerrno) {
    case ARM_IN_USE:
	_arm_read_only = 1;
	rtxsh_warn(current_command, "Opening RTX read only\n");
    case LOST_CONNECTION:
    case NO_CONNECTION:
	rtxsh_warn(current_command, "Retry the connection\n");
	if (arm_init_comms(1,2) == -1) /* hehehe */
    default:
	    armwarn();
    }
}

void
top_rtx_restart_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("restart the IPs\n");
	break;
    case HELP_LONG:
	printf("%s:\n\trestart the IPs\n", cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
top_rtx_restart(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (arm_restart() == -1)
	armerror();

}

void
readonly_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s [off]\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("switch to/from read-only mode\n");
	break;
    case HELP_LONG:
	printf("%s [off]:\n\tswitch to/from read-only mode\n",
		cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
doreadonly(argc,argv)
int argc;
char **argv;
{
    if (argc != 1  &&
	    (argc != 2 || strncmp(argv[1],"off",strlen(argv[1])) != 0))
	rtxsh_usage();

    if (argc == 2) {
	if (arm_interrupt(RTX_CMD_READ_WRITE,(libio *) 0) == -1)
	    armerror();
    } else {
	if (arm_interrupt(RTX_CMD_READ_ONLY,(libio *) 0) == -1)
	    armerror();
    }
}

void
rtx_priv_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s [off]\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("switch to/from privileged mode\n");
	break;
    case HELP_LONG:
	printf("%s [off]:\n\tswitch to/from privileged mode\n",
		cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
rtx_priv(argc,argv)
int argc;
char **argv;
{
    if (argc != 1  &&
	    (argc != 2 || strncmp(argv[1],"off",strlen(argv[1])) != 0))
	rtxsh_usage();

    if (argc == 2) {
	if (arm_interrupt(RTX_CMD_NO_PRIV,(libio *) 0) == -1)
	    armerror();
    } else {
	if (arm_interrupt(RTX_CMD_PRIV,(libio *) 0) == -1)
	    armerror();
    }
}

static command stopmodes[] = {
    { "dead_stop" },
    { "ramp_stop" },
    { "free_stop" },
    { "free_off" },
};

#define NSTOPMODES	sizeof(stopmodes)/sizeof(command)

void
top_rtx_stop_help(cmd,l)
command *cmd;
int l;
{
    int i;

    switch(l) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s [mode]\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("stop the arm\n");
	break;
    case HELP_LONG:
	printf("%s [mode]\n\tstop the motors in mode [mode]\n",cmd->cmd_string);
	printf("\twith [mode] one of: ");
	for (i = 0; i < NSTOPMODES; i++)
	    printf(" %s", stopmodes[i].cmd_string);
	putchar('\n');
    }
}

/* ARGSUSED */
void
top_rtx_stop(argc,argv)
int argc;
char **argv;
{
    static automaton *modes = 0;
    int mode;

    if (!modes)
	modes = makeindexedtable(stopmodes,NSTOPMODES,"mode");

    if (argc == 1)
	mode = 0;
    else if (argc == 2)
	mode = commandindex(modes,argv[1]);
    else rtxsh_usage();

    printf("rtx_stop %d\n", mode);
    if (arm_stop(mode) == -1)
	armerror();
}

void
top_rtx_go_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	break;
    case HELP_SHORT:
	printf("issue go command for specified motors\n");
	break;
    case HELP_LONG:
	printf("%s [motor ...]\n", cmd->cmd_string);
	printf("\tissue the numeric go command to the listed motors,\n");
	printf("\tor all motors, if none are specified\n");
	printf("\t%s does error checking on the new possition\n",
		cmd->cmd_string);
	break;
    }
}

void
top_rtx_go(argc,argv)
int argc;
char **argv;
{
    int bits, i, errs = 0;
    if (argc == 1) {
	bits = 0x1555;
	for (i = 0; i < ZEDOWN; i++) {
	    int pos;
	    if (arm_read(i,NEW_POSITION,&pos) == -1)
		armerror();

	    if (!check_arm_constraint(i,pos)) {
		rtxsh_warn(current_command, "%d is an illegal position for %s\n", pos, motorname(i));
		errs++;
	    }
	}
    } else {
	bits = 0;
	for (i = 1; i < argc; i++) {
	    int motor = findmotor(argv[i]);
	    int pos;

	    if (arm_read(motor,NEW_POSITION,&pos) == -1)
		armerror();

	    if (!check_arm_constraint(motor,pos)) {
		rtxsh_warn(current_command, "%d is an illegal position for %s\n",
			pos, motorname(motor));
		errs++;
	    }
	    bits |= GO_BITS_FOR(motor,FWD);
	}
    }
    if (errs)
	return;
    if (arm_go(NUMERIC,bits) == -1)
	armerror();
}

void
rtx_status_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	break;
    case HELP_SHORT:
	printf("print status information\n");
	break;
    case HELP_LONG:
	printf("%s [motor ...]\n",cmd->cmd_string);
	printf("\twithout arguments - give general status\n");
	printf("\twith arguments - give status of specified motors\n");
	break;
    }
}

void
rtx_status(argc,argv)
int argc;
char **argv;
{
    int status;

    if (argc == 1) {
	if (arm_general_status(&status) == -1)
	    armerror();

	if (status & 0x1)
	    printf("Tasks in progress\n");
	if (status & 0x2)
	    printf("Some axis have stopped\n");
	if (status & 0x4)
	    printf("Some error limits exceeded\n");
	if (status & 0x8)
	    printf("Some axis have been reset\n");
	if (status & 0x10)
	    printf("User I/O changed since last read\n");
	printf("User I/O configuration: %sput\n", (status&0x20)?"in":"out");
    } else {
	int i, motor;
	for (i = 1; i < argc;i++) {
	    motor = findmotor(argv[i]);
	    if (arm_motor_status(motor,&status) == -1)
		armerror();

	    printf("%s:\n", motorname(motor));
	    if (status & 0x1)
		printf("\tTask in progress\n");
	    if (status & 0x2)
		printf("\tAxis stopped\n");
	    if (status & 0x4)
		printf("\tError limit exceeded\n");
	    if (status & 0x8)
		printf("\tAxis reset\n");
	    printf("\tMotor mode: %se, %s, %s\n",
		(status&0x10) ? "relativ" : "absolut",
		(status&0x20) ? "force" : "position",
		(status&0x40) ? "engaged" : "free");
	}
    }
}

void
vrtx_probe_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	break;
    case HELP_SHORT:
	printf("probe for the type and initialize the library.\n");
	break;
    case HELP_LONG:
	printf("%s [serial device]\n",cmd->cmd_string);
	printf("\twithout arguments - IPC\n");
	printf("\twith arguments - argument given to rtxd (typically /dev/ttyS0)\n");
	break;
    }
}

/* global pointer needed by all vrtx-functions */ 
static rtx_t *rtxp = NULL;

void
vrtx_probe(argc,argv)
int argc;
char **argv;
{
    char *which;

    if (argc == 1) {
        strcpy(which,"IPC");
    } else if (argc == 2) {
	strcpy(which, argv[1]);
    } else { 
	vrtx_probe_help();
	return;
    }
    rtxp = rtx_probe(which);

    if(rtxp == NULL) {
	printf("Can not talk to RTX on device '%s'\n",which);
    } else {
	printf("Received non-zero pointer '0x%08X' to private data\n",rtxp);
    }
    
}

void
vrtx_test_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	printf("Call probe to initiate the pointer to the library.\n");
	break;
    case HELP_SHORT:
	printf("test the status of the library and re-init when necessary.\n");
	break;
    case HELP_LONG:
	printf("%s\n",cmd->cmd_string);
	printf("\twithout arguments\n");
	break;
    }
}


void
vrtx_test(argc,argv)
int argc;
char **argv;
{
    int return_value;

    if (argc == 1) {
        if (rtxp == NULL)
		vrtx_test_help(NULL,HELP_USAGE);
    } else { 
	vrtx_test_help();
	return;
    }
    return_value = rtx_test(rtxp);

    if(return_value != 0) {
	printf("Library with pointer '0x%08X' gives error-code '%d'\n",rtxp, return_value);
    } else {
	printf("Library with pointer '0x%08X' successful tested\n",rtxp);
    }
    
}

void
vrtx_command_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	printf("Call probe to initiate the pointer to the library.\n");
	break;
    case HELP_SHORT:
	printf("Execute command.\n");
	break;
    case HELP_LONG:
	printf("%s [what param1 param2 param3 ...] (all integers) \n",cmd->cmd_string);
	printf("Examples:\n");
	printf("\tRTX_CMD_FIRST pid readonly uid: command -1 -1 0 123\n");
	break;
    }
}


void
vrtx_command(argc,argv)
int argc;
char **argv;
{
    libio tmpio; 

    if (rtxp == NULL) {
	vrtx_command_help(NULL,HELP_USAGE);
        return;
   }
    if (argc < 2 || argc > 5) {
	vrtx_command_help();
	return;
    }
    tmpio.what = htonl(atoi(argv[1])); /* this was wrong */
    tmpio.what = atoi(argv[1]);
    tmpio.params[0] = htonl(atoi(argv[2]));
    tmpio.params[0] = atoi(argv[2]);
    tmpio.params[1] = htonl(atoi(argv[3]));
    tmpio.params[1] = atoi(argv[3]);
    tmpio.params[2] = htonl(atoi(argv[4]));
    tmpio.params[2] = atoi(argv[4]);
    rtx_command(rtxp,&tmpio);

    if(tmpio.what != RTX_RESP_ACK) {
	printf("Command '%s' with id '%d' gives error-code '%s' with id '0x%02X'\n", rtxstrcommand(atoi(argv[1])), atoi(argv[1]), armstrerror(tmpio.what), ((unsigned int) tmpio.what));
    } else {
	printf("Command '%s' with id '%d' receives acknowledgment\n",rtxstrcommand(atoi(argv[1])), atoi(argv[1]));
    }
}

void
vrtx_finish_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	printf("Call probe to initiate the pointer to the library.\n");
	break;
    case HELP_SHORT:
	printf("finish processing and free pointer to the library.\n");
	break;
    case HELP_LONG:
	printf("%s\n",cmd->cmd_string);
	printf("\twithout arguments\n");
	break;
    }
}


void
vrtx_finish(argc,argv)
int argc;
char **argv;
{
    int return_value;

    if (argc == 1) {
        if (rtxp == NULL)
		vrtx_finish_help(NULL,HELP_USAGE);
    } else { 
	vrtx_finish_help();
	return;
    }
    return_value = rtx_finish(rtxp);

    if(return_value != 0) {
	printf("Library with pointer '0x%08X' gives error-code '%d'\n",rtxp, return_value);
    } else {
	printf("Library with pointer '0x%08X' successful finished\n",rtxp);
    }
    rtxp = NULL;
    
}

int
str2num(str)
char *str;
{
    long num, strtol();
    char *p;

    num = strtol(str,&p,0);
    if (*p || str == p)
	rtxsh_error(NULL,"%s: not a number\n",str);
    return (int) num;
}

/* ARGSUSED */
static void
indexerrfun(a,str,type)
automaton *a;
char *str;
caddr_t type;
{
    char **cmds = completecommand(a,str,1);

    if (!cmds)
	cmds = completecommand(a,"",1);

    commanderror(str,(char *) type,cmds);
}

automaton
*makeindexedtable(cmds,ncmds,name)
command *cmds;
int ncmds;
char *name;
{
    automaton *a = createautomaton(cmds,ncmds);
    if (a)
	adderrfun(a,indexerrfun,name);
    return a;
}

#ifndef RTXD_PATH
void
rtx_log()
{
}

void
putstatus()
{
}

void
nlist()
{
}
#endif /* ifndef RTXD_PATH */ 

