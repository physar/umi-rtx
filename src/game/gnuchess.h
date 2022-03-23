/**************************************************************************/
/* gnuchess.h                                                   /\/\      */
/* Version 2.0   --  November 1994                              \  /      */
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
/* Release note 2.0: the part needed for the connection to gnu  \/\/      */
/**************************************************************************/
#ifndef GNUCHESS_H
#define GNUCHESS_H

#include "config.h" /* for gnuchess location */

#define BSIZE      512
#define CSIZE	   80        /* number from InputCommand() of gnuchessr */

#ifndef FALSE
#define FALSE   (0)
#endif

#ifndef TRUE
#define TRUE    (1)
#endif

#if 1
#define DEF_PROGRAM	GNUCHESS
#endif
#define TMP_FILE        "chess.error_report"

#define LEGAL   1

#define WHITE   -96
#define BLACK   -97
#define ILLEGAL -98
#define UNKNOWN -99

#endif /*GNUCHESS_H*/
