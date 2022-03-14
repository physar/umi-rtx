/**************************************************************************/
/* path.h                                                       /\/\      */
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
/* Release note 2.0: other naming of structures                 \/\/      */
/**************************************************************************/
#ifndef PATH_H
#define PATH_H

 /* Description of Path in Cartesian coordinates */

typedef struct xyz_list_item_s {
  double x, y, z, n1, n2, n3, theta, grip;
  struct xyz_list_item_s *next_data;
} xyz_list_item_t, *xyz_list_t;

 /* Description of Path in Joint coordinates */

typedef struct joint_list_item_s {
  double zed, shoulder, elbow, yaw, pitch, roll, grip;
  struct joint_list_item_s *next_data;
} joint_list_item_t, *joint_list_t;

#endif /*PATH_H*/
