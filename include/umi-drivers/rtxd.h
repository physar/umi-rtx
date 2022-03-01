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
 * defines for the daemon
 */

#ifndef _rtxd_h
#define _rtxd_h

#include <sys/socket.h>
#include <sys/un.h>
#include <netinet/in.h>


#define RTXD_DIR		"/opt/prac/robotics/rtxipc"
#define RTXD_LOCK		"/opt/prac/robotics/rtxipc/locks/rtx"
#define RTXD_LOG		"/opt/prac/robotics/rtxipc/logs/rtxlog"
/* Full pathname, must be used from programs to */
#define RTXD_SOCKET		"/opt/prac/robotics/rtxipc/ports/rtx-socket"
#define RTXD_PORTINFO		"/opt/prac/robotics/rtxipc/ports/rtx"
#define RTXD_DOWNLOAD		"/opt/prac/robotics/rtxipc/robot/rtxdriver"
#ifdef LINUX
#define RTXD_PATH		"/opt/prac/robotics/rtxipc/robot/linux/rtxd"
#else
#define RTXD_PATH		"/opt/prac/robotics/rtxipc/robot/solaris/rtxd"
#endif

typedef struct client {
    struct client       *c_next;
    int                 c_fd;
    int                 c_pid;
    int                 c_uid;
    int                 c_flags;
    time_t              c_last;
    time_t              c_first;
    union {
        struct sockaddr_un C_un;
        struct sockaddr_in C_in;
    } C_u;
} client;

#define c_un C_u.C_un
#define c_in C_u.C_in

void rtx_log(client *clnt, ...); 
#endif /* _rtxd_h */
