/**************************************************************************/
/* config.h                                                     /\/\      */
/* Version 2.0.3 -- March 2022                                  \  /      */
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
/* Release note 2.0: changed names!                             \/\/      */
/* Release note 2.0.1: Moved gnuchessr (based on gnuchess 3.1) to rtxipc  */
/* Release note 2.0.2: Interface to gnuchess 6.2.9 (Feb 2022)             */
/* Release note 2.0.3: data files on relative path (March 2022)           */
/**************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#define	UMI_RTX   "../data/umi.rtx"
#define	PIECES    "../data/pieces.rtx"
#define	BOARD     "../data/board.rtx"
#define GNUCHESS  "./bin/gnuchessr"

#define	MYBOARD   "../tmp/board.rtx"
#define MYPIECES  "../tmp/pieces.rtx"

#endif /* CONFIG_H */
