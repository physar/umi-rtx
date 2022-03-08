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
 * A shell skeleton.
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/signal.h>
#include <sys/file.h>
#include <setjmp.h>
#include <stdarg.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>
#include <unistd.h>

#ifdef READLINE
#include <readline/readline.h>
#endif

#if !defined  __USE_BSD && !defined __USE_GNU
extern char *sys_errlist[];
extern int sys_nerr;

#define strerror(num) \
	(((num) >= 0 && (num) < sys_nerr) ? sys_errlist[num] : "Unknown error")
#endif

#include "shell.h"
#include "states.h"
#include "globals.h"

void return_to_toplevel(), initlevel();
static void printerrprefix();

int (*check_exit)() = 0;

void dohelp(), help_help(), doquit(), quit_help(), makealias(), alias_help();
void unalias(), unalias_help(), doexit(), exit_help();
void dosource(), source_help();
void doup(), up_help();
static void toplevel_help();

extern commandtable toplevel;

static builtin basecmds[] = {
 { "help",		dohelp,			help_help },
 { "alias",		makealias,		alias_help },
 { "unalias",		unalias,		unalias_help },
#ifndef NOSOURCE
 { "source",		dosource,		source_help },
#endif
#ifndef ONELEVEL
 { "toplevel",		0,			toplevel_help,	&toplevel },
 { "up",		doup,			up_help },
 { "quit",		doquit,			quit_help },
#endif
 { "exit",		doexit,			exit_help },
 { "?",			dohelp,			help_help },
};

#define NBASECMDS (sizeof(basecmds)/sizeof(builtin))
/*
 * The baselevel defines the real builtins.
 */
commandtable baselevel = {
    basecmds, NBASECMDS,
};

/*
 * Pointer to the command structure of the command currently being executed
 * Commands should be able to nest (in this respect)
 */
command *current_command = 0;
/*
 * Pointer to the command executed last.  Needed for exit.
 */
command *previous_command = 0;
/*
 * The command table pointers
 * current_table	points to the table in which docommand is looking.
 *			this is the table linklevel links to.
 * prompt_table		points to the table which is active the toplevel for
 *			the user.
 * top_table		is the highest table which is recursively entered
 *			in linklevel()
 */

commandtable *current_table = 0, *prompt_table = &toplevel, *top_table = 0;

char **global_argv;
int global_argc;
char *current_file = 0;
int in_readline = 0;

static char *firstline = 0;

int
main(argc,argv)
int argc;
char **argv;
{
#if SVR4
    setvbuf(stderr, 0, _IOLBF, 0);
#else
    setlinebuf(stderr);
#endif

    seteuid(getuid());
    global_argc = argc;
    global_argv = argv;

#ifdef READLINE
    init_readline();
#endif

    (void) linklevel(&baselevel);
    (void) linklevel(&toplevel);

    if (global_argc == 1) {
	char *tmp = getenv("TTYPROMPT"),buf[MAXREADLINE];
	if (tmp && fgets(buf, sizeof(buf), stdin)) {
	    firstline = buf;
	    tmp = strchr(firstline,'\n');
	    if (tmp)
		*tmp = 0;
	}
	readexecutecommands(prompt_table,(char *) 0, 0);
    } else
	docommand(prompt_table,global_argc-1,global_argv+1);
    return 0;
}

/*
 * Read from in, prompt on out
 * if out == 0, then processing file. In that case abort function
 * when setjump returns (i.e. interrupt or something occured).
 *
 * This function is meant to be recursive on files, not on tables.
 * If a command changes the prompt_table, this is noted and the appropriate
 * action is taken.
 *
 */
void
readexecutecommands(table, infile, nonfatal)
commandtable *table;
char *infile;
int nonfatal;
{
    command *oldcmd = current_command;
    FILE *in = infile ? fopen(infile,"r") : stdin;
    int eof = 0, print_prompt;
    char *thename, *oldfile;
    commandtable *tmptbl;

    if (!in)
    {
	if (nonfatal)
	    return;
	else
	    rtxsh_error(current_command, "\"%s\": %s\n",infile,strerror(errno));
    }

    print_prompt = isatty(fileno(in));

    prompt_table = table;

    thename = infile ? strdup(infile) : "(stdin)";
    oldfile = current_file;

    do {

	table = prompt_table;

	if (eof || (setjmp(table->ct_env) && infile)) {
	    /* read error, end of input or error in source'd file */
	    table->ct_envvalid = 0;
	    current_file = oldfile;
	    current_command = oldcmd;
	    if (infile) {
		fclose(in);
		free(thename);
	    }
	    return;
	}

	table->ct_envvalid = 1;

	while (table == prompt_table && table->ct_envvalid) {
	    int argc;
	    char *s, buf[MAXREADLINE], *argv[MAXARGS];

	    current_command = oldcmd;
	    current_file = oldfile;

	    if (firstline) {
		s = firstline;
		firstline = 0;
#ifdef READLINE
		add_history(s);
#endif
		    
	    } else
#ifdef READLINE
	    /*
	     * Readline always reads from stdin.
	     */
	    if (print_prompt && in == stdin) {
		static char *line = 0;
		char *readline();

		if (line)
		    free(line);
		in_readline = 1;
		line = s = readline(table->ct_prompt ? table->ct_prompt : "");
		in_readline = 0;
		if (!s) {
		    doexit(1, (char**) 0);
		    clearerr(in);
		    break;
		}
		if (s && *s)
		    add_history(line);
	    } else
#endif
	    {
		if (print_prompt && table->ct_prompt) {
		    fputs(table->ct_prompt,stdout);
		    fflush(stdout);
		}
		if (!(s = fgets(buf,MAXREADLINE,in))) {
		    if (ferror(in))
			rtxsh_error(current_command, "\"%s\": %s\n",thename, strerror(errno));
		    if (in == stdin) {
			doexit(1, (char**) 0);
			clearerr(in);
		    } else 
			eof = 1;
		    break;
		}

		if (buf[strlen(buf)-1] != '\n')
		    rtxsh_error(current_command, "\"%s\": Line too long\n",thename);
	    }

	    argc = 0;

	    for (tmptbl = table; tmptbl; tmptbl = tmptbl->ct_up) {
		if (tmptbl->ct_catchall && (tmptbl->ct_catchall)(s))
		    break;
	    }
	    if (tmptbl)
		continue;

	    while (*s && argc < MAXARGS) {
		while (*s && isspace(*s))
		    *s++ = '\0';

		if (*s) {
		    if (argc >= MAXARGS)
			rtxsh_error(current_command, "\"%s\": Too many arguments\n",thename);
		    else
			argv[argc++] = s;
		}
		while (*s && !isspace(*s))
		    s++;
	    }

	    if (argc == 0 || argc >= MAXARGS || **argv == '#')
		continue;

	    current_file = infile ? thename : 0;

	    argv[argc] = 0;
	    top_table = 0;
	    docommand(table,argc,argv);
	    previous_command = current_command;
	}
	table->ct_envvalid = 0;
    } while (1);
}

void
docommand(table,argc,argv)
commandtable *table;
int argc;
char **argv;
{
    command *cmd;

    argv = aliasexpand(table,&argc,argv);

    if (!argv)
	return;

    current_table = table;
    current_command = cmd = findcommand(table->ct_cmds,argv[0]);

    if (cmd) { 
	builtin *b = (builtin*)cmd->cmd_val;
	if (b->b_fun)
	    (b->b_fun)(argc,argv);
	else if (b->b_table) {
	    if (linklevel(b->b_table) == -1) {
		if (b->b_table == &toplevel && b->b_table == prompt_table)
		    rtxsh_error(current_command, "already at toplevel\n");
		current_command = 0;
		goto uplevel;
	    }
	    if (argc == 1)
		prompt_table = b->b_table;
	    else
		docommand(b->b_table,argc-1,argv+1);
	} else
	    rtxsh_error(current_command, "nothing to do\n");
    } else {
	if (commanderr == CE_NOTFOUND) {
uplevel:
	    if (table->ct_up && table != top_table)
		docommand(table->ct_up,argc,argv);
	    else if (table != &baselevel)
		docommand(&baselevel,argc,argv);
	    else
		rtxsh_error(current_command, "%s: unknown command\n",argv[0]);
	} else {
	    char **res = completecommand(table->ct_cmds,argv[0],0);
	    fprintf(stderr,"%s: ambiguous command\n",argv[0]);
	    if (res) {
		fprintf(stderr,"Try one of:");
		while (*res)
		    fprintf(stderr," %s",*res++);
		putc('\n',stderr);
	    }
	    return_to_toplevel();
	}
    }
}

void
help_help(cmd,l)
command *cmd;
int l;
{
    char *completer(),**completion_matches();
    switch(l) {
    case HELP_USAGE:
	printf("Usage: %s [cmd ...] \n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("print help about commands\n");
	break;
    case HELP_LONG:
	printf("help\t\t- prints a short summary of all commands\n");
	printf("help cmd ...\t- gives help on the specified commands\n");
	break;
    case HELP_COMPLETE:
	complete_fun = completer;
	break;
    }
}

void
dohelp(argc,argv)
int argc;
char ** argv;
{
    int i;
    commandtable *table = top_table ? top_table : prompt_table;

    if (argc != 1) {
	for (i = 1; i < argc; i++) {
	    command *cmd = getcommand(table, argv[i]);
	    if (cmd) {
		builtin *b = (builtin*) cmd->cmd_val;
		if (b->b_help)
		    (b->b_help)(cmd,HELP_LONG);
		else if (b->b_table) {
		    char *name = b->b_table->ct_name && *b->b_table->ct_name ?
				    b->b_table->ct_name : cmd->cmd_string;
		    printf("%s [command [args]]:\n",name);
		    printf("\texecute command in or go to %s level\n", name);
		} else
		    printf("No help on %s\n",argv[i]);
	    } else if (commanderr == CE_NOTFOUND)
		rtxsh_error(current_command, "%s: no such command\n",argv[i]);
	    else
		rtxsh_error(current_command, "%s: ambiguous command\n",argv[i]);
	}
    } else {
	builtin *b = table->ct_bltns;
	if (table->ct_help)
	    (table->ct_help)(table);
	else
	    printf("\nCommands defined at the current level:\n\n");
	for (i = 0; i < table->ct_ncmds; i++) {
	    command *cmd =
		findcommand(table->ct_cmds,b[i].b_name);
	    printf("%-*.*s",HELP_WIDTH,HELP_WIDTH,
		table->ct_bltns[i].b_name);
	    if (b[i].b_help)
		(*b[i].b_help)(cmd,HELP_SHORT);
	    else if (b[i].b_table) {
		char *name = b[i].b_table->ct_name && *b[i].b_table->ct_name ?
				b[i].b_table->ct_name : cmd->cmd_string;
		printf("go to %s level\n",name);
	    } else
		printf("??\n");
	}
	/*
	 * help requested about specific level.
	 */
	if (top_table)
	    return;
#ifndef ONELEVEL
	printf("\nBuiltin commands:\n\n");
#endif
	table = &baselevel;
	b = table->ct_bltns;
	for (i = 0; i < table->ct_ncmds; i++) {
	    command *cmd =
		findcommand(table->ct_cmds,b[i].b_name);
	    printf("%-*.*s",HELP_WIDTH,HELP_WIDTH,
		table->ct_bltns[i].b_name);
	    if (b[i].b_help)
		(*b[i].b_help)(cmd,HELP_SHORT);
	    else if (b[i].b_table) {
		char *name = b[i].b_table->ct_name && *b[i].b_table->ct_name ?
				b[i].b_table->ct_name : cmd->cmd_string;
		printf("go to %s level\n",name);
	    }
	}
    }
}

#ifndef ONELEVEL
void
up_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	printf("%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("up one level\n");
	break;
    case HELP_LONG:
	printf("%s:\n\tup one level\n",
		cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
doup(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (prompt_table && prompt_table->ct_up
	&& prompt_table->ct_up != &baselevel)
	prompt_table = prompt_table->ct_up;
    else
	rtxsh_error(current_command, "at toplevel\n");
}

void
quit_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	printf("%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("up one level or exit to shell\n");
	break;
    case HELP_LONG:
	printf("%s:\n\tup one level or exit to shell (from top)\n",
		cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
doquit(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (prompt_table && prompt_table->ct_up
	&& prompt_table->ct_up != &baselevel)
	prompt_table = prompt_table->ct_up;
    else
	doexit(argc, argv);
}
#endif /* ONELEVEL */

void
exit_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE:
	fprintf(stderr,"%s accepts no arguments\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("exit to shell\n");
	break;
    case HELP_LONG:
	printf("%s:\n\texit to shell\n",cmd->cmd_string);
	break;
    }
}

/* ARGSUSED */
void
doexit(argc,argv)
int argc;
char **argv;
{
    if (argc != 1)
	rtxsh_usage();

    if (check_exit == 0 || (check_exit)(previous_command == current_command))
	exit(0);
}

void
rtxsh_usage()
{
    builtin *b;

    if (!current_command)
	return;

    b = (builtin *) current_command->cmd_val;
    if (b && b->b_help) {
	printerrprefix((command *) 0);
	(b->b_help)(current_command,HELP_USAGE);
    } else
	rtxsh_error(current_command, "incorrect usage\n");
    return_to_toplevel();
}


static void
printtableprefix(t)
commandtable *t;
{
    if (!t) {
	if (current_file)
	    fprintf(stderr,"\"%s\": ",current_file);
	return;
    }
    printtableprefix(t->ct_up);
    if (t->ct_name && *t->ct_name)
	fprintf(stderr,"%s: ",t->ct_name);
}

static void
printerrprefix(cmd)
command *cmd;
{
    printtableprefix(top_table ? top_table : current_table);
    if (cmd)
	fprintf(stderr,"%s: ", cmd->cmd_string);
}

/* VARARGS */
void
rtxsh_error(command *cmd, ...)
{
    va_list theargs;
    char *fmt;

    printerrprefix(cmd);

    va_start(theargs, cmd);
    fmt = va_arg(theargs, char *);
    vfprintf(stderr, fmt, theargs);
    return_to_toplevel();
}

/* VARARGS */
void
rtxsh_warn(command *cmd, ...)
{
    va_list theargs;
    char *fmt;

    printerrprefix(cmd);

    va_start(theargs, cmd);
    fmt = va_arg(theargs, char *);
    vfprintf(stderr, fmt, theargs);
}

/*
 * On error we return to the toplevel.
 */
void
return_to_toplevel()
{
    /*
     * We can only get here from a signal handler.
     * We must let readline do some cleaning up
     */
    refresh_line();
    if (in_readline)
	return;

    while (prompt_table && !prompt_table->ct_envvalid)
	prompt_table = prompt_table->ct_up;
    if (prompt_table)
	longjmp(prompt_table->ct_env, 1);
    /*
     * Else looks fatal. it can only happen before the initialisations
     * are done or if argc != 1.
     */
    else
	exit(1);
}

command
*getcommand(table,str)
commandtable *table;
char *str;
{
    command *cmd = 0;

    while (table && !cmd) {
	if (!table->ct_cmds)
	    initlevel(table);
	current_table = table;
	cmd = findcommand(table->ct_cmds,str);
	if (!cmd && commanderr == CE_AMBIGUOUS)
	    return 0;
	if (table == top_table) {
	    table = &baselevel;
	    continue;
	}
	table = table->ct_up;
    }
    return cmd;
}

void
commanderror(str,type,list)
char *str, *type, **list;
{
    int len;

    if (commanderr == CE_NOTFOUND)
	rtxsh_warn(current_command, "%s: invalid %s\n",str,type);
    else
	rtxsh_warn(current_command, "%s: ambiguous %s\n",str,type);
    if (list) {
	fprintf(stderr,"Try one of:");
	len = sizeof("Try one of:");
	while (*list) {
	    len += strlen(*list) + 1;
	    if (len >= 80) {
		fputs("\n\t",stderr);
		len = strlen(*list)+8;
	    } else
		putc(' ',stderr);
	    fprintf(stderr,"%s",*list++);
	}
	putc('\n',stderr);
    }
    return_to_toplevel();
}

#ifndef NOSOURCE
void
source_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s file\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("read commands from file\n");
	break;
    case HELP_LONG:
	printf("%s file:\n\tread commands from file\n",cmd->cmd_string);
	break;
    case HELP_COMPLETE:
	if (complete_ind == 1) {
	    char *filename_completion_function();

	    complete_fun = filename_completion_function;
	}
	break;
    }
}

void
dosource(argc,argv)
int argc;
char **argv;
{
    commandtable *table = prompt_table, *top = top_table;

    if (argc != 2)
	rtxsh_usage();

    top_table = 0;
    top = top ? top : table;
    readexecutecommands(top,argv[1], 0);

    if (!top_table || prompt_table == top) {
	prompt_table = table;
	top_table = top;
    }
    return;
}
#endif

void
initlevel(table)
commandtable *table;
{
    int i;
    command *commands = (command *) malloc(sizeof(command)*table->ct_ncmds);

    for (i = 0; i < table->ct_ncmds; i++) {
	commands[i].cmd_string = table->ct_bltns[i].b_name;
	commands[i].cmd_val = (caddr_t) &table->ct_bltns[i];
    }
    table->ct_cmds = createautomaton(commands,table->ct_ncmds);
    if (table->ct_init)
	(table->ct_init)(table);
}

/*
 * Set the level marker.
 * This will give a right structure when you enter go more than one level
 * deep in one command.
 * Make sure we don't create loops or move things up the chain.
 */
int
linklevel(table)
commandtable *table;
{
    commandtable *t = current_table;

    /*
     * this prevents users from `funny' level manipulations
     * you can remove this if you like.
     * Toplevel is the exception. So you can always say
     * toplevel <command>
     */
    if (table == prompt_table && table != &toplevel)
	return -1;

    while (t && t != table)
	t = t->ct_up;
    if (t)
	rtxsh_error(current_command, "SHELL error: linklevel loop\n");
    else
	table->ct_up = current_table;

    t = top_table ? top_table : prompt_table;
    while (t && t != table)
	t = t->ct_up;
    /*
     * if the table wasn't already in the list, it cannot have an active
     * stack frame
     */
    if (!t)
	table->ct_envvalid = 0;

    if (!table->ct_cmds)
	initlevel(table);

    current_table = top_table = table;

    return 0;
}

/* ARGSUSED */
static void
toplevel_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_SHORT:
	printf("return to toplevel or execute command at toplevel\n");
	break;
    case HELP_LONG:
	printf("%s [command [args]]:\n\texecute at/go to toplevel\n",
		cmd->cmd_string);
    }
}

#ifdef READLINE
void refresh_line()
{
#if 0
    /* current refresh asks for 2 arguments */
    if (in_readline)
	rl_refresh_line();
#endif
}
#endif
