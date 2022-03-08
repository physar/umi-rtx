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
 * The shell routines used for command completion and .
 *
 * Commands in sublevels are also completed.
 * Listing of toplevel commands does not show all available commands,
 * only those in the level just entered.
 * When directly at the prompt, all commands are shown.
 * Argument completion is attempted (a bit of a hack)
 */

/*
 * complete_arg store the argument to be completed.
 * complete_result stores the result.
 * complete_ind is the index in the argument list
 * complete_free is a flag that indicates whether the resulting strings
 *	can be freed or not.
 */
char *complete_arg, **complete_result, *(*complete_fun)();
int complete_ind, complete_free;

#ifdef READLINE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include <readline/readline.h>
#include <readline/history.h>

#include "shell.h"
#include "states.h"
#include "globals.h"

extern commandtable baselevel;


void
init_readline()
{
    char **command_completion(), *complete_argument();

    rl_readline_name = toplevel.ct_name;
    rl_attempted_completion_function = (Function *)command_completion;
    rl_completion_entry_function = (Function *) complete_argument;
}

char
**command_completion(text,start,end)
char *text;
int start, end;
{
    char **completion_matches(), *completer();
    char *s = rl_line_buffer;

    while (*s && isspace(*s))
	s++;

    top_table = 0;

    if (start == s - rl_line_buffer)
	return completion_matches(text,completer);
    else
	return NULL;
}

char *completer(text,st)
char *text;
int st;
{
    static int ind;
    static char **result = 0;
    static commandtable *table;

    if (st == 0 || (result && !result[ind])) {
	if (st == 0)
	    table = top_table ? top_table : prompt_table;
	result = 0;
	ind = 0;
	while (table && !result) {
	    result = completecommand(table->ct_cmds,text,0);
	    if (result) {
		char **res, **pp;
		res = pp = result;
		do {
		    command *cmd;
		    cmd = findcommand(table->ct_cmds,*pp);
		    if (!cmd ||
			(((builtin *)(cmd->cmd_val))->b_table != prompt_table
			 || prompt_table == &toplevel))
			*res++ = *pp++;
		    else
			pp++;
		} while(*pp);
		*res = 0;
		if (!*result)
		    result = 0;
	    }
	    if (table == top_table)
		table = &baselevel;
	    else
		table = table->ct_up;
	}
    }
    if (!result || !result[ind])
	return 0;

    return strdup(result[ind++]);
}

char *complete_argument(text,st)
char *text;
int st;
{
    static char *line = 0;
    static char **result = 0;
    static int ind, freeit;

    char *s = rl_line_buffer, *p;
    int inspace = 1;
    command *cmd;
    builtin *b;


    if (st == 0) {
	int argc = 0;
	char *argv[MAXARGS];
	char **av = argv;
	commandtable *table = prompt_table;

	if (freeit && result)
	    free((char*) result);

	result = 0; ind = 0;
	if (line)
	    free(line);
	p = line = malloc(rl_end+1);
	if (!p)
	    return 0;

	strncpy(p,rl_line_buffer,rl_end+1);

	complete_ind = 0;
	while (*s && s - rl_line_buffer < rl_point && argc < MAXARGS) {
	    if (isspace(*s)) {
		*p = '\0';
		if (!inspace) {
		    inspace = 1;
		    complete_ind++;
		}
	    } else {
		if (inspace) {
		    argv[argc++] = p;
		    inspace = 0;
		}
	    }
	    s++;
	    p++;
	}
	if (complete_ind == argc) {
	    argv[argc++] = "";
	}
	if (argc >= MAXARGS)
	    return 0;

	argv[argc] = 0;

	if (complete_ind == 0 || **argv == '#' || **argv == '!')
	    return 0;

	top_table = 0;

	while (argc > 1 && (cmd = getcommand(table,av[0]))) {
	    b = (builtin *) cmd->cmd_val;
	    if (b->b_table) {
		if (linklevel(b->b_table) == -1)
		    return 0;
		table = b->b_table;
		av++,argc--;
	    } else
		break;
	}
	if (!cmd && (commanderr == CE_NOTFOUND || argc != 1))
		return 0;

	complete_free = 0;
	complete_fun = 0;
	if (argc == 1) {
	    result = completecommand(table->ct_cmds,text,0);
	    if (!result)
		result = completecommand(baselevel.ct_cmds,text,0);
	} else {
	    if (b->b_help) {
		complete_arg = text;
		complete_result = 0;
		complete_ind = argc - 1;
		(b->b_help)(cmd,HELP_COMPLETE);
		result = complete_result;
	    }
#if 0
/* arnoud, Dec 2003, refresh needs two arguments on linux and modern unix */
	    if (!result && !complete_fun) {
		rl_refresh_line();
	    }
#endif
	}
	freeit = complete_free;
    }

    if (complete_fun)
	return (complete_fun)(text,st);

    if (result == 0 || result[ind] == 0)
	return 0;
    
    return freeit ? result[ind++] : strdup(result[ind++]);
}
#else /* ifdef readline */
char *completer
() { return 0; }
char **command_completion
() { return 0; }
#endif /* ifdef readline */
