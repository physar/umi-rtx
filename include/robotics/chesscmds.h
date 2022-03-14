/**************************************************************************/
/* chesscmds.h                                                  /\/\      */
/* Version 1.0   --  May 2006                                   \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser                                     _  \/\/  _   */
/*         Universiteit van Amsterdam                       | |      | |  */
/*         Informatica Instituut                            | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*         arnoud@science.uva.nl                            | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robotics course    \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/* Release note 1.0: definitions taken from chesscmds.c         \/\/      */
/**************************************************************************/
#ifndef CHESSCMDS_H
#define CHESSCMDS_H

typedef enum {USER,AUTOMATIC,FORCE} mode_t;

int get_command(char *command, mode_t mode);


#endif /* CHESSCMDS_H */
