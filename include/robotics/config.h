/**************************************************************************/
/* config.h                                                     /\/\      */
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
/* Release note 2.0: changed names!                             \/\/      */
/* Release note 2.0.1: Moved gnuchessr (based on gnuchess 3.1) to rtxipc  */
/* Release note 2.0.2: Interface to gnuchess 6.2.9 (Feb 2022)             */
/**************************************************************************/
#ifndef CONFIG_H
#define CONFIG_H

#ifdef SCIENCEPARK
#define	UMI_RTX   "/opt/prac/robotics/data/umi.rtx"
#define	PIECES    "/opt/prac/robotics/data/pieces.rtx"
#define	BOARD     "/opt/prac/robotics/data/board.rtx"
// #define GNUCHESS  "/home/arnoud/packages/gnuchess-6.2.9/src/gnuchess"
#define GNUCHESS  "/opt/prac/robotics/rtxipc/bin/gnuchessr"
#endif
#ifdef GENE
#define	UMI_RTX   "/home/stud/robotics/data/umi.rtx"
#define	PIECES    "/home/stud/robotics/data/pieces.rtx"
#define	BOARD     "/home/stud/robotics/data/board.rtx"
#define GNUCHESS  "/home/stud/robotics/bin/gnuchessr"
#endif
#ifdef OLD
#define	UMI_RTX   "/home/arnoud/src/robotica/data/umi.rtx"
#define	PIECES    "/home/arnoud/src/robotica/data/pieces.rtx"
#define	BOARD     "/home/arnoud/src/robotica/data/board.rtx"
#define GNUCHESS  "/home/mtjspaan/linux/bin/gnuchessr"
#endif

#define	MYBOARD   "board.rtx"
#define MYPIECES  "pieces.rtx"

#endif /* CONFIG_H */
