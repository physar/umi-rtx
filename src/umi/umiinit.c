/**************************************************************************/
/* umiinit.c                                                    /\/\      */
/* Version 2.1   --  July 2011                                  \  /      */
/*                                                              /  \      */
/* Author: Casper Dik, Arnoud Visser                         _  \/\/  _   */
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
/* Release note 1.0: originally init.pas, but now in C          \/\/      */
/*                   No changes, except for move elbow & yaw.             */
/*                   Casper, Augustus 1991                                */
/* Release note 2.0: originally standalone init.c, but now in umi_software*/
/*                   No changes, except second initialise_zed             */
/*                   Arnoud, November 1994                                */
/* Release note 2.1: Removed warnings about impropper use of exit         */
/*                   Arnoud, July 2011                                    */
/**************************************************************************/
/* Initialises the arm, by driving to the end-stops, followed by a        */
/* define_home                                                            */
/*      void umi_initialise_zed();                                        */
/*      void umi_initialise_gripper();                                    */
/*      void umi_initialise_wrist();                                      */
/*      void umi_initialise_shoulder();                                   */
/*      void umi_initialise_elbow_and_yaw();                              */
/*                                                                        */
/*      void umi_read_init_data();                                        */
/*                                                                        */
/*      void umi_set_limit_position();                                    */
/*      void umi_goto_init_position();                                    */
/**************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "rtx.h"
#include "umiinit.h"
#include "armlib.h"

#define STAT_TIP 1
#define STAT_STOP 2
#define STAT_ELE 4
#define STAT_RES 8

static int limit[NUMBER_OF_MOTORS];
static int post_init[NUMBER_OF_MOTORS];
static float counts_per_real[NUMBER_OF_MOTORS] = {
	14.611, 29.221, 3.750, 13.487, 13.487, 9.740, 14.706, 1.0 };

#ifdef NEEDED
static void
error_handler(int code, int address)
{
    fprintf(stderr,"Error %x\n",code);
    armerrno = 0;
    arm_restart();
    if (armerrno) {
	fprintf(stderr,"fatal\n");
	exit(1);
    }
}
#endif /* NEEDED */
static int 
hit_end_stop(int m)
{
    int status;

    arm_motor_status(m,&status);
    return status & STAT_RES;
}

static void
drive_one_motor(int mode,int motor,int state)
{
    arm_stop(DEAD_STOP);
    arm_go(mode,GO_BITS_FOR(motor,state));
}

static void
drive_wrist(int mode,int w1_s,int w2_s)
{
    arm_stop(DEAD_STOP);
    arm_go(mode,GO_BITS_FOR(WRIST1,w1_s)|GO_BITS_FOR(WRIST2,w2_s));
}

static void
drive_until_done(int m)
{
    int status;

    drive_one_motor(NUMERIC,m,FWD);
    do {
	arm_motor_status(m,&status);
    } while ((status & STAT_TIP) && armerrno == 0);
}

static void
drive_against_end_stop(int m,int dir)
{
    drive_one_motor(MANUAL,m,dir);
    while (!hit_end_stop(m) && armerrno == 0)
	;
}

static void
drive_wrist_against_end_stop(int w1,int w2)
{
    drive_wrist(MANUAL,w1,w2);
    while ((!hit_end_stop(WRIST1) || !hit_end_stop(WRIST2)) && armerrno == 0)
	;
}

void
umi_set_limit_position()
{
    int i;
    for (i = 0; i < ZEDOWN; i++)
	arm_write(i,CURRENT_POSITION,limit[i]);
}

void
umi_goto_init_position()
{
    int i;
    for (i = 0; i < ZEDOWN; i++)
	arm_write(i,NEW_POSITION,post_init[i]);
    arm_go(NUMERIC,0x1555);
}

void
umi_initialise_zed()
{
    if (!armerrno) {
	printf("Initialising Zed\n");
	drive_against_end_stop(ZED,FWD);
	arm_define_origin();
    }
}

void
umi_initialise_gripper()
{
    int gforce;
    if (!armerrno) {
	printf("Initialising Gripper\n");
	arm_read(GRIP,MAX_FORCE,&gforce);
	arm_write(GRIP,MAX_FORCE,gforce/2);
	drive_against_end_stop(GRIP,RVS);
	arm_define_origin();
	arm_reload_pids();
	arm_write(GRIP,NEW_POSITION,30);
	drive_until_done(GRIP);
	arm_define_origin();
	arm_reload_pids();
    }
}

static void
move_elbow_out_a_bit()
{
    printf("move elbow out ..\n");
    arm_write(YAW,MAX_FORCE,0);
    arm_write(ELBOW,NEW_POSITION,440);
    drive_until_done(ELBOW);
    printf("move elbow out done..\n");
    arm_define_origin();
    arm_reload_pids();
}

void
umi_initialise_wrist()
{
    if (!armerrno) {
	printf("Initialising Wrist\n");
	move_elbow_out_a_bit();
	drive_wrist_against_end_stop(RVS,FWD);
	drive_wrist_against_end_stop(RVS,RVS);
	drive_wrist_against_end_stop(RVS,FWD);
	drive_wrist_against_end_stop(RVS,RVS);
	arm_define_origin();
	arm_reload_pids();
    }
}

void
umi_initialise_shoulder()
{
    if (!armerrno) {
	printf("Initialising Shoulder\n");
	drive_against_end_stop(SHOULDER,FWD);
	arm_define_origin();
	arm_reload_pids();
    }
}

void
umi_initialise_elbow_and_yaw()
{
    if (!armerrno) {
	printf("Initialising Elbow and Yaw\n");
	arm_write(ELBOW,SPEED,80);
	arm_write(ELBOW,MAX_FORCE,30);
	arm_write(YAW,SPEED,100);
	arm_write(YAW,MAX_FORCE,60);
	arm_write(YAW,ERROR_LIMIT,1500);

	arm_stop(DEAD_STOP);
	arm_go(MANUAL,GO_BITS_FOR(ELBOW,RVS)|GO_BITS_FOR(YAW,RVS));

	while ((!hit_end_stop(ELBOW) || !hit_end_stop(YAW)) && armerrno == 0)
	    ;
	arm_define_origin();
	arm_reload_pids();
    }
}

#define FZED		0
#define FSHOULD		1
#define FELBOW		2
#define	FYAW		3
#define	FPITCH		4
#define	FROLL		5
#define	FGRIP		6
#define FNUMBER		7

static float f_limit[FNUMBER] = {
   0.0, 90.0, -180.0, -200.0, -98.0, -132.0, 0.0
};
static float f_post_init[FNUMBER] = {
   -5.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
};

void
umi_read_init_data()
{
    FILE *idata = fopen("../data/init.dat","r");
    char buf[128];
    int m = 0;
    if (idata == 0) {
	fprintf(stderr,"Can't open the data file 'init.dat'\n");
	fprintf(stderr,"Using data from program\n");
    } else {
	while (fgets(buf,128,idata) && *buf != '.');
	while (fgets(buf,128,idata)) {
	    if (sscanf(buf,"%f %f",f_limit+m,f_post_init+m) != 2) {
		fprintf(stderr,"Invalid init file\n");
		exit(1);
	    }
	    m++;
	    if (m > FNUMBER)
		break;
	}
	fclose(idata);
	if (m != FNUMBER) {
	    fprintf(stderr,"%d items in init file, expected %d\n", m, FNUMBER);
	    exit(1);
	}
    }
    limit[ELBOW] = f_limit[FELBOW]*counts_per_real[ELBOW] + 0.5;
    post_init[ELBOW] = f_post_init[FELBOW]*counts_per_real[ELBOW] + 0.5;

    limit[SHOULDER] = f_limit[FSHOULD]*counts_per_real[SHOULDER] + 0.5;
    post_init[SHOULDER] = f_post_init[FSHOULD]*counts_per_real[SHOULDER] + 0.5;

    limit[ZED] = f_limit[FZED]*counts_per_real[ZED] + 0.5;
    post_init[ZED] = f_post_init[FZED]*counts_per_real[ZED] + 0.5;

    limit[WRIST1] = (f_limit[FPITCH] + f_limit[FROLL])
		*counts_per_real[WRIST1] + 0.5;
    post_init[WRIST1] = (f_post_init[FPITCH] + f_post_init[FROLL])
		*counts_per_real[WRIST1] + 0.5;

    limit[WRIST2] = (f_limit[FPITCH] - f_limit[FROLL])
		*counts_per_real[WRIST2] + 0.5;
    post_init[WRIST2] = (f_post_init[FPITCH] - f_post_init[FROLL])
		*counts_per_real[WRIST2] + 0.5;

    limit[YAW] = f_limit[FYAW]*counts_per_real[YAW] + 0.5;
    post_init[YAW] = f_post_init[FYAW]*counts_per_real[YAW] + 0.5;

    limit[GRIP] = f_limit[FGRIP]*counts_per_real[GRIP] + 0.5;
    post_init[GRIP] = f_post_init[FGRIP]*counts_per_real[GRIP] + 0.5;
}

#ifdef MAIN
static void
arm_init_position()
{
    umi_read_init_data();
    arm_restart();
    arm_stop(FREE_OFF);

    umi_initialise_zed();
    umi_initialise_gripper();
    umi_initialise_wrist();
    umi_initialise_shoulder();
    umi_initialise_elbow_and_yaw();
    umi_initialise_zed();

    if (!armerrno) {
	printf("\nGoing to initial position...");
	umi_set_limit_position();
	umi_goto_init_position();
    }
    putchar('\n');
    if (!armerrno) {
	printf("RTX is now initialised and ready to use\n");
    } else {
	printf("RTX can not be initialised!\n");
	printf("Error: %s\n",armstrerror(armerrno));
    }
}

int
main()
{
    int debug = 0;

    armerrno = 0;
    printf("U.M.I. RTX Robot Arm\n");
    printf("====================\n");
    printf("Initialisation Program $Revision$\n");
    arm_init_comms(1,debug);
    arm_init_position();
    exit(armerrno != 0);
}
#endif /* MAIN */
