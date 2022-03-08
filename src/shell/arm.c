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
 * Raw command line interface to all arm commands
 */

#include <stdio.h>
#include <string.h>
#include "rtx.h"
#include "states.h"
#include "globals.h"
#include "armlib.h"
#include "names.h"
#include "misc.h"

char **completemotormode(), **completegomode(), **completemotor(),
    **completesoak(), **completeparam();

void doinit_comms(), init_comms_help();
void doversion(), version_help();
void top_rtx_restart(), top_rtx_restart_help();
void dodefine_origin(), define_origin_help();
void doreload_pids(), reload_pids_help();
void doset_mode(), set_mode_help();
void doread(), read_help();
void dowrite(), write_help();
void dogo(), go_help();
void dointerpolate(), interpolate_help();
void dogeneral_status(), general_status_help();
void domotor_status(), motor_status_help();
void top_rtx_stop(), top_rtx_stop_help();
void dosoak(), soak_help();
void doraw(), raw_help();
void arm_level_help();

builtin armbuiltins[] = {
 { "init_comms",	doinit_comms,		init_comms_help, },
 { "version",		doversion,		version_help, },
 { "restart",		top_rtx_restart,	top_rtx_restart_help, },
 { "define_origin",	dodefine_origin,	define_origin_help, },
 { "reload_pids",	doreload_pids,		reload_pids_help, },
 { "set_mode",		doset_mode,		set_mode_help, },
 { "read",		doread,			read_help, },
 { "write",		dowrite,		write_help, },
 { "go",		dogo,			go_help, },
 { "interpolate",	dointerpolate,		interpolate_help, },
 { "general_status",	dogeneral_status,	general_status_help, },
 { "motor_status",	domotor_status,		motor_status_help, },
 { "stop",		top_rtx_stop,		top_rtx_stop_help, },
 { "soak",		dosoak,			soak_help, },
 { "raw",		doraw,			raw_help, },
};

commandtable armlevel = {
    armbuiltins, sizeof(armbuiltins)/sizeof(builtin),
    "arm", "arm> ",
    0, arm_level_help,
};

void
define_origin_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("current position becomes home position\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	fprintf(stderr,"[no arguments]\n");
	break;
    }
}

/* ARGSUSED */
void
dodefine_origin(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();
    
    if (arm_define_origin() == -1)
	armerror();
}

void
init_comms_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <toggle mode> <debug level>\n",
	    cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("initialise communication with daemon and robot\n");
	break;
    case HELP_LONG:
	break;
    }
}
void
doinit_comms(argc,argv)
int argc;
char **argv;
{
    if (argc != 3)
	rtxsh_usage();

    if (arm_init_comms(str2num(argv[1]),str2num(argv[2])) == -1)
	armerror();
}

void
version_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("print software and firmware versions\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	fprintf(stderr,"[no arguments]\n");
	break;
    }
}
/* ARGSUSED */
void
doversion(argc,argv)
int argc;
char **argv;
{
    int version[3];

    if (argc != 1)
	rtxsh_usage();

    if (arm_version(version) == -1)
	armerror();
    printf("Shell version: $Revision$\n");
    printf("Driver version %d.%d\n",version[0],version[1]);
    printf("Robot version %d.%d\n",(version[2]>>8)&0xff,version[2]&0xff);
}

void
reload_pids_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("reset all servo loop control parameters\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	fprintf(stderr,"[no arguments]\n");
	break;
    }
}
/* ARGSUSED */
void
doreload_pids(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (arm_reload_pids() == -1)
	armerror();
}

void
set_mode_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <motor> <mode>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("set motor in mode\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completemotor(complete_arg);
	else if (complete_ind == 2)
	    complete_result = completemotormode(complete_arg);
	break;
    }
}
void
doset_mode(argc,argv)
int argc;
char **argv;
{
    if (argc != 3)
	rtxsh_usage();

    if (arm_set_mode(findmotor(argv[1]),findmotormode(argv[2])) == -1)
	armerror();
}

void
read_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <motor> <parameter>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("print value of specified parameter\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completemotor(complete_arg);
	else if (complete_ind == 2)
	    complete_result = completeparam(complete_arg);
	break;
    }
}
void
doread(argc,argv)
int argc;
char **argv;
{
    int value;

    if (argc != 3)
	rtxsh_usage();

    if (arm_read(findmotor(argv[1]),findparam(argv[2]),&value) == -1)
	armerror();
    printf("%s.%s = %d\n",argv[1],argv[2],value);
}

void
write_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <motor> <parameter> <value>\n",
	    cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("set specified control parameter\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completemotor(complete_arg);
	else if (complete_ind == 2)
	    complete_result = completeparam(complete_arg);
	else if (complete_ind == 3)
	    fprintf(stderr,"<numeric>\n");
	break;
    }
}

void
dowrite(argc,argv)
int argc;
char **argv;
{
    if (argc != 4)
	rtxsh_usage();

    if (arm_write(findmotor(argv[1]),findparam(argv[2]),str2num(argv[3])) == -1)
	armerror();
}

void
go_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <go-mode> <go-bits>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("start one or more motors\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completegomode(complete_arg);
	else if (complete_ind == 2)
	    fprintf(stderr,"<go-bits>\n");
	break;
    }
}

void
dogo(argc,argv)
int argc;
char **argv;
{
    if (argc != 3)
	rtxsh_usage();

    if (arm_go(findgomode(argv[1]),str2num(argv[2])) == -1)
	armerror();
}

void
interpolate_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s (7 offsets)\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("move smoothly\n");
	break;
    case HELP_LONG:
	break;
    }
}

void
dointerpolate(argc,argv)
int argc;
char **argv;
{
    int data[NUMBER_OF_MOTORS-1], i;

    if (argc != NUMBER_OF_MOTORS)
	rtxsh_usage();

    for (i = 0; i < ZEDOWN; i++)
	data[i] = str2num(argv[i+1]);

    if (arm_interpolate(data) == -1)
	armerror();
}

void
general_status_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("print general status\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	fprintf(stderr,"[no arguments]\n");
	break;
    }
}

/* ARGSUSED */
void
dogeneral_status(argc,argv)
int argc;
char **argv;
{
    int status;

    if (argc != 1)
	rtxsh_usage();

    if (arm_general_status(&status) == -1)
	armerror();
    printf("status = %x\n", status);
}

void
motor_status_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <motor>\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("get status of a motor\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completemotor(complete_arg);
	break;
    }
}

void
domotor_status(argc,argv)
int argc;
char **argv;
{
    int status;

    if (argc != 2)
	rtxsh_usage();

    if (arm_motor_status(findmotor(argv[1]),&status) == -1)
	armerror();
    printf("status of %s = %x\n", argv[1], status);
}

void
soak_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <soak-mode>\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("start/stop soak testing\n");
	break;
    case HELP_LONG:
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1)
	    complete_result = completesoak(complete_arg);
	break;
    }
}

void
dosoak(argc,argv)
int argc;
char **argv;
{
    if (argc != 2)
	rtxsh_usage();

    if (arm_soak(findsoak(argv[1])) == -1)
	armerror();
}

void
raw_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <ip#> <cmdbyte> [<param1> <param2>]\n",
	    cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("issue raw command\n");
	break;
    case HELP_LONG:
	break;
    }
}

void
doraw(argc,argv)
int argc;
char **argv;
{
    int nbytes;
    int b1, b2, b3, len, i, resp[3];

    if (argc < 3 || argc > 5)
	rtxsh_usage();

    nbytes = argc - 2;
    switch (nbytes) {
    case 3:
	b3 = str2num(argv[4]);
    case 2:
	b2 = str2num(argv[3]);
    case 1:
	b1 = str2num(argv[2]);
    }
    if (arm_raw(str2num(argv[1]),nbytes,b1,b2,b3,&len,resp) == -1)
	armerror();
    printf("result:");
    for (i = 0; i < len; i++)
	printf(" %02x",resp[i]);
    putchar('\n');
}

/*ARGSUSED*/
void
arm_level_help()
{
    printf("\n%s\n%s\n%s\n%s\n%s\n\n",
    "This level contains the commandline version of the library.",
    "A function can be called by passing all its value parameters",
    "on the commandline.",
    "The result parameters, if any, will be printed.",
    "Consult `Programming RTX using the library' for details."
    );
}
