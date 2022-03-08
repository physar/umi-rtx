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
 * A state switcher for strings.
 *
 * functions defined in this module:
 *
 *	automaton *newautomaton();
 *		- create empty automaton.
 *	
 *	automaton *addcommand(automaton *, command *)
 *	automaton *addalias(automaton *, command *, char *)
 *		- add a string to an automaton.
 *		- the first function uses cmd_string as entry,
 *		  the second char *
 *	
 *	automaton *createautomaton(command *,int)
 *		- process an entire array of cmds.
 *		- store the first element, so we can compute the index
 *		- in commandindex()
 *
 *	automaton *delcommand(automaton *, command *)
 *	automaton *delalias(automaton *, char *)
 *		- deletes a command or alias
 *
 *	command *findcommand(automaton *, char *)
 *		- fetch a command
 *
 *	char **completecommand(automaton *, char *, int)
 *		- return all possible matches of command argument.
 *		- flag int gives aliases or not.
 *
 *	int commandindex(a,str)
 *		- returns the index of a certain command.
 *
 *	macro defined in states.h:
 *
 *	adderrfun(automaton *,void (*)(),caddr_t arg)
 *		- function called when the command isn't found.
 *	addokfun(automaton *,void (*)(),caddr_t arg)
 *		- function called when the command is found.
 *
 *		  function called as:
 *		- fun(automaton *, char *, caddr_t)
 *		
 *
 */

#include <sys/types.h>
#include <malloc.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "states.h"

static state *findstate();

static state *
newstate(prev,cmd)
state *prev;
command *cmd;
{
    state *s = (state *) malloc(sizeof(state));
    if (!s)
	return 0;
    s->s_cmd = cmd;
    s->s_states = 0;
    s->s_nul = 0;
    s->s_lower = 0;
    s->s_upper = 0;
    s->s_count = 0;
    s->s_prev = prev;
    return s;
}

automaton
*newautomaton()
{
    automaton *new = (automaton *) malloc(sizeof(automaton));
    if (!new)
	return 0;
    new->au_start =  newstate((state *) 0, (command *) 0);
    new->au_okfun = new->au_errfun = 0;
    new->au_okarg = new->au_errarg = 0;
    new->au_base = 0;
    new->au_ncmds = new->au_depth = 0;

    return new;
}

#define gettransition(s,c) \
	(((c) < (s)->s_lower || (c) > (s)->s_upper) ? 0 \
	: (s)->s_lower == (s)->s_upper ? (s)->s_one : \
	(s)->s_states[(c)-(s)->s_lower])

/*
 * If one state is in use by more than one cmd,
 * the cmd* is 0 (unless both commands have same command *).
 */
static state *
addtransition(s,c,cmd)
state *s;
char c;
command *cmd;
{
    int t = c & 0xff;
    state *s2;

    if (t == 0) {
	/* A command was added twice. This is an error */
	if (s->s_nul)
	    return 0;
	s->s_nul = newstate(s,cmd);
	s = s->s_nul;
	if (!s)
	    return 0;
	return s;
    }
    s2 = gettransition(s,t);
    if (s2) {
	if (cmd != s2->s_cmd) s2->s_cmd = 0;
	return s2;
    }
    s2 = newstate(s,cmd);
    if (!s2)
	return 0;
    s->s_count++;
    /*
     * The most common case: only one character is handled by putting it
     * in the state itself.
     */
    if (s->s_lower == 0) {
	s->s_upper = s->s_lower = t;
	return s->s_one = s2;
    }
    if (t >= s->s_lower && t <= s->s_upper)
	return s->s_states[t-s->s_lower] = s2;
    else {
	int delta,i,size = s->s_upper - s->s_lower + 1;
	state **newstates;
	if (size == 1) { /* first allocation */
	    delta = t - s->s_lower;
	    if (delta < 0) delta = -delta;
	    delta++;
	    newstates = (state **) malloc(delta * sizeof(state*));
	    if (!newstates)
		return 0;
	    /*
	     * don't do first and last, one is the old state,
	     * the other the new state
	     */
	    for (i = 0; ++i < delta;)
		newstates[i] = 0;
	    if (t > s->s_lower) {
		s->s_upper = t;
		newstates[delta-1] = s2;
		newstates[0] = s->s_one;
	    } else {
		s->s_lower = t;
		newstates[0] = s2;
		newstates[delta-1] = s->s_one;
	    }
	} else if (t < s->s_lower) {
	    /*
	     * We must shift the old state ptr's
	     */
	    delta = s->s_lower - t;
	    newstates = (state **) realloc((char*) s->s_states,
				    (delta + size) * sizeof(state *));
	    if (!newstates)
		return 0;
	    for (i = size; i-- > 0;)
		newstates[i+delta] = newstates[i];
	    for (i = 1; i < delta; i++)
		newstates[i] = 0;
	    newstates[0] = s2;
	    s->s_lower = t;
	} else {
	    delta = t - s->s_upper;
	    newstates = (state **) realloc((char*) s->s_states,
				    (delta + size) * sizeof(state *));
	    if (!newstates)
		return 0;
	    for (i = size; i < size+delta-1; i++)
		newstates[i] = 0;
	    newstates[size+delta-1] = s2;
	    s->s_upper = t;
	}
	s->s_states = newstates;
	return s2;
    }
}

automaton
*addalias(a,c,str)
automaton *a;
command *c;
char *str;
{
    state *s;
    int len;

    if (!str || !*str)
	return 0;
    if (!a) {
	a = newautomaton();
	if (!a)
	    return 0;
    }

    if (a->au_base) {
	s = findstate(a,c->cmd_string);
	if (!s || !s->s_nul || s->s_nul->s_cmd != c)
	    a->au_base = 0; /* arnoud@science.uva.nl: was == , which seems to be a bug */
    }

    len = strlen(str) + 1;

    if (len > a->au_depth)
	 a->au_depth = len;

    s = a->au_start;
    do {
	s = addtransition(s,*str,c);
    } while (s && *str++);

    if (!s)
	return 0;

    a->au_ncmds ++;
    return a;
}

automaton
*addcommand(a,c)
automaton *a;
command *c;
{
    return addalias(a,c,c->cmd_string);
}

automaton
*createautomaton(cmds,ncmds)
command *cmds;
int ncmds;
{
    int i;
    automaton *a = newautomaton();

    if (!a)
	return 0;

    for (i = 0; i < ncmds; i++)
	if (!addcommand(a,&cmds[i]))
	    return 0;

    a->au_base = cmds;

    return a;
}

automaton
*delalias(a,str)
automaton *a;
char *str;
{
    state *s, *sp;

    if (!a)
	return 0;

    s = findstate(a,str);
    /*
     * A nul state must follow this state.
     */
    if (!s || !s->s_nul)
	return 0;

    free((char *) s->s_nul);
    s->s_nul = 0;
    a->au_ncmds--;
    /*
     * We walk to the root. If count = 0 and there is no nul state
     * (And this isn't the root) we remove the reference to the
     * state above us.
     */
    while (s->s_count == 0 && s->s_nul == 0 && s->s_prev != 0) {
	sp = s->s_prev;
	if (--sp->s_count) {
	    int i, ind = -1;
	    state *other;
	    for (i = 0; i <= sp->s_upper - sp->s_lower;i++) {
		if (sp->s_states[i] == s)
		    sp->s_states[i] = 0;
		else if (sp->s_states[i])
		    ind = i;
	    }
	    if (sp->s_count == 1) {
		if (ind == -1) {
		    fprintf(stderr,"Oops: delcommand\n");
		    abort();
		}
		/* UNION! save old value first */
		other = sp->s_states[ind];
		free((char*) sp->s_states);
		sp->s_one = other;
		sp->s_lower = sp->s_upper = sp->s_lower + ind;
	    }
	} else {
	    sp->s_one = 0;
	    sp->s_upper = sp->s_lower = 0;
	}
	free((char *) s);
	s = sp;
    }
    /*
     * We stopped because there was a command that was a substring of the
     * command we had. Propagate it's cmd value to the last state visited
     * this command can now use shorter abbreviation
     */
    if (s->s_count == 0 && s->s_prev) {
	s->s_cmd = s->s_nul->s_cmd;
	s = s->s_prev;
    }
    /*
     * Propagate command value, if this is a unique path.
     * Recognized by no nul branches and always one follow state.
     * Don't do this to the start state!
     */
    if (s->s_count == 1 && s->s_one->s_cmd) {
	sp = s;
	while (sp && !sp->s_nul && sp->s_count == 1 && sp->s_prev) {
	    sp->s_cmd = s->s_one->s_cmd;
	    sp = sp->s_prev;
	}
    }
    return a;
}

automaton *
delcommand(a,cmd)
automaton *a;
command *cmd;
{
    return delalias(a,cmd->cmd_string);
}

/*
 * This function wil return the command * on succes.
 * 0 on failure.
 */

int commanderr;

static state *
findstate(a,str)
automaton *a;
char *str;
{
    state *s = a->au_start;
    char *p = str;

    while (s && *p) {
	s = gettransition(s,*p); p++; /* MACRO! */
    }
    return s;
}

command *
findcommand(a,str)
automaton *a;
char *str;
{
    state *s = findstate(a,str);
    command *cmd;

    if (!s)
	commanderr = CE_NOTFOUND;
    else if (s->s_nul)
	s = s->s_nul;
    else if (!s->s_cmd)
	commanderr = CE_AMBIGUOUS;

    cmd = s ? s->s_cmd : 0;

    if (cmd) {
	if (a->au_okfun)
	    (a->au_okfun)(a,str,a->au_okarg);
    } else {
	if (a->au_errfun)
	    (a->au_errfun)(a,str,a->au_errarg);
    }
    return cmd;
}

/*
 * Walk states. Construct the string. This is slow.
 * Built a string in the first free string of the array.
 * If a string is completed copy it to the next free element.
 */
static
char **
completecommandr(result,ind,s,newind,alias)
char **result;
int *ind;
state *s;
int newind, alias;
{
    int j;

    if (!s)
	return result;

    if (s->s_nul) {
	result[*ind][newind] = '\0';
	if (alias || !strcmp(result[*ind], s->s_nul->s_cmd->cmd_string)) {
	    strcpy(result[*ind+1],result[*ind]);
	    (*ind)++;
	}
	if (!s->s_lower)
	    return result;
    }
    if (!s->s_lower)
	return 0;
    if (s->s_lower == s->s_upper) {
	result[*ind][newind] = s->s_lower;
	return completecommandr(result,ind,s->s_one,newind+1,alias);
    }
    for (j = 0; j <= s->s_upper - s->s_lower; j++) {
	if (s->s_states[j] == 0)
	    continue;
	result[*ind][newind] = s->s_lower + j;
	if (!completecommandr(result,ind,s->s_states[j],newind+1,alias))
	    return 0;
    }
    return result;
}

char **
completecommand(a,str,alias)
automaton *a;
char *str;
int alias;
{
    static char **result = 0, *store = 0;
    static int asize = 0, csize = 0;
    state *s = findstate(a,str);
    int i;
    char *p;

    if (!s || a->au_depth == 0)
	return 0;

    /*
     * Don't use realloc. We'll overwrite the contents anyway.
     */
    if (asize <= a->au_ncmds) {
	asize = a->au_ncmds+1;
	if (result) free((char*) result);
	result = (char **) malloc(asize * sizeof(char *));
    }
    if (csize < asize * a->au_depth) {
	csize = asize * a->au_depth;
	if (store) free(store);
	store = malloc(csize * sizeof(char));
    }
    p = store;
    for (i = 0; i < asize; i++, p+=a->au_depth)
	result[i] = p;

    i = 0;
    strcpy(result[0],str);
    if (!completecommandr(result,&i,s,strlen(str),alias))
	return 0;
    result[i] = 0;
    return i == 0 ? 0 : result;
}

int
commandindex(a,str)
automaton *a;
char *str;
{
    command *cmd = findcommand(a,str);
    if (a->au_base == 0 || !cmd)
	return -1;
    else
	return cmd - a->au_base;
}
