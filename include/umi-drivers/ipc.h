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
 * The address constants of the IPC.
 */
#ifndef _ipc_h
#define _ipc_h

/*
 * On board, the IPCBASE is 0. (the high bits are masked)
 * When mmap'ed from a sun, the mapped address must be known in
 * ipcbase.
 */
#ifdef IPC
#define IPCBASE		0x00000000
#else
#define IPCBASE		ipcbase
extern char *ipcbase;
#endif

#define	SRAM		(IPCBASE + 0x00000000)
#define ACQADR		(IPCBASE + 0x00040003)
#define	HCNTR		(IPCBASE + 0x00040012)
#define	VCNTR		(IPCBASE + 0x00040020)
#define	OFGAIN		(IPCBASE + 0x00040032)
#define	VIDCTRL		(IPCBASE + 0x00040042)
#define	CLUT		(IPCBASE + 0x00040053)

#define	IGALOW		(IPCBASE + 0x00080000)
#define	DUART		(IPCBASE + 0x000a0002)
#define	INTV		(IPCBASE + 0x000c0003)
#define	DRAM		(IPCBASE + 0x00100000)
#define	VRAM		(IPCBASE + 0x00200000)
#define	IGA		(IPCBASE + 0x00300000)

/* compute the offset from the start of the IPC */
#define IPCOFFSET(x)	((x) - IPCBASE)

/*
 * Output port bits
 */
#define	RTSA		0x01
#define	RTSB		0x02
#define RUNCPU		0x04
#define BUSOPEN		0x08
#define WRITETRANS	0x10
#define DIISCD		0x20
#define	NORMACCES	0x40
#define INT3		0x80

/*
 * Input port bits
 */

#define	CTSA		0x01
#define	CTSB		0x02
#define ACQDONE		0x04
#define	SYNFAIL		0x08
#define	EFEVEN		0x10
#define	VSY		0x20

/*
 * The DUART registers with special meaning for the IPC.
 */
#ifdef ASM
#define	DUARTGET	DUART_ADDR(OPCR)
#define	DUARTSET 	DUART_ADDR(SETOB)
#define	DUARTRST 	DUART_ADDR(RSTOB)
#else
#define	DUARTGET	DUART_REG(OPCR)
#define	DUARTSET 	DUART_REG(SETOB)
#define	DUARTRST 	DUART_REG(RSTOB)
#endif
#define	STATUS		DUARTGET

#endif /* _ipc_h */
