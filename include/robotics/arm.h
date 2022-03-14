/**************************************************************************/
/* arm.h                                                        /\/\      */
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
/* Release note 2.0: structure instead of array                 \/\/      */
/**************************************************************************/
#ifndef ARM_H
#define ARM_H

/***************************************************************************
 Kinematic description of the UMI-RTX robot in the Denavit-Hartenberg
 Representation.
  
 theta[i]      is the joint angle from the X[i-1] axis to the X[i] axis
               about the Z[i] axis (using the right-handed rule).
 d[i]          is the distance form the origin of the (i-1)th coordinate
               frame to the intersection of the Z[i-1] axis with the X[i]
               axis along the Z[i-1] axis.
 a[i]          is the offset distance from the intersection of the Z[i-1]
               axis with the X[i] axis to the origin of the (i)th frame
               along the X[i] axis (or shortest distance between the Z[i-1]
               and Z[i] axes).
 alfa[i]       is the offset angle form the Z[i-1 axis to the Z[i] axis
               about the X[i] axis (using the right-handed rule).
***************************************************************************/
#ifdef __STDC__

typedef enum joint_type_e {END_EFFECTOR, ROTARY, PRISMATIC} joint_type_t;
typedef enum joint_e {ZED, SHOULDER, ELBOW, YAW, PITCH, ROLL, GRIPPER} joint_t;

#else /* __STDC__ */

#define END_EFFECTOR 0
#define ROTARY    1
#define PRISMATIC 2

#define ZED 0
#define SHOULDER 1
#define ELBOW 2
#define YAW 3
#define PITCH 4
#define ROLL 5
#define GRIPPER 6

#endif /* __STDC__ */

#define NOJOINTS  7

typedef struct link_s {
  double theta;		/* degree */
  double alpha;		/* degree */
  double a;		/* mm */
  double d;		/* mm */
  double ECmin;		/* minimum encoder counts */
  double ECmax;		/* maximum encoder counts */
  double Rmin;		/* minimum reach in mm or degrees */
  double Rmax;		/* maximum reach in mm or degrees */
  double RT;		/* flag to indicate which parameter is variable */
} link_t;

typedef struct robot_s {
  char name[81];
  link_t link[NOJOINTS];
} robot_t;

#endif /*ARM_H*/
