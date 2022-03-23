/******************************************************************************/
/***                                                                        ***/
/*** Interface to UMI-RTX robot. Software written by                        ***/
/***                                                                        ***/
/***    Arnoud Visser, a.visser@uva.nl                                      ***/
/***                                                                        ***/
/***    Software thorougly messed up by Matthijs Spaan                      ***/
/***    Reused by Arnoud after 10-20 years                                  ***/
/***                                                                        ***/
/******************************************************************************/

#include <stdio.h>
#include "path.h"
#include "umi.h"

#define COUPLED 0
#define CHECK_IK 0

void
move_list(l)
joint_list_t l;
{
    joint_list_t c;
    if (l == NULL)
    {
        printf("Can't execute an empty list.\n");
        return;
    }
	if(0) printf("  zed\tshoul\telbow\t  yaw\tpitch\t roll\t grip\n");

    c = l;
    do
    {
  		if(0) printf("%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\n",
            c->zed, c->shoulder, c->elbow, c->yaw, c->pitch, c->roll, c->grip);
#if 1
  		if(COUPLED)
  			umi_move(c->zed,
					  c->shoulder,
					  c->elbow,
					  c->yaw,
					  c->pitch,
					  c->roll,
					  c->grip);
  		else
  			umi_move(c->zed,
					  c->shoulder,
					  c->elbow,
					  c->yaw - (c->elbow / 2.0),
					  c->pitch,
					  c->roll,
					  c->grip);
#else
                if(COUPLED)
                        move_real(c->zed,
                                          c->shoulder,
                                          c->elbow,
                                          c->yaw,
                                          c->pitch,
                                          c->roll,
                                          c->grip);
                else
                        move_real(c->zed,
                                          c->shoulder,
                                          c->elbow,
                                          c->yaw - (c->elbow / 2.0),
                                          c->pitch,
                                          c->roll,
                                          c->grip);
#endif

	/* increase the speed with 10% */

	// umi_speed (1.1,1.1,1.1,1.1,1.1,1.1,1.1);	

		
    } while ( (c = c->next_data) != NULL );

if(0) printf("Done.\n");
}

#if CHECK_IK

static void
print_xyz_list(l0)
xyz_list_t l0;
{
  xyz_list_t next;

  printf("xyz_list:\n");
  printf("    x\t    y\t    z\t   n1\t   n2\t   n3\ttheta\t grip\n");
  next = l0;
  while(next != NULL) {
	    printf("%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\t%5.1f\n",
                next->x,
                next->y,
                next->z,
                next->n1,
                next->n2,
                next->n3,
                next->theta,
                next->grip);
        next = next->next_data;
  }
}
#endif
