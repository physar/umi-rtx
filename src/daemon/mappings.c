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
 * All the name to number mappings for rtx
 */

#include "states.h"
#include "globals.h"
#include <stdio.h>
#include "names.h"


static command motornames[] = {
    { "elbow" },
    { "shoulder" },
    { "zed" },
    { "wrist1" },
    { "wrist2" },
    { "yaw" },
    { "gripper" },
    { "zedown" },
};

#define NMOTORNAMES (sizeof(motornames)/sizeof(command))

static automaton *motortable = 0;
static automaton *paramtable = 0;
static automaton *gomodetable = 0;
static automaton *motormodetable = 0;
static automaton *soaktable = 0;
static automaton *daemontable = 0;

int
findmotor(s)
char *s;
{
    return commandindex(motortable,s);
}

char
*motorname(motor)
{
    return motornames[motor].cmd_string;
}

char
**completemotor(str)
char *str;
{
    return completecommand(motortable,str,0);
}

static command paramnames[] = {
 { "error" },
 { "current_position" },
 { "error_limit" },
 { "new_position" },
 { "speed" },
 { "kp" },
 { "ki" },
 { "kd" },
 { "dead_band" },
 { "offset" },
 { "max_force" },
 { "current_force" },
 { "acceleration_time" },
 { "user_ram" },
 { "user_io" },
 { "actual_position" },
};

#define NPARAMNAMES (sizeof(paramnames)/sizeof(command))

int
findparam(s)
char *s;
{

    return commandindex(paramtable,s);
}

char
*paramname(param)
{
    return paramnames[param].cmd_string;
}

char
**completeparam(str)
char *str;
{
    return completecommand(paramtable,str,0);
}

static command gomodenames[] = {
 { "manual" },
 { "numeric" },
};

#define NGOMODENAMES (sizeof(gomodenames)/sizeof(command))

int
findgomode(str)
char *str;
{
    return commandindex(gomodetable,str);
}

char
**completegomode(str)
char *str;
{
    return completecommand(gomodetable,str,0);
}

static command motormodenames[] = {
 { "position" },
 { "force" },
 { "absolute" },
 { "relative" },
 { "user_input" },
 { "user_output" },
};

#define NMOTORMODENAMES (sizeof(motormodenames)/sizeof(command))

int
findmotormode(str)
char *str;
{
    return commandindex(motormodetable,str);
}

char
**completemotormode(str)
char *str;
{
    return completecommand(motormodetable,str,0);
}

static command soaknames[] = {
 { "init" },
 { "on" },
 { "init_soak" },
 { "off" },
};

#define NSOAK (sizeof(soaknames)/sizeof(command))

int
findsoak(str)
char *str;
{
    return commandindex(soaktable,str);
}

char
**completesoak(str)
char *str;
{
    return completecommand(soaktable,str,0);
}

static command daemonpar[] = {
 { "maxidle" },
 { "maxtime" },
};

#define NDAEMONPAR 	(sizeof(daemonpar)/sizeof(command))

char *
parname(i)
int i;
{
    return daemonpar[i].cmd_string;
}

int
parnumber(str)
char *str;
{
    return commandindex(daemontable,str);
}

int
numberofpar()
{
    return NDAEMONPAR;
}

char
**completepar(str)
char *str;
{
    return completecommand(daemontable,str,0);
}

void
init_names()
{
    if (!soaktable) {
	soaktable = makeindexedtable(soaknames,NSOAK,"soak mode");
	if (!soaktable)
	    error(current_command, "Out of memory\n");
    }
    if (!motortable) {
	motortable = makeindexedtable(motornames,NMOTORNAMES,"motor");
	if (!motortable)
	    error(current_command, "Out of memory\n");
	addalias(motortable,motornames+findmotor("zed"),"ze");
	addalias(motortable,motornames+findmotor("zed"),"z");
	addalias(motortable,motornames+findmotor("wrist1"),"w1");
	addalias(motortable,motornames+findmotor("wrist2"),"w2");
    }
    if (!paramtable) {
	paramtable = makeindexedtable(paramnames,NPARAMNAMES,"parameter");
	if (!paramtable)
	    error("Out of memory\n");
	addalias(paramtable,paramnames+1,"cp");
	addalias(paramtable,paramnames,"e");
	addalias(paramtable,paramnames,"er");
	addalias(paramtable,paramnames,"err");
	addalias(paramtable,paramnames+findparam("current_force"),"cf");
    }
    if (!gomodetable) {
	gomodetable = makeindexedtable(gomodenames,NGOMODENAMES,"go mode");
	if (!gomodetable)
	    error("Out of memory\n");
    }
    if (!motormodetable) {
	motormodetable = makeindexedtable(motormodenames,NMOTORMODENAMES,
		"motor mode");
	if (!motormodetable)
	    error("Out of memory\n");
    }
    if (!daemontable) {
	daemontable = makeindexedtable(daemonpar,NDAEMONPAR,"parameter");
	if (!daemontable)
	    error("Out of memory\n");
	addalias(daemontable,daemonpar+parnumber("maxidle"),"idle");
	addalias(daemontable,daemonpar+parnumber("maxtime"),"time");
    }
}
