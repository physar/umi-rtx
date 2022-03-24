/**************************************************************************/
/* umicmds.c (former hardware_joints.c)                         /\/\      */
/* Version 2.1   --  July 2011                                  \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam, Matthijs Spaan      _  \/\/  _   */
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
/* Release note 2.1: added umi_stop()                           \/\/      */
/**************************************************************************/
/* translate joint angles into encoder values                             */
/* 	umi_move(): convert angles, send values and go!                   */
/*      umi_init(): load commands, and move all motors to the endstop     */
/*      umi_moving(): busy or not?                                        */
/*      umi_stop(): stop all movement                                     */
/**************************************************************************/

/* doesn't work  #define __USE_POSIX // for L_cuserid */
#define L_cuserid 9 /* from <bits/stdio_lim.h */
#include <unistd.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "rtx.h"
#include "rtxcmds.h"
#include "umi.h"
#include "umiinit.h"
#include "armlib.h"

#define FORWARD_ZED	16
#define FORWARD_SLD	4
#define FORWARD_ELB	1
#define FORWARD_YAW	1024
#define FORWARD_WR1	256
#define FORWARD_WR2	64
#define FORWARD_GRP	4096
#define FORWARD_ALL	5461

#define ZED_LOW		-3478
#define ZED_HIGH	-20   /* official 0 some strange noise occur  */
#define SLD_LOW		-2630
#define SLD_HIGH	2630
#define ELB_LOW		-2630
#define ELB_HIGH	2206
#define YAW_LOW		-1071
#define YAW_HIGH	1071
#define GRP_LOW		-30
#define GRP_HIGH	1200

#define ADD_W1W2_MIN	-2642
#define ADD_W1W2_MAX	108
#define SUB_W1W2_MIN	-3560
#define SUB_W1W2_MAX	4882

#define WRONG		-1
#define OK               0

#ifdef SCIENCEPARK
#define Name_File "/opt/prac/robotics/rtxipc/robot/name_file"
#else
#define Name_File "../data/name_file"
#endif

static  char    user_name[L_cuserid];
static	int	initiation_ready;
static  int	enc_zed, enc_sld, enc_elb, enc_yaw, enc_wr1, enc_wr2, enc_grp;

static	void	print_status(/*int status*/);
static void umi_limit();
static void umi_go();

int umi_moving()
/* return 1 if arm is busy else 0 */
{
	int stat;

	arm_general_status(&stat);
	if ((stat & RESP_MASK_TASKS) == 1)
		return 1;
	else if (!initiation_ready)
		return 1;
	else
		return 0;
}

void umi_init()
{
        int     cnt;
        int     stat;
        int     uis;
        int     debug = 0;

        if(arm_general_status(&stat) == OK) {

             if(0) printf("\nCURRENT STATUS\n");
             if(0) print_status(stat);

             if(!(stat & (RESP_MASK_ERR_LIMITS |
                          RESP_MASK_AXIS_RESET |
                          RESP_MASK_USER_IO_CONF) )) {
                  if(1) printf("\nRTX ready to use\n");
                  initiation_ready = 1;
                  return;
             }
        }

	/*
         * no unauthorised access to the robot, also not interactively
         */
        //cuserid(user_name);
        getlogin_r(user_name,L_cuserid);
        uis = UserIsSpecial(user_name);

        if(!uis) {
          printf("No access rights for %s\n",user_name);
          printf("Contact the practicum-assistants\n");
          return;
        }

        printf("U.M.I. RTX Robot Arm\n");
        printf("====================\n");
        printf("Initialisation Program, UMI version 19 June 2014\n");

        /*
         * Load the commands to the server                    
         */
        initiation_ready = 0;
        if ((arm_init_comms(1,debug)) == WRONG)
                armperror("init connections ");
        if (arm_restart() == WRONG)
                armperror("restart");

        if (arm_stop(FREE_OFF) == WRONG)
                armperror("enable PWM output");

        umi_initialise_zed();
        umi_initialise_gripper();
        umi_initialise_wrist();
        umi_initialise_shoulder();
        umi_initialise_elbow_and_yaw();

        umi_read_init_data();

        umi_set_limit_position();
        umi_goto_init_position();

        if(!armerrno) {
                printf("\nRTX is now external initialised\n");
                initiation_ready = 1;
                return;
        }

        if ((arm_soak(INIT)) == WRONG)
                armperror("soak init");

        cnt = 16;

        while (cnt--)
        {
                if (arm_general_status(&stat) == WRONG)
                        armperror("general_status");

                printf(".");
#if 0
                fflush(stdout);
#endif
                sleep(5);
        }
        initiation_ready = 1;
        printf("\nRTX is now internal initialised\n");
}

void umi_stop()
{
       if (arm_stop(FREE_STOP) == WRONG)
                armperror("tried FREE_STOP");

       sleep(1); /* wait a second */

       if (arm_stop(FREE_OFF) == WRONG)
                armperror("tried FREE_OFF");
}

void umi_move(zed, shoulder, elbow, yaw, pitch, roll, gripper)
double zed, shoulder, elbow, yaw, pitch, roll, gripper;
{
        double j_wrist1, j_wrist2, j_yaw, j_zed;
        static  int initia = 0;

        if (!initia)
        {
                umi_init();
                initia = 1;
        }

        j_wrist1 = 0.5 * (roll + pitch);
        j_wrist2 = 0.5 * (pitch - roll);
        j_yaw = yaw + (elbow/2);
        j_zed = zed; /* correct this */

        enc_zed = ZED_LOW + ((3478.0 / 925.0) * j_zed);
        enc_sld = (((5260.0 / 180.0) * shoulder));
        enc_elb = (((4836.0 / 331.0) * elbow));
        enc_yaw = (((2142.0 / 220.0) * j_yaw));
        enc_wr1 = (((2750.0 / 102.0) * j_wrist1));
        enc_wr2 = (((8442.0 / 313.0) * j_wrist2));
        enc_grp = GRP_LOW + ((1230.0 / 90.0 ) * gripper);

        umi_limit();
        umi_go();
        /* um_print(); */
}

static void
umi_go()
{
        int errno;	/* armerrno is not reset after a succesful arm_cmnd */

        if(!initiation_ready) {
            printf("No access rights for %s\n",user_name);
            printf("Contact the practicum-assistants\n");
            return;
        }

	if (arm_write(ZED, NEW_POSITION, enc_zed) == WRONG)
		armperror("position error with zed ");

	if (arm_write(SHOULDER, NEW_POSITION, enc_sld) == WRONG)
		armperror("position error with sld ");

	if (arm_write(ELBOW, NEW_POSITION, enc_elb) == WRONG)
		armperror("position error with elb ");

	if (arm_write(YAW, NEW_POSITION, enc_yaw) == WRONG)
		armperror("position error with yaw ");

	if (arm_write(WRIST1, NEW_POSITION, enc_wr1) == WRONG)
		armperror("position error with wr1 ");

	if (arm_write(WRIST2, NEW_POSITION, enc_wr2) == WRONG)
		armperror("position error with wr2 ");

	if (arm_write(GRIPPER, NEW_POSITION, enc_grp) == WRONG)
		armperror("position error with grp ");


        do {

                if (arm_go(NUMERIC, FORWARD_ALL) == WRONG) {
                        errno = armerrno;
                        if(errno != ARM_AXIS_BUSY) {
                                armperror("error with arm_go ");
                        }
                } else {
                        errno = 0;
                }
        }while(errno == ARM_AXIS_BUSY || errno == NO_RESPONSE);
}

#ifdef NEEDED
static void 
umi_print()
{
	printf("encoder values to arm %d %d %d %d %d %d %d\n", enc_zed, enc_sld,
			 enc_elb, enc_yaw, enc_wr1, enc_wr2, enc_grp);
}
#endif /* NEEDED */

static void
umi_limit()
{
	if (enc_zed < ZED_LOW)
		enc_zed = ZED_LOW;
	else if (enc_zed > ZED_HIGH)
		enc_zed = ZED_HIGH;


	if (enc_sld < SLD_LOW)
		enc_sld = SLD_LOW;
	else if (enc_sld > SLD_HIGH)
		enc_sld = SLD_HIGH;

	if (enc_elb < ELB_LOW)
		enc_elb = ELB_LOW;
	else if (enc_elb > ELB_HIGH)
		enc_elb = ELB_HIGH;

	if (enc_yaw < (YAW_LOW + (enc_elb/3)))
		enc_yaw = (YAW_LOW + (enc_elb/3));
	else if (enc_yaw > (YAW_HIGH + (enc_elb/3)))
		enc_yaw = (YAW_HIGH + (enc_elb/3));


	if (enc_grp < GRP_LOW)
		enc_grp = GRP_LOW;
	else if (enc_grp > GRP_HIGH)
		enc_grp = GRP_HIGH;

	if ((enc_wr1+enc_wr2) > ADD_W1W2_MAX)
		enc_wr1 = enc_wr2 = 0;
	else if ((enc_wr1+enc_wr2) < ADD_W1W2_MIN)
		enc_wr1 = enc_wr2 = 0;
	else if ((enc_wr1-enc_wr2) > SUB_W1W2_MAX)
		enc_wr1 = enc_wr2 = 0;
	else if ((enc_wr1-enc_wr2) < SUB_W1W2_MIN)
		enc_wr1 = enc_wr2 = 0;
}

int UserIsSpecial(char name[])
{
        int  found;
        FILE *fp;
        char *s1;
        char *s2;
        char *cp;
        int  i;

        s1 = (char *) malloc(80*sizeof(char));
        s2 = (char *) malloc(80*sizeof(char));

        cp = s1;
        for (i=0;i<40;i++)
        {
                *cp = name[i];
                cp++;
        }


        found = 0;
        fp = fopen(Name_File,"r");
        if (fp == NULL)
        {
                printf("No such file %s\n",Name_File);
                return(0);
        }

        while ((!found) && ((fscanf(fp, "%s", s2))!=EOF))
        {
                if (!(strcmp(name, s2)))
                        found = 1;

                getc(fp);
        }

        fclose(fp);
        return found;
}

static void print_status(status)
int status;
{
        printf("status = %d(0x%x)\n",status,status);
        if (status & RESP_MASK_TASKS)
            printf("Some tasks in progress\n");
        if (status & RESP_MASK_AXIS)
            printf("Some axis are stopped\n");
        if (status & RESP_MASK_ERR_LIMITS)
            printf("Some error limits exceeded\n");
        if (status & RESP_MASK_AXIS_RESET)
            printf("Some axis have been reset\n");
        if (status & RESP_MASK_USER_IO)
            printf("User I/O changed since last read\n");
        printf("User I/O configuration: %sput\n",
                (status & RESP_MASK_USER_IO_CONF)?"in":"out");
}

