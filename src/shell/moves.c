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
 * File defining the move operations on the arm.
 */

#include <stdio.h>
#include <string.h>
#include "rtx.h"
#include "states.h"
#include "globals.h"

struct constraint {
	int con_min, con_max;
} arm_constraints[NUMBER_OF_MOTORS] = {
	{ -2630,  2206 },	/* ELBOW */
	{ -2630,  2630 },	/* SHOULDER */
	{ -3303,     0 },	/* ZED */
#define PITCH	WRIST1
	{ -2642,   108 },	/* PITCH */
#define ROLL	WRIST2
	{ -3650,  4882 },	/* ROLL */
	{ -1071,  1071 },	/* YAW */
	{   -30,  1200 },	/* GRIP */
	{ -3303,     0 },	/* ZEDOWN */

};

/*
 * Check the constraints on arm movement
 */
int
check_arm_constraint(motor,pos)
int motor,pos;
{
    int curpos[NUMBER_OF_MOTORS];
    struct constraint consts[NUMBER_OF_MOTORS];
    int m;
    for (m = 0; m < ZEDOWN; m++) {
	consts[m] = arm_constraints[m];
	if (arm_read(m,CURRENT_POSITION,&curpos[m]) == -1)
	    armerror();
    }
    consts[YAW].con_min += curpos[ELBOW]/3;
    consts[YAW].con_max += curpos[ELBOW]/3;
    curpos[motor] = pos;
    if (motor == WRIST1 || motor == WRIST2) {
	int pitch, roll;
	pitch = curpos[WRIST1] + curpos[WRIST2];
	roll = curpos[WRIST1] - curpos[WRIST2];
	if (pitch < consts[PITCH].con_min || pitch > consts[PITCH].con_max ||
	    roll < consts[ROLL].con_min || roll > consts[ROLL].con_max)
		return 0;
    } else if (pos < consts[motor].con_min || pos > consts[motor].con_max)
	return 0;

    if ((motor == ZED || motor == ZEDOWN) && pos < -2767)
	warn(current_command, "Zed is moving into danger area\n");

    return 1;
}

void
move_arm_help(cmd,l)
command *cmd;
int l;
{
    char *s = "", *m = "";
    switch (*cmd->cmd_string) {
    case 'u': m = " (without checks)";
    case 'm': s = "absolute"; break;
    case 'r': s = "relative"; break;
	
    }
    switch(l) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <motor> [to] <position>\n", cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("move%s a motor to a (%s) position\n",m,s);
	break;
    case HELP_LONG:
	printf("%s <motor> [to] <position>\n", cmd->cmd_string);
	printf("\ta go command is issued to move <motor> to <position>\n");
	if (!m) printf("\tthe new position is first checked for validity\n");
	if (*s == 'r')
	    printf("\tthe movement is relative to the current position\n");
	break;
    }
}

static void
move_arm_s(argc,argv,rel, safe)
int argc;
char **argv;
int rel, safe;
{
    int num;
    int motor, cur;

    if (!(argc == 3 || (argc == 4 && strcmp(argv[2],"to") == 0)))
	rtxsh_usage();

    num = str2num(argv[argc-1]);

    motor = findmotor(argv[1]);
    if (motor < 0)
	return;
    if (rel) {
	if (arm_read(motor,CURRENT_POSITION,&cur) == -1)
	    armerror();
	num += cur;
    }
    if (safe && !check_arm_constraint(motor,num)) {
	char *motorname();
	error(current_command, "%d is an illegal position for %s\n", num, motorname(motor));
	return;
    }
    if (arm_write(motor,NEW_POSITION,num) == -1)
	armerror();

    if (arm_go(NUMERIC,GO_BITS_FOR(motor,FWD)) == -1)
	armerror();
}

void
move_arm_unsafe(argc,argv)
int argc;
char **argv;
{
    move_arm_s(argc,argv,0,0);
}

void
move_arm(argc,argv)
int argc;
char **argv;
{
    move_arm_s(argc,argv,0,1);
}

void
move_arm_rel(argc,argv)
int argc;
char **argv;
{
    move_arm_s(argc,argv,1,1);
}
