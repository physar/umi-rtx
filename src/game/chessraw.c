/**************************************************************************/
/* chessraw.c                                                   /\/\      */
/* Version 2.0   --  December 1994                              \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam                      _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Dept. of Computer Systems                        | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*         arnoud@fwi.uva.nl, dam@fwi.uva.nl                | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/* Release note 2.0: Combined send<->get move                   \/\/      */
/* Release note 2.0.1: Updating to 64bits (Feb 2022)                      */
/*                                                                        */
/* This file is the interface to non-graphical version of Gnu Chess       */
/*                                                                        */
/*   Revision: 1991-04-15                                                 */
/*                                                                        */
/* Copyright (C) 1986, 1987, 1988, 1989, 1990 Free Software Foundation    */
/* Copyright (c) 1988, 1989, 1990  John Stanback                          */
/*                                                                        */
/* This file is not a part of CHESS.                                      */
/*                                                                        */
/* CHESS is distributed in the hope that it will be useful, but WITHOUT   */
/* ANY WARRANTY.  No author or distributor accepts responsibility to      */
/* anyone for the consequences of using it or for whether it serves any   */
/* particular purpose or works at all, unless he says so in writing.      */
/* Refer to the CHESS General Public License for full details. It should  */
/* be in a file named COPYING.                                            */
/*                                                                        */
/**************************************************************************/
#include <stdio.h>
#include <malloc.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <ctype.h>
#include <fcntl.h>
#include <stdlib.h>
#include "gnuchess.h"

/* #include <sys/dir.h> at SunOs 4 and #include <dirent.h> at SunOS 5 */
#ifndef MAXNAMELEN
#define MAXNAMLEN       512	/* maximum fileame length */
#endif

static  int	pid;
static  FILE	*from;
static  FILE	*to;
static  int	easy = 1;
static  int	force = 0;
static  int	timeunit = 60, movesperunit = 4;
      	int	debug = 1;
static	char	path[] = DEF_PROGRAM;
static	char	name[] = "gnuchessr";
static	char	temp_file[MAXNAMLEN];
static	int	is_inited = 0;
static  int     gnuchess_major = 3;

#if 1
int
chess_init()
{
        int toprog[2], fromprog[2], x;
        /* int  errprog[2];  For redirection of stderr */
        char buf[BSIZE], buf2[BSIZE];
        char time[10];
        char moves[10];
	int blackflag = 0;

        pipe(toprog);
        pipe(fromprog);
        /*pipe(errprog);        For redirection of stderr */

        strcpy(temp_file,TMP_FILE);
#if 0
        strcpy(temp_file,mktemp(temp_file));
#else
        fopen(temp_file,"w");
#endif

        if(debug)
		printf("Created file: %s\n",temp_file);

        if (!(pid = fork())) {
                /* Start up the program. */
                dup2(toprog[0], 0);
                dup2(fromprog[1], 1);
                /*dup2(errprog[1], 2); For redirection of stderr */

                x = open(temp_file,  O_WRONLY | O_CREAT | O_TRUNC, 0755);
                dup2(x, 2);
                close(x);

                close(toprog[0]);
                close(toprog[1]);
                close(fromprog[0]);
                close(fromprog[1]);
                /*close(errprog[0]); For redirection of stderr */
                /*close(errprog[1]); For redirection of stderr */
                sprintf (time, "%d", timeunit/60);
                sprintf (moves, "%d", movesperunit);
                execl(path, name, moves, time,  (char *) NULL);
                perror(name);
                exit(1);
        }

        close(toprog[0]);
        close(fromprog[1]);
        /*close(errprog[1]); For redirection of stderr */

        from = fdopen(fromprog[0], "r");
        setbuf(from, NULL);
        to = fdopen(toprog[1], "w");
        setbuf(to, NULL);

        /* Get the first line... */
        fgets(buf, BSIZE, from);
        if (debug) {
                fprintf(stderr, "%s says %s\n", name, buf);
        }
        if (strncmp(buf, "GNU Chess", 9) == 0) { 
            fgets(buf, BSIZE, from); // GNU Chess 6.2.9 has 4 extra lines
            fgets(buf, BSIZE, from); // GNU Chess 6.2.9 has 4 extra lines
            fgets(buf, BSIZE, from); // GNU Chess 6.2.9 has 4 extra lines
            fgets(buf, BSIZE, from); // GNU Chess 6.2.9 has 4 extra lines
            gnuchess_major = 6;
        } else {
            fprintf(stderr, "Communicating with background programs %s\n", buf);
            gnuchess_major = 6;
        }

        if (blackflag) {
                fputs("switch\n", to);
                fflush(to);
                fgets(buf, BSIZE, from);
                if (0 && debug) {
                        fprintf(stderr, "%s says %s\n", name, buf);
                        fprintf(stderr, "switch\n");
                }
        }

        /*
        chess_beep();
        */


        return (TRUE);
}
#else
int
chess_init()
{
	int toprog[2], fromprog[2], errorprog;
	char buf[BSIZE];
	char time[10];
	char moves[10];

	pipe(toprog);
	pipe(fromprog);

        strcpy(temp_file,TMP_FILE);
#if 0
        strcpy(temp_file,mktemp(temp_file));
#else
        fopen(temp_file,"w");
#endif

	printf("Created file: %s\n",temp_file);

	if (!(pid = fork())) {
		/* Start up the program. */

		dup2(toprog[0], 0);
		dup2(fromprog[1], 1);

		dup2(errorprog, 2);
		errorprog = open(temp_file,  O_WRONLY | O_CREAT | O_TRUNC, 0755);
		close(errorprog);

		close(toprog[0]);
		close(toprog[1]);
		close(fromprog[0]);
		close(fromprog[1]);

		sprintf (time, "%d", timeunit/60);
		sprintf (moves, "%d", movesperunit);
		execl(path, name, moves, time,  (char *) NULL);

		perror(name);
		return(FALSE);
	}

	close(toprog[0]);
	close(fromprog[1]);

	from = fdopen(fromprog[0], "r");
	setbuf(from, NULL);
	to = fdopen(toprog[1], "w");
	setbuf(to, NULL);

	/* Get the first line... */
	fgets(buf, BSIZE, from);
	if (0 && debug)  
		fprintf(stderr, "answer    : %s\n", buf);

	return (TRUE);
}
#endif
void
chess_quit()
{
        is_inited = 0;
        fclose(from);
        fclose(to);
	kill(pid, SIGTERM);
	unlink(temp_file);
        return;
}

int chess_verify(human_move)
char *human_move;
{
	char move[CSIZE];
	int  l;

	l = sscanf(human_move,"%s",move); 
	if(l == EOF || move == NULL)
		return(ILLEGAL);

	/* only check's the format, not if it is a legal or illegal move */
	if(
	       /* format "e2e4" */
	       (                      move[0] >= 'a' && move[0] <= 'h' &&
                                      move[1] >= '1' && move[1] <= '8' &&
                                      move[2] >= 'a' && move[2] <= 'h' &&
                                      move[3] >= '1' && move[3] <= '8' )
	       /* format "Pf3" */
            || (                     (move[0] == 'p' || move[0] == 'r' ||
                                      move[0] == 'n' || move[0] == 'b' ||
                                      move[0] == 'q' || move[0] == 'k' ||
                                      move[0] == 'P' || move[0] == 'R' ||
                                      move[0] == 'N' || move[0] == 'B' ||
                                      move[0] == 'Q' || move[0] == 'K' ) &&
                                      move[1] >= 'a' && move[1] <= 'h' &&
                                      move[2] >= '1' && move[2] <= '8' )
          )
	{
		if (0 && debug)
                	fprintf(stderr, "verify    : true\n");
		return(LEGAL);
        } else {
		if (0 && debug)
                	fprintf(stderr, "verify    : false\n");
		return(ILLEGAL);
	}
}

char *
chess_command(command)
char *command;
{
	int i, search_size, no_answers = 2, no_arguments = 0;
        char answer[BSIZE], s[CSIZE], a[CSIZE], *search, *start ;

        if (!is_inited){
                if(chess_init()==FALSE)
                        return(NULL);
                is_inited++;
        }

        printf("inside chess_command(%s)\n", command);

	i = sscanf(command,"%s",s);
	if (i == EOF || s[0] == 0) {
		strcpy(s, "quit");         /* Let gnuchessr exit, not our program */
	}

	if (chess_verify(s) == LEGAL)      /* format "e2e4" or "Qf3" */
	        no_answers = 2;
        else if (strcmp (s, "bd") == 0)
		no_answers = 10;
	else if (strcmp (s, "alg") == 0)
		no_answers = 0;
	else if (strcmp (s, "quit") == 0)
		no_answers = 0;
	else if (strcmp (s, "exit") == 0)
		no_answers = 0;
	else if (strcmp (s, "post") == 0)
		no_answers = 0;
	else if (strcmp (s, "set") == 0)
	{
		no_answers = 4;
		no_arguments = -99;
	}
	else if (strcmp (s, "edit") == 0)
	{
		no_answers = 4;
		no_arguments = -99;
	}
	else if (strcmp (s, "setup") == 0)
	{
		no_answers = 1;
		no_arguments = 8;
	}
	else if (strcmp (s, "first") == 0)
		no_answers = 2;
	else if (strcmp (s, "go") == 0)
		no_answers = 2;
	else if (strcmp (s, "help") == 0)
		no_answers = 21;
	else if (strcmp (s, "force") == 0)
		no_answers = 0;
	else if (strcmp (s, "book") == 0)
		no_answers = 0;
	else if (strcmp (s, "new") == 0)
		no_answers = 0;
	else if (strcmp (s, "list") == 0)
		no_answers = 0;
	else if (strcmp (s, "hash") == 0)
		no_answers = 0;
	else if (strcmp (s, "level") == 0)
	{
                no_answers   = 0;
                no_arguments = -3;
        }
	else if (strcmp (s, "clock") == 0)
	{
                no_answers   = 0;
                no_arguments = -3;
        }
	else if (strcmp (s, "beep") == 0)
		no_answers = 0;
	else if (strcmp (s, "Awindow") == 0)
	{
                no_answers   = 0;
                no_arguments = 1;
        }
	else if (strcmp (s, "Bwindow") == 0)
	{
                no_answers   = 0;
                no_arguments = 1;
        }
	else if (strcmp (s, "rcptr") == 0)
		no_answers = 0;
	else if (strcmp (s, "hint") == 0)
		no_answers = 0;
	else if (strcmp (s, "both") == 0)
		no_answers = 1000;
	else if (strcmp (s, "reverse") == 0)
		no_answers = 0;
	else if (strcmp (s, "switch") == 0)
		no_answers = 2;
	else if (strcmp (s, "white") == 0) 
        {
                if (gnuchess_major == 3)
		    no_answers = 0;
                else
		    no_answers = 1;
        }
	else if (strcmp (s, "black") == 0)
        {
                if (gnuchess_major == 3)
		    no_answers = 0;
                else
		    no_answers = 1;
        }
	else if (strcmp (s, "undo") == 0)
		no_answers = 0;
	else if (strcmp (s, "remove") == 0)
		no_answers = 0;
	else if (strcmp (s, "get") == 0)
	{
		no_answers   = 0;
		no_arguments = 1;
	}
	else if (strcmp (s, "xget") == 0)
		no_answers = 0;
	else if (strcmp (s, "save") == 0)
	{
		no_answers   = 1;
		no_arguments = 1;
	}
	else if (strcmp (s, "depth") == 0)
	{
		no_answers   = 0;
		no_arguments = 1;
	}
	else if (strcmp (s, "hashdepth") == 0)
	{
		no_answers   = 0;
		no_arguments = 2;
	}
	else if (strcmp (s, "random") == 0)
		no_answers = 0;
	else if (strcmp (s, "easy") == 0)
		no_answers = 0;
	else if (strcmp (s, "contempt") == 0)
	{
		no_answers   = 0;
		no_arguments = 1;
	}
	else if (strcmp (s, "xwndw") == 0)
	{
		no_answers   = 0;
		no_arguments = 1;
	}
	else if (strcmp (s, "test") == 0)
		no_answers   = 2;
	else if (strcmp (s, "o-o") == 0)
		no_answers = 2;
	else if (strcmp (s, "o-o-o") == 0)
                no_answers = 2;
	else if (strcmp (s, "O-O") == 0)
		no_answers = 2;
	else if (strcmp (s, "O-O-O") == 0)
                no_answers = 2;
	else 
		no_answers = 1;

        if( strcmp (s, "switch") == 0 || strcmp (s, "go")  == 0 ||
            strcmp (s, "o-o-o")  == 0 || strcmp (s, "o-o") == 0 ||
            strcmp (s, "O-O-O")  == 0 || strcmp (s, "O-O") == 0 ||
            strcmp (s, "first")  == 0 || chess_verify(s) == LEGAL )
	{
		/* the computer is thinking in the opponents time, */
                /* so he has to be stopped first.                  */ 
		if (!easy)
         	       kill (pid, SIGINT);
		if (force && strcmp (s, "switch") != 0)
			no_answers = 1;
	}

	fputs(s, to);
	fputs("\n", to);
        if (debug)  
		fprintf(stderr, "command   : %s\n", s);

	search_size = 6; // +1 to accomodate for the null terminator
	search = malloc(search_size);
	start  = malloc(search_size);
	strcpy(search,"%s\n");
	strcpy(start ,"%*s ");

	while(no_arguments--)
        {
		int l,c;

		if(no_arguments == -4) {
		/* double argument in clock & level */
			start  = realloc(start ,CSIZE);
			search = realloc(search,CSIZE);

			l = sscanf(command,"%*s %s %s\n", search, start );
			sprintf(a,"%s %s",search, start );

			no_arguments = 1;
			start  = realloc(start ,12);
			search = realloc(search,12);
			search_size = 12;
			strcpy(search, "%*s %*s %s\n");
                	strcpy(start ,"%*s ");

		} else {

			search_size += 4;
			start  = realloc(start ,search_size);
			search = realloc(search,search_size);

			strcat(start , search);
			strcpy(search, start );
			strcpy(start ,"%*s ");

			l = sscanf(command,search,a);

		} /* if(no_arguments = -4)  */

		if( l == EOF || a == NULL) {
			if(strcmp (s, "save") == 0   || strcmp (s, "get") == 0 ) {
				strcpy(a,"chess.000");
			} else if(strcmp (s, "setup") == 0 ) {
				strcpy(a,"--------");
			} else if( no_arguments < 0 ) {
				strcpy(a,".");
			} else {
				no_answers = 1;
				strcpy(a,"?");
			}
		}

		fputs(a, to);
        	fputs("\n\0", to);
		fflush(to);
        	if (debug)  
                	fprintf(stderr, "argument  : %s\n", a);

		if(no_arguments < 0) {		/* editing */
			if(strcmp(a,".") == 0)
				no_arguments = 0;
		}
                if(strcmp (s, "setup") == 0 || strcmp (s, "get") == 0 ||
                   strcmp (s, "set") == 0 ) {
			/* nothing */
		} else {
                        /* arnoud: for which commands this block is needed? */
			do {
				c = fgetc(from);
	
			} while( c != '=' && c != ':');

			c = fgetc(from);
		}			
        };
	free(search);
        free(start );

	while(no_answers--) {

                alarm(60); // enough time to remove a piece from the board.
		fgets(answer, BSIZE, from);

        	if (debug)  
                	fprintf(stderr, "answer    : %s", answer);
        	 
		for (search = answer; !isalpha(*search); search++)
                	;		

		if(strncmp (search, "Illegal", 7) == 0) {
			return(NULL); 
		}
		if(strncmp (search, "Black", 5) == 0) {
                        // TBC
                        if( strcmp (s, "go")  == 0) {
                           printf("%s has won!\n", search);
                           return("Quit");
                        }
			is_inited = 0;
                        return(NULL);
                }
                if(strncmp (search, "White", 5) == 0) {
                        if( strcmp (s, "go")  == 0) {
                           printf("%s has won!\n", search);
                           return("Quit");
                        }
			is_inited = 0;
                        return(NULL);
                }
                if(strncmp (search, "Draw", 4) == 0) {
                        if( strcmp (s, "go")  == 0) {
                           printf("It is a draw!\n");
                           return("Quit");
                        }
                }

	};

	if( strcmp (s, "quit") == 0   || strcmp (s, "exit") == 0 )
	{
		chess_quit();
	}

	if( strcmp (s, "easy") == 0   )
	{
		easy = easy ? 0 : 1;
	}

	if( strcmp (s, "force") == 0   )
	{
		force = force ? 0 : 1;
	}

	if( strcmp (s, "switch") == 0 || strcmp (s, "black")  == 0 ||
            strcmp (s, "white")  == 0 )
	{
		force = 0;
	}
	if( strcmp (s, "switch") == 0 || strcmp (s, "go")  == 0 ||
	    strcmp (s, "o-o-o")  == 0 || strcmp (s, "o-o") == 0 ||
	    strcmp (s, "O-O-O")  == 0 || strcmp (s, "O-O") == 0 ||
	    strcmp (s, "first")  == 0 || chess_verify(s) == LEGAL )
	{
		return(search);
	} else {
		return(NULL);
	}

}
