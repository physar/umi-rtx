/**************************************************************************/
/* simulator.h                                                  /\/\      */
/* Version 2.1   --  March 2022                                 \  /      */
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
/* Release note 2.1: functions used outside simulator.c         \/\/      */
/**************************************************************************/
#ifndef SIMULATOR_H
#define SIMULATOR_H

#include "path.h"

void rtx_simulator_list(joint_list_t joints); /* ask for user conformation every step */ 
void rtx_move_list(joint_list_t joints);

#endif /*SIMULATOR_H*/
