/***************************************************************************/
/***									 ***/
/***  Software written by Edo H. Dooijes, 				 ***/
/*** for Computer Graphics course					 ***/
/***  No guarantee is given that procedurers are correct, since this is  ***/
/***  an early copy made by Joris van Dam				 ***/
/***									 ***/
/*** BEWRAE OF THE SINE AND COSINE. USE PRINT TO MAKE SURE THE RIGHT     ***/
/*** VALUES ARE CALCULATED 						 ***/
/***************************************************************************/

#define DEBUG 0

#include <stdio.h>
#include <math.h>
#include "matrix.h"



double 
det22(a11, a12, a21, a22) 
     double       a11, a12, a21, a22; 
{ 
     /* determinant of 2 * 2 Matrix */

        return(a11 * a22 - a21 * a12);
} 


double 
det33(a11, a12, a13, a21, a22, a23, a31, a32, a33) 
     double       a11, a12, a13, a21, a22, a23, a31, a32, a33; 
{   
     /* det33 */ 
      	return(a11 * det22(a22, a23, a32, a33) - a12 * det22(a21, a23, a31, a33) +
	a13 * det22(a21, a22, a31, a32));
} 


double 
Det3(a, b, c)
     VEC3         a, b, c;
{
        return(det33(a[0], b[0], c[0], a[1], b[1], c[1], a[2], b[2], c[2]));
}



double 
Det4(a, b, c, d) 
     VEC4         a, b, c, d; 
{
	VEC4         dum;

     dum[0] = det33(b[1], c[1], d[1], b[2], c[2], d[2], b[3], c[3], d[3]); 
     dum[1] = det33(b[0], c[0], d[0], b[2], c[2], d[2], b[3], c[3], d[3]); 
     dum[2] = det33(b[0], c[0], d[0], b[1], c[1], d[1], b[3], c[3], d[3]); 
     dum[3] = det33(b[0], c[0], d[0], b[1], c[1], d[1], b[2], c[2], d[2]); 
     return(a[0] * dum[0] - a[1] * dum[1] + a[2] * dum[2] - a[3] * dum[3]);
} 


void 
imat3(m) 
     MAT33        m;
{
     /* 3 x 3 Identity matrix */

	int          r, k;

     	for (r = 0; r < 3; r++) 
       		for (k = 0; k < 3; k++) 
      		if (r == k)
			m[r][k] = 1.0; 
       		else
			m[r][k] = 0.0; 
} 


void 
imat4(m) 
     MAT44        m;
{
	int          r, k;

	for (r = 0; r < 4; r++) 
    		for (k = 0; k < 4; k++) 
		{
        		if (r == k)
				m[r][k] = 1.0; 
               		else
				m[r][k] = 0.0; 
		}
} 



void 
VSmul3(v,s)
  VEC3 v;
  double s;
{
     /* written by ANUJ DEV  (thank god) */
	int	i;
	
	for(i=0; i<3; i++)
		v[i] *= s;
}

void 
VSmul4(v,s)
  VEC4 v;
  double s;
{
    /* you guessed right, the man is a genius */
	int	i;

	for(i=0; i<4; i++)
		v[i] *= s;
}

void 
VVcrossprod3(v1,v2,v3)
  VEC3 v1,v2,v3;
{
     /* uitprodukt */

	v3[0] = v1[1]*v2[2] - v1[2]*v2[1];
	v3[1] = v1[2]*v2[0] - v1[0]*v2[2];
	v3[2] = v1[0]*v2[1] - v1[1]*v2[0];
}

double 
VVprod3(v1,v2)
  VEC3 v1,v2;
{
     /* inprodukt */

  double p;
  int    i;

  for (i=0,p=0.0;i<3;i++)
    p+=v1[i]*v2[i];
  return (p);
}


void 
MMprod3(m1, m2, m) 
     MAT33        m1, m2, m;
{
     /* m3 = m1 * m2 */
	int          i, r, k;

     for (r = 0; r < 3; r++) 
         for (k = 0; k < 3; k++) 
         {
		m[r][k] = 0.0; 
                for (i = 0; i < 3; i++) 
                	m[r][k] = m[r][k] + m1[r][i] * m2[i][k]; 
         } 
} 


void 
Rot2D(theta, m) 
     double       theta; 
     MAT33        m;
{
	double       c, s;

     s = sin(theta);
     c = cos(theta); 
     imat3(m); 
     m[0][0] = c;
     m[1][1] = c; 
     m[0][1] = -s;
     m[1][0] = s; 
} 


void 
Transl2D(tx, ty, m) 
     double       tx, ty; 
     MAT33        m; 
{ 
     imat3(m); 
     m[0][2] = tx; 
     m[1][2] = ty; 
} 


void 
Scale2D(sx, sy, m) 
     double       sx, sy; 
     MAT33        m; 
{ 
     imat3(m); 
     m[0][0] = sx;
     m[1][1] = sy; 
} 


void 
MVprod3(m, u, v) 
     MAT33        m; 
     VEC3         u, v;
{
	int          i, j;

     for (i = 0; i < 3; i++) 
     {
	v[i] = 0.0; 
        for (j = 0; j < 3; j++) 
        	v[i] = v[i] + m[i][j] * u[j]; 
     } 
} 


double 
VVprod4(a, b) 
     VEC4         a, b;
{
	int          i;
    	double       prod;

     prod = 0.0; 
     for (i = 0; i < 4; i++)
	prod = prod + a[i] * b[i]; 
     return(prod); 
} 


void Mprint(m)
	MAT44 m;
{
	int i,k;

	for (k = 0; k < 4; k++)
         {
		for (i = 0; i < 4; i++)
                {
			printf("m[%d%d] is %g\t",k,i,m[k][i]);
		}
		printf("\n");
	}
}


void 
MMprod4(m1, m2, m3) 
     MAT44        m1, m2, m3;
{
     /* m3 = m1 * m2 */
	int          i, r, k;
	MAT44	     m;

     for (r = 0; r < 4; r++) 
         for (k = 0; k < 4; k++) 
         {
		m[r][k] = 0.0; 
                for (i = 0; i < 4; i++) 
		{
                       m[r][k] = m[r][k] + m1[r][i] * m2[i][k]; 
		       /*
		       printf("	m1[%d,%d] = %f, m2[%d,%d] = %f\n",r,i,m1[r][i],
			i,k,m2[i][k]);
			*/
		}
		/*
		printf("\n");
		*/
         } 
      for (r=0;r<4;r++)
          for (k=0;k<4;k++)
                m3[r][k]=m[r][k];
} 


void Roty3D(theta, m)

double       theta; 
MAT44        m;
{
	double       c, s;

     s = sin(theta);
     c = cos(theta); 
     if (DEBUG) printf("theta %lf, cos %g, sin %lf\n",theta,c,s);
     imat4(m); 
     m[0][0] = c;
     m[0][2] = s; 
     m[2][0] = -s;
     m[2][2] = c; 
} 


void Rotx3D(theta, m) 

double       theta; 
MAT44        m; 
{
	double       c, s;

     s = sin(theta);
     c = cos(theta); 
     if (DEBUG) printf("theta %lf, cos %lf, sin %lf\n",theta,c,s);
     imat4(m); 
     m[1][1] = c;
     m[1][2] = -s; 
     m[2][1] = s;
     m[2][2] = c; 
} 


void Rotz3D(theta, m) 

double       theta; 
MAT44        m;
{
	double       c, s;

     s = sin(theta);
     c = cos(theta); 
     if (DEBUG) printf("theta %lf, cos %lf, sin %lf\n",theta,c,s);
     imat4(m); 
     m[0][0] = c;
     m[0][1] = -s; 
     m[1][0] = s;
     m[1][1] = c; 
} 


void Transl3D(tx, ty, tz, m) 

double       tx, ty, tz; 
MAT44        m;
{ 
     imat4(m); 
     m[0][3] = tx; 
     m[1][3] = ty; 
     m[2][3] = tz; 
} 


void Scale3D(sx, sy, sz, m) 

double       sx, sy, sz; 
MAT44        m;
{ 
     imat4(m); 
     m[0][0] = sx;
     m[1][1] = sy; 
     m[2][2] = sz; 
} 


void MVprod4(m, u, w) 

MAT44        m; 
VEC4         u, w;
     /* w = m * u */
{
	int          i, j;
	VEC4         v;

     for (i = 0; i < 4; i++) 
     {
	v[i] = 0.0; 
        for (j = 0; j < 4; j++) 
        	v[i] = v[i] + m[i][j] * u[j]; 
     } 
     for (i=0;i<4;i++)
        w[i]=v[i];
} 


void RtoL(m) 

MAT44        m;
{ 
    /* van linkshandig naar rechtshandig coordinaten stelsel */
     imat4(m); 
     m[2][2] = -1.0; 
} 




int Plane(p, q, r, a) 

VEC4         p, q, r, a;
  /* computes 4-vector describing plane defined by p,q,r.
 no check on collinearity of p, q, r !  */ 
{ 
     double eps = 1E-15;
     VEC3   u,v;
     a[0] = det33(p[1], q[1], r[1], p[2], q[2], r[2], p[3], q[3], r[3]); 
     a[1] = -det33(p[0], q[0], r[0], p[2], q[2], r[2], p[3], q[3], r[3]); 
     a[2] = det33(p[0], q[0], r[0], p[1], q[1], r[1], p[3], q[3], r[3]); 
     a[3] = -det33(p[0], q[0], r[0], p[1], q[1], r[1], p[2], q[2], r[2]); 

	if (a[3] < eps)
	{
		u[0]  = q[0] - p[0];
		u[1]  = q[1] - p[1];
		u[2]  = q[2] - p[2];
		v[0] = r[0] - p[0];
		v[1] = r[1] - p[1];
		v[2] = r[2] - p[2];
		return((!VVprod3(u,v)) < eps);
	}
	else
		return(TRUE);
} 

