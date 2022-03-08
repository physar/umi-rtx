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
 * Parameter handling.
 * print and set.
 */

#include <stdio.h>
#include <string.h>
#include <rtx.h>
#include "states.h"
#include "globals.h"
#include "misc.h"

char *motorname();
char *paramname();

/* ARGSUSED */
void
rtx_print_help(cmd,l)
command *cmd;
int l;
{
    switch(l) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s motor ...\n",cmd->cmd_string);
	fprintf(stderr,"or     %s motor.param ...\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("print parameter(s) of (a) motor(s)\n");
	break;
    case HELP_LONG:
	printf("%s\n", cmd->cmd_string);
	printf("\twithout arguments, print all parameters of all motors\n");
	printf("\twith motors as argument, print all parameters of those motors\n");
	printf("\twith parameters as arguments, print those parameters\n");
	printf("\t\t (a parameter is specified as motorname.parameter\n");
    }
}

static
void
print_a_param(m,p)
int m,p;
{
    int res;
    if (arm_read(m,p,&res) == -1)
	armerror();
    
    printf("   %5d",res);
}

void
rtx_print(argc,argv)
int argc;
char **argv;
{
    int i;
    int motor, param;
    int dolong, doshort;
    int m[NUMBER_OF_MOTORS+1];
    char *motnames[NUMBER_OF_MOTORS+1];

    dolong = 0; doshort = 0;

    for (i = 1; i < argc; i++ ) {
	if (strchr(argv[i],'.'))
	    doshort++;
	else {
	    dolong++;
	    motor = findmotor(argv[i]);
	    argv[i] = motorname(motor);
	}
    }

    if (dolong && doshort)
	error("<motor.param> and <motor> don't mix\n");

    if (doshort) {
	for (i = 1; i < argc; i++ ) {
	    char *s = strchr(argv[i],'.');
	    int res;

	    *s++ = '\0';
	    motor = findmotor(argv[i]);
	    param = findparam(s);
	    if (arm_read(motor,param,&res) == -1)
		armerror();
	    printf("%s.%s= %5d\n",motorname(motor),paramname(param),res);
	}
    } else {
	if (!dolong) {
	    argc = NUMBER_OF_MOTORS;
	    for (i = 1; i < argc;i++) {
		m[i] = i-1;
		motnames[i] = motorname(i-1);
	    }
	    argv = motnames;
	} else {
	    if (argc > NUMBER_OF_MOTORS)
		argc = NUMBER_OF_MOTORS;
	    for (i = 1; i < argc;i++)
		m[i] = findmotor(argv[i]);
	}
	printf("%14.14s","");
	for (i = 1; i < argc; i++ )
	    printf(" %7.7s",argv[i]);
	putchar('\n');
	for (param = 0; param < NUMBER_OF_DATA_CODES;param++) {
	    printf("%-14.14s",paramname(param));
	    for (i = 1; i < argc; i++ )
		print_a_param(m[i],param);
	    putchar('\n');
	}
    }
}

void
rtx_set_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <parameter> = <value>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("sets specified parameter to value\n");
	break;
    case HELP_LONG:
	printf("%s <parameter> = <value>\n",cmd->cmd_string);
	printf("\tstore <value> (integer) in <parameter>\n");
	break;
    }
}

void
rtx_set(argc,argv)
int argc;
char **argv;
{
    char *m = 0, *p = 0, *v = 0;
    int motor, param;
    int val;

    switch(argc) {
    case 2:		/* x=y */
	v = strchr(argv[1],'=');
	if (!v) break;
	*v++ = '\0';
	m = argv[1];
	break;
    case 3:		/* x= y, or x =y */
	m = argv[1];
	v = argv[2];
	if (m[strlen(m)-1] == '=') {
	    m[strlen(m)-1] = '\0';
	} else if (*v == '=')
	    v++;
	else
	    v = m = 0;
	break;
    case 4:		/* x = y */
	if (argv[2][0] != '=' || argv[2][1])
	    break;
	m = argv[1];
	v = argv[3];
	break;
    }
    if (!m || !v || !*v)
	rtxsh_usage();

    p = strchr(m,'.');
    if (!p)
	rtxsh_error("%s: not a valid parameter specification\n", m);

    *p++ = '\0';
    motor = findmotor(m);
    param = findparam(p);
    val = str2num(v);
    if (arm_write(motor,param,(int) val) == -1)
	armerror();
}
