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
 * The indirection layer for the robot driver mechanisms.
 *
 * Each layer defines the following functions:
 *
 *	- rtx_probe 	- probe for the type and initialize the library.
 *	- rtx_stop	- stop the library
 *	- rtx_command	- execute one command
 *	- rtx_test	- test the status of the library.
 */

#include <stdlib.h>
#include <stdio.h>
#include "rtxd.h"
#include "ipcio.h"
#include "ttyio.h"
#include "vrtx.h"

extern rtx_ops_t ipc_io_ops, tty_io_ops;

rtx_ops_t *rtx_ops[] = {
    &ipc_io_ops,
    &tty_io_ops,
};

int nrtxops = sizeof(rtx_ops)/sizeof(rtx_ops_t *);

/*
 * rtx_init function, calls all the rtx_ops one by one
 */
rtx_t
*rtx_probe(s)
char *s;
{
    int i;
    void *data;
    rtx_t *res;

    if (s == 0)
	s = getenv("RTX_SERVER");

    if (s == 0)
    {
	rtx_log((void *) 0,"No RTX_SERVER specified\n");
	return 0;
    }

    for (i = 0; i < nrtxops; i++)
	if ((data = (*rtx_ops[i]->probe)(s)))
	    break;

    if (data == 0)
    {
	rtx_log((void *) 0,"No rtx_ops found for '%s'\n", s);
	return 0;
    }

    res = (rtx_t*) malloc(sizeof(rtx_t));
    if (res != 0) {
	res->ops = rtx_ops[i];
	res->data = data;
    }
    return res;
}

int
rtx_finish(r)
rtx_t *r;
{
    int res = (*r->ops->finish)(r->data);

    free((char*) r);

    return res;
}
