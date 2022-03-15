/* /* Code inspired by a solution from Jaap Kreykamp an Chris Niekel (1994) */

#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#ifdef __GNUC__
#include <math.h>
#endif
#ifdef ARNOUD
#include <math.h>
#else
#include "interface.h"
#endif
#include "chess.h"
#include "arm.h"
#include "path.h"
#include "config.h"
#include "xml_parser.h"

#ifndef PI
#define PI 3.1415927
#endif

#define ABS(x) ((x<0)?(-(x)):(x))
#define MIN(x,y) ( (x<y) ? x : y )
#define MAX(x,y) ( (x>y) ? x : y )
#define RAD2DEG(x) (x*180.0/PI)

#define Rzed(x) robot.link[0].x 
#define Rshoulder(x) robot.link[1].x 
#define Relbow(x) robot.link[2].x 
#define Ryaw(x) robot.link[3].x 
#define Rpitch(x) robot.link[4].x 
#define Rroll(x) robot.link[5].x 
#define Rgrip(x) robot.link[6].x 

#define Rtheta theta
#define Ralpha alpha
#define Ra a
#define Rd d
#define RECmax ECmax
#define RECmin ECmin
#define RRmin Rmin
#define RRmax Rmax
#define RRT RT

#define LEFT        1
#define RIGHT       2
#define MARGE       20

#define COUPLED 0
#define POS(x)      ((x==LEFT)? "left" :  "right")
/* zmargin - How high must the zed be before we can safely stretch the arm? */
#define zmargin 150.0

/* Global vars that indicate the current position of the robotarm */
double zed, shoulder, elbow, yaw, pitch, roll, grip;
double zsafe = 31.0; /* Increase the zed always be this to avoid ruining the
    table.. Looks like that with the simulator.. */
static double wrist_angle;
#if 0        
double el_correct = 0.0;
double sh_correct = 0.0;
#else
double el_correct = 3.5;
double sh_correct = -2.5;
#endif

robot_t robot;
int position = LEFT;

/* The lengths of the upper-arm, lower-arm, and the flexibility of the joints.
   (better known as the DH representation) */


static void robot_info (r)
robot_t *r;
{
	int i;
    printf("Robotnaam = %s\n", r->name);
    printf("Theta\talpha\ta\td\tecmin\tecmax\tRmin\tRmax\tRT\n");

    for (i = 0; i < NOJOINTS; i++)
    {
        printf(" %7.2lf",r->link[i].theta);
        printf(" %7.2lf",r->link[i].alpha);
        printf(" %7.2lf",r->link[i].a    );
        printf(" %7.2lf",r->link[i].d    );
        printf(" %7.2lf",r->link[i].ECmin);
        printf(" %7.2lf",r->link[i].ECmax);
        printf(" %7.2lf",r->link[i].Rmin );
        printf(" %7.2lf",r->link[i].Rmax );
        printf(" %7.2lf",r->link[i].RT   );
        printf("\n");
    }
}

static void simulate(l)
joint_list_t l;
{
    joint_list_t c;
    if (l == NULL)
    {
        printf("Can't simulate an empty list.\n");
        return;
    }
    printf("zed\tsh\telbow\tyaw\tpitch\troll\tgrip\n");
    c = l;
    do
    {
        printf("%5.1lf\t%5.1lf\t%5.1lf\t%5.1lf\t%5.1lf\t%5.1lf\t%5.1lf\n",
            c->zed, c->shoulder, c->elbow, c->yaw, c->pitch, c->roll, c->grip);

        
    } while ( (c = c->next_data) != NULL );

/*    printf("%5.2lf\t%5.2lf\t%5.2lf\t%5.2lf\t%5.2lf\t%5.2lf\t%5.2lf\n",
        l->zed, c->shoulder, l->elbow, l->yaw, l->pitch, l->roll, l->grip);
    */
    printf("Done.\n");
#ifndef ARNOUD	
#ifndef __GNUC__
    simulator_list(l);
#endif
#endif
}


static double length (x, y)
double x, y;
/* returns the length of vector (x,y) */

{
#ifdef ARNOUD
    double result;

    result = sqrt(x * x + y * y);
    if(0) printf("arnoud says: length = %10.4f\n",result);
    return( result );
#else
    return sqrt ((double) x*x + y*y);
#endif
}


static double y_angle (x, y) 
double x, y;
/* The angle of vector (x,y) with the Y-axis.
 * We would like to call this routine 'angle' but that's a compiled function
 * for scilimage. Life can be tough. Fortunately, there is vi. :-)
 */

{
    if ((x == 0.0) && (y == 0.0)) 
    {
        /* Can't calculate the y_angle of such tiny vectors. */
        return 0.0;
    }
#ifdef ARNOUD
    return(90.0 - RAD2DEG (atan2 (y, x)) );
#else
    return RAD2DEG (atan2 (x, y));
#endif
    /* atan2() should be (y,x), but we want 90 deg - atan(y,x) which is the
        same as the above. */
}


static double range (r) 
double r;
/* Make sure y_angle r is within the range <-180,180> */

{
    while (r < -180.0) 
    {
        r += 360.0;
    }
    while (r >=  180.0)  /* We prefer -180 over 180 */
    {
        r -= 360.0;
    }
    return r;
}


static double cos_formula (a, b, c)
double a, b, c;
/* Yes, it's the cosinus formula, given 3 lengths, calculate an y_angle
 * (The y_angle opposite of line a.)(and b and c are tied together.)
 */
{
    if (b*c == 0.0) 
    {
        return 0.0;
    }
    return RAD2DEG (acos ((double) (b*b + c*c - a*a) / ABS(2*b*c)));
}


static void add_kin (list, zd, shoulder, elbow, yaw, pitch, roll, grip)
joint_list_t *list;
double zd, shoulder, elbow, yaw, pitch, roll, grip;
/* Put the given z,s,e,y,p,r,g at the end of the list. 
 * so the robot will move into that position. 
 */

{
#ifdef TYPEDEF
    joint_list_t p; /* current joint_value */
    joint_list_t q;
#else
    joint_list_t p,q;
#endif

    p  = (joint_list_t ) malloc (sizeof(joint_list_item_t)); /* malloc new joint */

    /* Set the proper things for the new joint. */
    p -> zed       = zd;
    p -> shoulder  = shoulder;
    p -> elbow     = elbow;
    p -> yaw       = yaw;
    p -> pitch     = pitch;
    p -> roll      = roll;
    p -> grip      = grip;
    p -> next_data = NULL;

	if (list == NULL) printf("BIG ERROR!\n");
    /* Add it to the end of the list. */
    if (*list ==  (joint_list_t ) NULL ) /* first one? */
    {
        *list = p; /* let p be the first element */
    } else {

        q = *list;

        while ((q -> next_data) != NULL) 
        {
            q = q -> next_data;
        }
        q -> next_data = p;
    }
}


void
init_invkin () 
/* Start up routine.. Set the robot in some nice position. 
 * and find out how tall it is. 
 */
{
    joint_list_t list = NULL;
#if 0
    int fd;

    fd = open (UMI_RTX, 0); 
    if (fd < 0) 
    {
        perror(UMI_RTX);
        fprintf(stderr,"Sorry, don't know the lengths of the arm.\n"); 
        fprintf(stderr,"Can't continue.\n");
        return;
    }
    if (read (fd, &robot, sizeof (robot_t)) < sizeof (robot_t))
    {
        perror ("bad read");
        fprintf(stderr,"Sorry, couldn't read all information for the robot.\n");
        fprintf(stderr,"Maybe I could try to deduce some more information..\n");
        fprintf(stderr,"But quitting is easier.\n");
        return;
    }
    close (fd);
    printf("Read information about robot from file:\n\t'%s'.\n", UMI_RTX);
#else
    read_xml_robot (UMI_RTX, &robot);
#endif

#ifdef DEBUG
    robot_info (&robot);
#endif

#ifdef MAIN

#ifdef SIMULATION
    printf("Placing robot in standard position.\n");
#endif

    zed      = 800.0;
    shoulder = -90.0;
    elbow    = 0.0;
    yaw      = 0.0;
    pitch    = 0.0;
    roll     = 0.0;
    grip     = 0.0;


    add_kin (&list, zed, shoulder, elbow, yaw, pitch, roll, grip);

#ifdef SIMULATION
    simulate(list);
    printf("Robot should now be in the starting position.\n");
#else
#if 0
    /* move_list() is defined and tested in ../playall or ../game */
    move_list(list);
    printf("Robot should now be in the starting position.\n");
#endif
    free(list);
#endif
#endif /* MAIN */
}

static void stretch(list,sh, el, z)
joint_list_t *list;
double sh, el, z;
{
    /* elbow and shoulder etc. are still as in the last position. */
    if ( (elbow > -MARGE ) && (elbow < MARGE ) )
    { 
        /* no use to go up.. We don't make strange curves.. Only a small 
           difference..
        */
        position = (el >= 0) ? RIGHT : LEFT;
        return;
    }
#if 1
    add_kin(list, MIN(z+150.0, Rzed(Rd)),shoulder, elbow, yaw, pitch, 
        roll, grip);
    add_kin(list, MIN(z+150.0, Rzed(Rd)), sh, el, yaw, pitch, roll, grip);
#else
    add_kin(list, MIN(140.0, Rzed(Rd)),shoulder, elbow, yaw, pitch,
        roll, grip);
    add_kin(list, MIN(140.0, Rzed(Rd)), sh, el, yaw, pitch, roll, grip);
#endif
        /* When up, move into the new position. */
    add_kin(list,z, sh,el, yaw,pitch, roll,grip);
        /* Move down again. Another item will be added to the list later
           that sets the y,p,r,g to the proper position. */

    position = (el >= 0) ? RIGHT : LEFT;
}


static int in_range(sh, el)
double sh, el;
{
    /* return 0 if sh or el is out of range. Only when both are 
       in range return non-zero.
    */

    if ((sh < Rshoulder(RRmin)) || (sh > Rshoulder(RRmax)) )
    {
        return 0;
    }
    if ( (el < Relbow(RRmin)) || (el > Relbow(RRmax)) )
    {
        return 0;
    }
    return 1;
}


static int move_arm(list,x,y,z)
joint_list_t *list;
double x,y,z;
{
    double gamma, beta, alpha, sh_l, sh_r, el_l, el_r, d;
    double upper_arm, lower_arm;
    int r,l;

    upper_arm = Rshoulder(Ra); /* Just another name that makes more sense. */
    lower_arm = Relbow(Ra);

    d = length(x,y);
    /* First some simple tests.. */
#ifdef ARNOUD
    if ( y < 0 ) 
    {
        printf("Sorry, can't move the robot to that point. Too dangerous.\n");
        return -1;
    }
#endif
    if ( d > (upper_arm + lower_arm) )
    {
#ifdef ARNOUD
	shoulder = y_angle(x,y);
	elbow = 0;
#else
        printf("Sorry, too far.. Can't stretch my arm THAT far.\n");
#endif
        return -1;
    }
    alpha = cos_formula(upper_arm, d, lower_arm);
    beta = y_angle(x,y);
    gamma = cos_formula(d,upper_arm, lower_arm);
#ifdef ARNOUD
#if 0
    printf("   x',   y',    z' = %f, %f, %f\n",   x ,   y ,    z );
    printf("alpha, beta, gamma = %f, %f, %f\n",alpha, beta, gamma);
#endif
#endif
    /* y_angle: if the vector is (0,y) then the beta. is 0. 
     * When the vector is like (+100,0) the beta. is 90
     * when the vector is like (-100,0) the beta. is -90.
    */
    /* First calculate the left hand arm. */
    el_l = range(gamma - 180);
    sh_l = range(beta + alpha);
    
    /* Now try the right arm. */
    el_r = range(180.0 - gamma);
    sh_r = range(beta - alpha); 

    el_l = el_l + el_correct;
    el_r = el_r + el_correct;

    sh_l = sh_l + sh_correct;
    sh_r = sh_r + sh_correct;
    r = in_range(sh_r, el_r); /* 0 means out of range, !0, in_range */
    l = in_range(sh_l, el_l);

    zed = z; /* This is always so.. */
    wrist_angle = beta; /* global to be used in move_wrist */

    if ( (!r) && (!l) )
    {
        /* Oops, a little problem, better not try this position.. */
#ifdef ARNOUD
        printf("Can't move to (%lf,%lf,%lf).\n",x,y,z);
        printf("Calculated: Left: sh = %.2lf, el = %.2lf.\n", sh_l, el_l);
        printf("           Right: sh = %.2lf, el = %.2lf.\n", sh_r, el_r);
#endif
        return -1;
    }
    if (!r)
    {
        /* Got to do it with a left arm.. */
        if (position == LEFT)
        {
            /* We're in the right position.. */
        }
        else
        {
            /* We have to change.. */
            stretch(list, sh_l, el_l, z);
        }
        shoulder = sh_l;
        elbow = el_l;
        position = (elbow > 0) ? RIGHT : LEFT;
        return 0;
    }
    if (!l)
    {
        /* Got to do it with a right arm.. */
        if (position == RIGHT)
        {
            /* We're in the right position */
        }
        else
        {
            stretch(list, sh_r, el_r, z);
        }

        shoulder = sh_r;
        elbow = el_r;
        position = (elbow > 0) ? RIGHT : LEFT;

        return 0;
    }
    /* It can be done either way. No worries about the arm. */
    if (position == LEFT)
    {
        shoulder = sh_l;
        elbow = el_l;
    }
    else
    {
        shoulder = sh_r;
        elbow = el_r;
    }
    position = (elbow > 0) ? RIGHT : LEFT;

    return 0;
}


static int
move_wrist (nx, ny, nz, theta, d)
double nx, ny, nz, theta, d;
/* Given the 3 normals and the gripper, move the 'wrist' of the robot into
 * the proper position, so, that whatever the position of the board is, the
 * gripper will always pick up the items from the top. This should be done
 * after the rest of the arm is in the right position, because we have to
 * know the position of the elbow and shoulder.
 */

{
    double alpha, beta, gamma;

    /* calculate pitch from normal-vector, angle of -90.0 is pointing   *
     * straight down                                                    */
    alpha = -90.0 + y_angle (length (nx, ny), nz);

    /* correct angle if it is out of range this is done by turning the  *
     * wrist around 180.0 degrees                                       */
    if ((alpha < Rpitch(RRmin)) || (alpha > Rpitch(RRmax)))
    {
        alpha = range (-180.0 - alpha); /* flip alpha around z-axis     */
        ny    = -ny;                    /* turning vector 180.0 degrees */
        nx    = -nx;                    /* around the z-axis            */
    }

    /* if the normal-vector doesn't point straight up, the yaw is       *
     * needed to keep the gripper parallel with the normal vector       *
     * otherwise the yaw is used to keep the mouth aligned with the     *
     * chessboard.                                                      */
#ifdef ARNOUD
    if(COUPLED) {
		wrist_angle = 0.0;
	}

    if (length (nx, ny) < 0.001 )
    {
		beta  = range( -1.0 * wrist_angle );
    } else {
		beta  = range (y_angle ((-1.0 * nx), (-1.0 * ny)) - wrist_angle);
    }
	gamma = range (-1.0 * theta);
#else

    if ((nx != 0.0) || (ny != 0.0))
    {
        /* the '- shoulder - elbow' is needed to correct the rotation   *
         * from the shoulder and elbow                                  */
#if 0             
        beta  = range (y_angle (nx, ny) - shoulder - elbow);
#else
	/* coupled */
	beta  = range (y_angle (nx, ny));
#endif
        gamma = range (theta - y_angle (nx, ny));
    }
    else
    {
#if 0
        beta  = range (- shoulder - elbow - theta);
#else
	/* coupled */
	beta  = range (-theta);
#endif
        gamma = 0.0;
    }
#endif
    /* correct angle if it is out of range, this causes the pitch-angle *
     * to flip around the z-axis, the roll doesn't change, because of   *
     * the symetry of the gripper.                                      */
    if ((beta < Ryaw(RRmin)) || (beta > Ryaw(RRmax)))
    {
        alpha = range (-180.0 - alpha);
        beta  = range (beta + 180.0);
    }

    /* correct angle if it is out of range.                             */
    if ((gamma < Rroll(RRmin)) || (gamma > Rroll(RRmax)))
    {
        gamma = range (gamma + 180.0);
        alpha = -90.0 - y_angle (length (nx, ny), nz);
    }

    /* check angle if it is out of range.                               */
    if ((alpha < Rpitch(RRmin)) || (alpha > Rpitch(RRmax)))
    {
#ifdef ARNOUD
#if 1
        printf ("couldn't reach position (pitch = %f,yaw = %f).\n",alpha,beta);
#endif
#else
        printf ("couldn't reach position.\n");
#endif
        return -1;
    }
    d = MIN (Rgrip(RRmax), d); /* make sure it doesn't exceed the max.   */
    d = MAX (Rgrip(RRmin), d); /* make sure it doesn't exceed the min.   */

    yaw   = beta;
    pitch = alpha;
    roll  = gamma;
    grip  = d;

    if(0) printf("yaw, pitch, roll= %f, %f, %f\n",beta,alpha,gamma);
    return 0;
}


static int
move_robot (list, x, y, z, nx, ny, nz, theta, d)
joint_list_t *list;
double x, y, z, nx, ny, nz, theta, d;

{
    int r1, r2;

#ifndef ARNOUD
    r1 = move_arm(list, x - nx*Rroll(Rd), y - ny*Rroll(Rd), z+ nz*Rroll(Rd) + zsafe);
#else
    r1 = move_arm(list, x + nx*Rroll(Rd), y + ny*Rroll(Rd), z+ nz*Rroll(Rd) + zsafe);
#endif
    r2 = move_wrist (nx, ny, nz, theta, d);

    add_kin (list, zed, shoulder, elbow, yaw, pitch, roll, grip);

    position = (elbow > 0)? RIGHT: LEFT;
#ifdef ARNOUD
    if (r1 < 0) printf ("couldn't get arm in position\n");
    if (r2 < 0) printf ("couldn't get wrist in position\n");
    if (0 && r1 >= 0 && r2 >= 0) printf ("arm and wrist in position\n");
#endif

    return ((r1 < 0) || (r2 < 0))?-1:0;
}


static void
tryout (x, y, z, nx, ny, nz, theta, d)
double x, y, z, nx, ny, nz, theta, d;

{
    joint_list_t list = NULL;
    if (move_robot (&list, x, y, z, nx, ny, nz, theta, d) < 0)
    {
        fprintf (stderr, "can't reach given point.\n");
    }
    simulate(list);
}

static void 
move_back(jlist)
joint_list_t *jlist;
{
    zed = MIN(zed + zmargin, Rzed(RRmax));

    if (elbow >= 0.0)
    {
        /* Swing the arm to the left.. */
        elbow = 90.0; /* To avoid hitting something. */
        shoulder = 90.0;
    }
    else
    {
        /* Swing to the right.. */
        elbow = -90.0;
        shoulder = -90.0;
    }

    add_kin(jlist, zed, shoulder, elbow,yaw, pitch, roll, grip);
}



void
inverse_kinematics (xyz_list, joint_list)
xyz_list_t xyz_list;
joint_list_t *joint_list;

{
    while (xyz_list != (xyz_list_t ) NULL) /* while moves left */
    {
        double x, y, z, n1, n2, n3, theta, d;

        x     = xyz_list -> x;
        y     = xyz_list -> y;
        z     = xyz_list -> z;
        n1    = xyz_list -> n1;
        n2    = xyz_list -> n2;
        n3    = xyz_list -> n3;
        theta = xyz_list -> theta;
        d     = xyz_list -> grip;

        if(0) printf("Going to move robot to (%.1lf, %.1lf, %.1lf)\n",x,y,z);
        if (move_robot (joint_list, x, y, z, n1, n2, n3, theta, d) < 0)
        {
            fprintf(stderr,"Oops, prepare for a masochistic robot!\n");
        }

        xyz_list = xyz_list -> next_data; /* look at next elt in movelist */
    }
#ifndef ARNOUD
    move_back(joint_list);
#endif
}



static void
goto_xy(x,y)
double x,y;
{
    xyz_list_item_t xyz; /* Normally a pointer, but then I have to malloc.. */
    joint_list_t jlist = NULL;

    xyz.x = x;
    xyz.y = y;
    xyz.z = 0.0;
    xyz.grip = 0.0;
    xyz.n1 = 0.0;
    xyz.n2 = 0.0;
    xyz.n3 = 1.0;
    xyz.theta = 0.0;
    xyz.next_data = NULL;

    inverse_kinematics(&xyz, &jlist);
    simulate(jlist);
}


static void go_to(x,y)
int x;
int y;
{
    double x1,y1;
    x1 = (double) x;
    y1 = (double) y;

    printf("goto_xy(%.2lf,%.2lf).\n",x1,y1);
    goto_xy(x1,y1);
}

#ifdef MAIN  
int main ()
{
#ifdef MYINVKIN
    simchess();
#else
    int x,y;
    init_invkin();
#endif
    return 0;
}
#endif
