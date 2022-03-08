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
 * The alias functions.
 */

#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include "states.h"
#include "globals.h"

typedef struct alias {
    char **a_words;
    int a_size;
    int a_loop;
} alias;

static void
freealias(exp)
char **exp;
{
    char **o = exp;
    while (*exp)
	free(*exp++);
    free((char *) o);
}

char **
aliasexpand(table,argc,argv)
commandtable *table;
int *argc;
char **argv;
{
    static char **store = 0;
    char **freeit = store;
    char **tmp = argv;
    command *cp;
    alias *ap;
    int cnt = 0;
    automaton *aliases;

    if (!table || !table->ct_aliases)
	return argv;

    aliases = table->ct_aliases;

    while ((cp = findcommand(aliases,*tmp)) && cnt++ < 20) {
	int i;
	ap = (alias *) cp->cmd_val;
	store = (char **) malloc((ap->a_size+*argc-1) * sizeof(char*));
	for (i = 0; i < ap->a_size - 1; i++)
	    store[i] = strdup(ap->a_words[i]);
	for (;i < *argc + ap->a_size - 2; i++)
	    store[i] = strdup(tmp[i-ap->a_size+2]);
	store[i] = 0;
	*argc += ap->a_size - 2;
	if (freeit)
	    freealias(freeit);
	freeit = tmp = store;
	if (ap->a_loop)
	    return tmp;
    }
    if (cp) {
	fprintf(stderr,"Alias loop\n");
	return 0;
    }
    /* Not an alias - no expansion */
    if (commanderr == CE_NOTFOUND)
	return tmp;
    fprintf(stderr,"%s: ambiguous alias\n",*tmp);
    return 0;
}

static void
listalias(table,str)
commandtable *table;
char *str;
{
    char **sp;
    command *ap = findcommand(table->ct_aliases,str);
    if (!ap)
	if (commanderr == CE_NOTFOUND)
	    rtxsh_error("%s: not an alias\n", str);
	else
	    rtxsh_error("%s: ambiguous\n", str);

    sp = ((alias *)ap->cmd_val)->a_words;
    printf("%s aliased to", ap->cmd_string);
    while (*sp)
	printf(" %s",*sp++);
    putchar ('\n');
}

void
alias_help(cmd,kind)
command *cmd;
int kind;
{
    switch(kind) {
    case HELP_USAGE: break ; /* never */
    case HELP_SHORT:
	printf("list or make aliases\n");
	break;
    case HELP_LONG:
	printf("%s                    - list all aliases\n",cmd->cmd_string);
	printf("%s <alias>            - list the alias for <alias>\n",
		cmd->cmd_string);
	printf("%s <alias> <word ...> - make <alias> an alias for <word...>\n",
		cmd->cmd_string);
	break;
    }
}

void
makealias(argc,argv)
int argc;
char **argv;
{
    command *c;
    alias *ap;
    char **sp;
    automaton *aliases;
    commandtable *table = top_table ? top_table : prompt_table;

    if (!table)
	rtxsh_error("NO ENV?!?\n");

    aliases = table->ct_aliases;

    if (!aliases)
	table->ct_aliases = aliases = newautomaton();

    if (!aliases)
	rtxsh_error("Out of memory\n");

    if (argc == 1) {
	char **all;

	all = completecommand(aliases,"",0);
	if (!all)
	    return;
	while (*all)
	    listalias(table,*all++);
	return;
    }

    if (argc == 2)  {
	listalias(table,argv[1]);
	return;
    }

    if (strcmp(argv[1],"unalias") == 0)
	rtxsh_error("%s: Too dangerous to alias that\n",argv[1]);

    c = findcommand(aliases,argv[1]);
    if (c && strcmp(c->cmd_string,argv[1]) == 0) {
	int i;
	delcommand(aliases,c);
	ap = (alias *) c->cmd_val;
	for (i = 0; i < ap->a_size - 1; i++)
	    free(ap->a_words[i]);
	free((char*) ap->a_words);
    } else {
	c = (command *) malloc(sizeof(command));
	ap = (alias *) malloc(sizeof(alias));
	if (c) {
	    c->cmd_string = strdup(argv[1]);
	}
    }
    sp = (char **) malloc(argc * sizeof(char *));
    if (!ap || !sp || !c || !c->cmd_string)
	rtxsh_error("Out of memory\n");

    ap->a_words = sp;
    ap->a_loop = !strcmp(argv[1],argv[2]);
    argv++;
    while (*++argv)
	*sp++ = strdup(*argv);
    *sp = 0;
    ap->a_size = argc - 1; /* sub 1 for alias, 1 for name, add 1 for NULL */
    c->cmd_val = (caddr_t) ap;
    addcommand(aliases,c);
}

void
unalias_help(cmd,kind)
command *cmd;
int kind;
{
    switch (kind) {
    case HELP_USAGE:
	fprintf(stderr,"Usage: %s <alias>\n",cmd->cmd_string);
	break;
    case HELP_SHORT:
	printf("remove an alias\n");
	break;
    case HELP_LONG:
	printf("%s <alias>\n",cmd->cmd_string);
	printf("\tremove alias <alias>\n");
	break;
    }
}

void
unalias(argc,argv)
int argc;
char **argv;
{
    command *ap;
    automaton *aliases;
    commandtable *table = top_table ? top_table : prompt_table;

    if (argc != 2)
	rtxsh_usage();

    if (!table)
	rtxsh_error("NO ENV?!?\n");

    aliases = table->ct_aliases;

    if (!aliases || !(ap = findcommand(aliases,argv[1])))
	rtxsh_error("%s: not an alias\n",argv[1]);

    delcommand(aliases,ap);
}
