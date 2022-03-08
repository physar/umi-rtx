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
 * The defintions for the automaton.
 */

#ifndef _states_h
#define _states_h

#include <sys/types.h>

extern int commanderr;
#define CE_AMBIGUOUS	1
#define CE_NOTFOUND	2

typedef struct command {
    char *cmd_string;
    union {
	caddr_t	CMD_val;
	int     CMD_int;
    } CMD_un;
} command;

#define cmd_val CMD_un.CMD_val
#define cmd_int CMD_un.CMD_int

typedef struct state {
    command *s_cmd;
    int s_lower, s_upper, s_count;
    union {
	struct state **S_states;
	struct state *S_one;
    } S_un;
    struct state *s_nul, *s_prev;
} state;

#define s_states	S_un.S_states
#define s_one		S_un.S_one

typedef struct automaton {
    state *au_start;
    command *au_base;
    int au_depth, au_ncmds;
    void  (*au_errfun)();
    caddr_t au_errarg;
    void  (*au_okfun)();
    caddr_t au_okarg;
} automaton;

automaton *addcommand(), *newautomaton(), *createautomaton(), *addalias(),
	*delcommand(), *delalias();

command *findcommand();
char **completecommand();
int commandindex();

#define addokfun(a,fun,arg) \
	(((a)->au_okfun = fun), ((a)->au_okarg = (caddr_t) arg), (a))

#define adderrfun(a,fun,arg) \
	(((a)->au_errfun = fun), ((a)->au_errarg = (caddr_t) arg), (a))

#endif /* _states_h */
