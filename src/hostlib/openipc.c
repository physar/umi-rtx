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
 * The module that opens and maps the IPC.
 */

#include <sys/types.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include "ipcrt.h"
#include "duartset.h"

#define VMEDEV		"/dev/vme32"
#define VMEOFFSET	0xfb000000 /* sun 4/110 */
/* #define THEOFFSET	0x15400000  others */
#define IPCDEV		"/dev/ipc0"
#define THEOFFSET	0x0

char *ipcbase = 0;

int
initipc()
{
    if (ipcbase == 0) {
	int fd = open(IPCDEV,O_RDWR);
	off_t offset = THEOFFSET;

	if (fd < 0) {
	    perror(IPCDEV);
	    fd = open(VMEDEV,O_RDWR);
	    offset = VMEOFFSET;
	    if (fd < 0) {
		perror(VMEDEV);
		return -1;
	    }
	}
	ipcbase = mmap((caddr_t) 0,0x200000,PROT_READ|PROT_WRITE,
		    MAP_SHARED,fd,offset);
	close(fd);
	if (ipcbase == (caddr_t) -1) {
	    ipcbase = 0;
	    perror("mmap");
	    return -1;
	}
    }
    /* unmap unused portions, not done */
    /* open the bus */
    duartset(BUSOPEN);
    return 0;
}
