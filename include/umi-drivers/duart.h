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
 * The 68681 DUART constants.
 */
#ifndef _duart_h
#define _duart_h

/*
 * The DUART has a 3.6864 Mhz clock. The timer divides by 16
 * The wave is twice the value of the CTUR and CTUL
 */
#define DUART_FREQ	(3686400 / 16 / 2)

/*
 * Addresses of DUART registers = DUART + REG*4
 * DUART_ADDR is the address (so we can use it with as)
 * DUART_REG is somethng you can write to/read from in c.
 */

#if !defined(__STDC__) && !defined(HAVE_VOLATILE) && !defined(volatile)
#define volatile
#endif

#define DUART_ADDR(x)	(DUART + ((x) * 4))
#define DUART_REG(x)	(*((volatile char *) (DUART_ADDR(x))))

#define	PORTA		0
#define	PORTB		1

#define	MR1(x)		((8*x))
#define	MR2(x)		((8*x))
#define SR(x)		(0x1 + (8*x))
#define	CSR(x)		(0x1 + (8*x))
#define	CR(x)		(0x2 + (8*x))
#define	RHR(x)		(0x3 + (8*x))
#define THR(x)		(0x3 + (8*x))

#define MR1A		0x0
#define MR2A		0x0
#define	SRA		0x1
#define	CSRA		0x1
#define	CRA		0x2
#define	RHRA		0x3
#define	THRA		0x3
#define	IPCR		0x4
#define	ACR		0x4
#define	ISR		0x5
#define	IMR		0x5
#define	CTU		0x6
#define	CTL		0x7
#define	MR1B		0x8
#define	MR2B		0x8
#define	SRB		0x9
#define	CSRB		0x9
#define	CRB		0xa
#define	RHRB		0xb
#define	THRB		0xb
#define	IVR		0xc
#define	IPP		0xd
#define	OPCR		0xd
#define STARTC		0xe
#define	SETOB		0xe
#define STOPC		0xf
#define	RSTOB		0xf

/*
 * DUART interrupts
 */
#define DINT_INPUT	0x80
#define	DINT_BREAKB	0x40
#define	DINT_FULLB	0x20
#define	DINT_READYB	0x10
#define	DINT_TIMER	0x08
#define	DINT_BREAKA	0x04
#define	DINT_FULLA	0x02
#define	DINT_READYA	0x01
/*
 * SR[AB] bits
 */

#define SR_RREADY	0x01
#define SR_FFULL	0x02
#define SR_TREADY	0x04
#define SR_TEMPTY	0x08

#endif /* _duart_h */
