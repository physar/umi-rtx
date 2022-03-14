/**************************************************************************/
/* umi.h                                                        /\/\      */
/* Version 2.1   --  July 2011                                  \  /      */
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
/* Release note 2.0 (August 1994: New interface definition      \/\/      */
/* Release note 2.1: Added umi_stop                                       */
/**************************************************************************/
#ifndef UMI_H
#define UMI_H

#ifdef __STDC__

int umi_moving();
void umi_init();
void umi_move(double zed,
              double shoulder,
              double elbow,
              double yaw,
              double pitch,
              double roll,
              double grip);
void umi_stop();
int UserIsSpecial(char name[]);

#else /* __STDC__ */

int umi_moving();
void umi_init();
void umi_move();
void umi_stop();

#endif /* __STDC__ */

#endif /*UMI_H*/
