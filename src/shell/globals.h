/*
 * $Id: globals.h,v 1.9 2006/06/21 11:21:22 arnoud Exp $
 *
 * The global definition file.
 */

#ifndef _globals_h
#define _globals_h

#include <setjmp.h>

#define HELP_USAGE	0
#define HELP_SHORT	1
#define HELP_LONG	2
#define HELP_COMPLETE	3

#define MAXREADLINE 4096
#define MAXARGS 128

#define HELP_WIDTH 16

typedef struct builtin {
    char *b_name;
    void (*b_fun)();
    void (*b_help)();
    struct commandtable *b_table;
} builtin;

#ifdef _SIGJBLEN
#define jmp_buf sigjmp_buf
#define setjmp(env) sigsetjmp(env, 1)
#define longjmp(env,x) siglongjmp(env, x)
#endif

typedef struct commandtable {
    builtin *ct_bltns;
    int ct_ncmds;
    char *ct_name, *ct_prompt;
    void (*ct_init)(), (*ct_help)();
    int (*ct_catchall)();  /* meta command line interpretation */
    automaton *ct_cmds;
    automaton *ct_aliases;
    struct commandtable *ct_up;
    jmp_buf ct_env;		/* were we'll return with return_to_toplevel */
    int	ct_envvalid;
} commandtable;

extern command *previous_command, *current_command, *getcommand();

extern commandtable *prompt_table, *top_table, toplevel;

automaton* makeindexedtable();

void rtxsh_usage(), rtxsh_error(command* cmd, ...), rtxsh_warn(command* cmd, ...), commanderror();
void readexecutecommands(), docommand(), setlevel(), enterlevel();
char **aliasexpand();

#define armerror()		rtxsh_error(current_command, "%s\n",armstrerror(armerrno))	
#define armwarn()		rtxsh_warn(current_command, "%s\n",armstrerror(armerrno))	

extern char *complete_arg, **complete_result, *(*complete_fun)();
extern int complete_ind, complete_free;

extern int (*check_exit)();

#ifdef READLINE
extern void init_readline();
extern void refresh_line();
extern void add_history(char *s);
#else
#define refresh_line() ((void) 0)
#endif

#endif /* _globals_h */
