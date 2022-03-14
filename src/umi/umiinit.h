/**************************************************************************/
/* umiinit.h                                                        /\/\      */
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
/* Release note 2.0: New                                        \/\/      */
/**************************************************************************/
#ifndef UMIINIT_H
#define UMIINIT_H

#ifdef __STDC__

void umi_initialise_zed();
void umi_initialise_gripper();
void umi_initialise_wrist();
void umi_initialise_shoulder();
void umi_initialise_elbow_and_yaw();

void umi_read_init_data();

void umi_set_limit_position();
void umi_goto_init_position();

#else /* __STDC__ */

void umi_initialise_zed();
void umi_initialise_gripper();
void umi_initialise_wrist();
void umi_initialise_shoulder();
void umi_initialise_elbow_and_yaw();

void umi_read_init_data();

void umi_set_limit_position();
void umi_goto_init_position();

#endif /* __STDC__ */

#endif /*UMIINIT_H*/
