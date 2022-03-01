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
/* Copyright (c) 1991-2022 Universiteit van Amsterdam                     */
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
 * Constants of the runtime environment.
 *
 * I.e. constants not defined by the IPC environment,
 * the mc680x0 manual or the DUART reference sheet.
 */

#ifndef _ipcrt_h
#define _ipcrt_h

#include "ipc.h"
#include "duart.h"

#if !defined(ASM) && defined(IPC)
unsigned long mtime();
#endif

#define DUARTVECTOR		AUTOVECTOR(2)

#define VECTORBASE		DRAM
#define TEXTSTART		(DRAM+0x02000)
#define STACK			(DRAM+0x100000)

/* after the vectors */
#define CLOCK			(DRAM+0x400)
#define CLOCKFREQ		(CLOCK+4)
#define FILESTART		(CLOCKFREQ+4)
#define TEXTSUM			(FILESTART+4)
#define DATASUM			(TEXTSUM+4)
#define PARAMPTR		(DATASUM+4)
#define RUNSTATUS		(DRAM+0x500)
#define EXITVAL			(RUNSTATUS+4)
#define DUMPSP			(EXITVAL+4)
#define DUMPPC			(DUMPSP+4)
#define DUMPSR			(DUMPPC+4)
#define DUMPREGS		(DUMPSR+4)
#define STATUSBASE		(DRAM+0x600)
#define STATUSMAX		0x300
#define PROGNAME		(STATUSBASE+STATUSMAX)
#define PROGNAMEMAX		0x100
#define STATUSMAGIC		0x12345678

#define INDRAM(x)		((x) >= IPCOFFSET(DRAM) && \
					(x) <= IPCOFFSET(STACK))
#define INSRAM(x)		((x) >= 0x400 && (x) <= 0x20000)
#define INRAM(x)		(INDRAM(x) || INSRAM(x))
	

#define RUNNING			0
#define EXITED			1
#define ABORTED			2
#define BADTRAP			3

#endif /* _ipcrt_h */
