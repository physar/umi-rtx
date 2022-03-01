/**************************************************************************/
/*                                                              /\/\      */
/*                                                              \  /      */
/*                                                              /  \      */
/* Author: Arnoud Visser, Joris van Dam, Casper Dik          _  \/\/  _   */
/*         University of Amsterdam                          | |      | |  */
/*         Dept. of Computer Systems                        | | /\/\ | |  */
/*         Kruislaan 403, NL 1098 SJ Amsterdam              | | \  / | |  */
/*         THE NETHERLANDS                                  | | /  \ | |  */
/*                                                          | | \/\/ | |  */
/*                                                          | \______/ |  */
/* This software has been written for the robot course       \________/   */
/* at our department. No representations are made about                   */
/* the suitability of this software for any purpose other       /\/\      */
/* than education.                                              \  /      */
/*                                                              /  \      */
/*                                                              \/\/      */
/* This file is copyright protected.                                      */
/* Copyright (c) 1991-2003 Universiteit van Amsterdam                     */
/* This software or any part thereof may only be used for non-commercial  */
/* research or purposes, as long as the author and University are         */
/* mentioned. Commercial use without explicit prior written consent by    */
/* the Universiteit van Amsterdam is strictly prohibited. This copyright  */
/* notice must be included with any copy of this software or any part     */
/* thereof.                                                               */
/*                                                                        */
/* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS    */
/* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT      */
/* LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR  */
/* A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT   */
/* OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,  */
/* SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT       */
/* LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,  */
/* DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY  */
/* THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT    */
/* (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE  */
/* OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.   */
/**************************************************************************/
/*
 * $Id$
 *
 * RTX constants.
 */

#ifndef _rtx_h
#define _rtx_h

 /* communications faults */
#define	COMMS_FAULT		0x10
#define	COMMS_NOT_READY		0x11
#define	COMMS_NOT_INITIALISED	0x12

 /* response errors */
#define	NO_RESPONSE		0x20
#define	RESPONSE_OVERRUN	0x21
#define	RESPONSE_INCOMPLETE	0x22
#define	RESPONSE_UNKNOWN	0x23

 /* arm warnings */
#define	ARM_IN_PROGRESS		0x30
#define	ARM_STORED		0x31
#define	ARM_AXIS_BUSY		0x32
#define	ARM_DECODER_BUSY	0x33
#define	ARM_PARAMETER_OOR	0x34

 /* arm errors */
#define	ARM_READ_ONLY		0x40
#define	ARM_SELECTION_OOR	0x41
#define	ARM_COMMAND_OOR		0x42
#define	ARM_NOT_SUPPORTED	0x43
#define	ARM_FRAME_TIMEOUT	0x44
#define	ARM_FRAME_OVERRUN	0x45
#define	ARM_PARITY		0x46
#define	ARM_RESTARTED		0x47

 /* library errors */
#define	SELECTION_OOR		0x50
#define	CHECKSUM		0x51
#define	READ_ONLY		0x52
#define	PARAMETER_OOR		0x53
#define	TOGGLE_MODE_OFF		0x54
#define	PRIVILEGE_VIOLATION	0x55 /* Execution of unprivileged commands */
#define ARM_IN_USE		0x56 /* Arm is in use by other process */
#define COMMAND_OOR		0x57 /* Unimplemented command */
#define LOST_CONNECTION		0x58 /* The connection to the daemon was lost */
#define NO_CONNECTION		0x59 /* No connection could be made */
#define CONNECTION_READ_ONLY	0x5a /* the connection was opened read only */

/*
 * The motors
 */
#define ELBOW			0
#define SHOULDER		1
#define ZED			2
#define WRIST1			3
#define WRIST2			4
#define YAW			5
#define	GRIP			6
#define	GRIPPER			6
#define	ZEDOWN			7
#define NUMBER_OF_MOTORS	8

/*
 * The modes
 */
#define POSITION_MODE		0
#define FORCE_MODE		1
#define ABSOLUTE_MODE		2
#define RELATIVE_MODE		3
#define USER_INPUT		4
#define USER_OUTPUT		5
#define NUMBER_OF_MODES 	6

/*
 * Stop modes
 */
#define DEAD_STOP		0
#define RAMP_STOP		1
#define FREE_STOP		2
#define FREE_OFF		3
#define NUMBER_OF_STOP_MODES	4

/*
 * Go modes
 */
#define MANUAL_GO		0
#define MANUAL			0
#define NUMERIC_GO		1
#define NUMERIC			1
#define NUMBER_OF_GO_MODES	2

/*
 * The parameters
 */
#define ERROR			 0
#define CURRENT_POSITION	 1
#define ERROR_LIMIT		 2
#define NEW_POSITION		 3
#define SPEED			 4
#define KP			 5
#define KI			 6
#define KD			 7
#define DEAD_BAND		 8
#define OFFSET			 9
#define MAX_FORCE		10
#define CURRENT_FORCE		11
#define ACCELERATION_TIME	12
#define USER_RAM		13
#define USER_IO			14
#define ACTUAL_POSITION		15
#define NUMBER_OF_DATA_CODES	16

/*
 * The soak modes
 */
#define INIT			0
#define	SOAK_ON			1
#define	INIT_SOAK		2
#define SOAK_OFF		3
#define NUMBER_OF_SOAK		4

#ifndef ASM
#if defined(IPC) || defined(RTXD)
extern int rtxerr;
#else /* IPC */
extern int armerrno;
extern char *armstrerror();
#endif /* IPC */
#endif /* ASM */

#define STOP	0
#define FWD	1
#define RVS	2
#define OFF	3

#define GO_BITS_FOR(m,s) ((s) << ((m) << 1))

#endif /* _rtx_h */
