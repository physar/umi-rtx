/***************************************************************************/
/* matrix.h                                                     /\/\      */
/* Version 2.0   --  Januari  1995                              \  /      */
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
/* Release note 2.0:                                            \/\/      */
/**************************************************************************/
#ifndef MATRIX_H
#define MATRIX_H
/**************************************************************************/
/***									 ***/
/***  Software written by Edo H. Dooijes, 				 ***/
/*** for Computer Graphics course					 ***/
/***  No guarantee is given that procedurers are correct, since this is  ***/
/***  an early copy made by Joris van Dam				 ***/
/***									 ***/
/*** BEWARE OF THE DIFFERENCE BETWEEN degrees AND rad.                   ***/
/*** USE PRINT TO MAKE SURE THE RIGHT VALUES ARE CALCULATED              ***/
/***************************************************************************/

#ifndef TRUE
#define TRUE 1
#endif
#ifndef FALSE
#define FALSE 0
#endif


typedef double 		VEC3[3];
typedef double 		VEC4[4];
typedef double          MAT33[3][3];
typedef double          MAT44[4][4];


#ifdef __STDC__

double det22(double a11, double a12, double a21, double a22);
double det33(double a11, double a12, double a13,
             double a21, double a22, double a23,
             double a31, double a32, double a33);
double Det3(VEC3 a, VEC3 b, VEC3 c);
double Det4(VEC4 a, VEC4 b, VEC4 c, VEC4 d);
void   imat3(MAT33 m);
void   imat4(MAT44 m);
void   VSmul3(VEC3 v, double s);
void   VSmul4(VEC4 v, double s);
void   VVcrossprod3(VEC3 v1, VEC3 v2, VEC3 v3);
double VVprod3(VEC3 v1, VEC3 v2);
void   MMprod3(MAT33 m1, MAT33 m2, MAT33 m3);
void   Rot2D(double theta, MAT33 m);
void   Transl2D(double tx, double ty, MAT33 m);
void   Scale2D(double sx, double sy, MAT33 m);
void   MVprod3(MAT33 m, VEC3 u, VEC3 v);
void   Mprint(MAT44 m);
double VVprod4(VEC4 a, VEC4 b);
void   MMprod4(MAT44 m1, MAT44 m2, MAT44 m3);
void   Rotx3D(double theta, MAT44 m);
void   Roty3D(double theta, MAT44 m);
void   Rotz3D(double theta, MAT44 m);
void   Transl3D(double tx, double ty, double tz, MAT44 m);
void   Scale3D(double sx, double sy, double sz, MAT44 m);
void   MVprod4(MAT44 m, VEC4 u, VEC4 v);
void   RtoL(MAT44 m);
int    Plane(VEC4 p, VEC4 q, VEC4 r, VEC4 a);

#else /* __STDC__ */

double det22();
double det33();
double Det3();
void   imat3();
void   imat4();
void   VSmul3();
void   VSmul4();
void   VVcrossprod3();
double VVprod3();
void   MMprod3();
void   Rot2D();
void   Transl2D();
void   Scale2D();
void   MVprod3();
void   Mprint();
double VVprod4();
void   MMprod4();
void   Rotx3D();
void   Roty3D();
void   Rotz3D();
void   Transl3D();
void   Scale2D();
void   MVprod4();
void   RtoL();
int    Plane();

#endif

#endif /*MATRIX_H*/
