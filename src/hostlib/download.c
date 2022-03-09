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
 * Download a sun3 executable file to the IPC
 *
 * Note: this program won't run under Solaris because Solaris nlist
 * doesn't work on a.out format, until you find your own nlist.
 *
 * Yet, under Ubuntu /usr/include/nlist.h is installed with libelf-dev
 */
#include <sys/types.h>
#include <sys/file.h>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <errno.h>
#include <nlist.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include "ipcrt.h"
#include "openipc.h" 
#include "checksum.h"
#define sun
#include "m68kipc/exec.h"
#include "m68kipc/a.out.h"

static int
setresetvector(stack,pc)
int stack;
int pc;
{
    if (ipcbase == 0 && initipc() != 0)
	return -1;

    ((int *) ipcbase)[0] = stack;
    ((int *) ipcbase)[1] = pc;
    return 0;
}

/*
 * required symbols
 */
struct nlist nl[] = {
#define NL_START	0
	{ "start" },		/* Start of text segment (from crt0) */
#define	NL_ETEXT	1
	{ "_etext" },		/* End of text segment (from ld) */
#define	NL_SDATA	2
	{ "startdata" },	/* Start of data segment  (from crt0)*/
#define	NL_EDATA	3
	{ "_edata" },		/* End of initialised data segment (from ld) */
#define	NL_END		4
	{ "_end" },		/* End of data segment (from ld) */
	{ NULL },
};

int
download(file)
char *file;
{
    int fd;
    struct exec exec;
    char *addrp, *start;
    int addrlen,offset;

    if (ipcbase == 0 && initipc() != 0)
	return -1;
    
    nlist(file,nl);
    if (nl[NL_START].n_type == 0 || nl[NL_SDATA].n_type == 0 ||
	nl[NL_ETEXT].n_type == 0 || nl[NL_EDATA].n_type == 0 ||
	nl[NL_END].n_type == 0) {
	fprintf(stderr,"%s: no namelist or missing entries\n",file);
	return -1;
    }
    /*
     * sanity checks
     */
    if (!INRAM(nl[NL_START].n_value)) {
	fprintf(stderr,"Text segment not in SRAM or DRAM\n");
	return -1;
    }
    if (!INRAM(nl[NL_ETEXT].n_value)) {
	fprintf(stderr,"Text segment not in SRAM or DRAM\n");
	return -1;
    }
    if (!INRAM(nl[NL_SDATA].n_value)) {
	fprintf(stderr,"Data segment not in SRAM or DRAM\n");
	return -1;
    }
    if (!INRAM(nl[NL_EDATA].n_value)) {
	fprintf(stderr,"Data segment not in SRAM or DRAM\n");
	return -1;
    }
    if (!INRAM(nl[NL_END].n_value)) {
	fprintf(stderr,"Data segment not in SRAM or DRAM\n");
	return -1;
    }
    if (nl[NL_ETEXT].n_value > nl[NL_SDATA].n_value) {
	fprintf(stderr,"Text and data segment overlap\n");
	return -1;
    }

    fd = open(file,O_RDONLY);
    if (fd < 0) {
	perror(file);
	return -1;
    }

    if (read(fd,(char *) &exec,sizeof exec) != sizeof exec) {
	close(fd);
	fprintf(stderr,"Can't read exec header of %s\n",file);
	return -1;
    }
    /*
     *
     */
    if (N_BADMAG(exec)) {
	fprintf(stderr,"%s has bad magic number\n",file);
	close(fd);
	return -1;
    }

    if (!INRAM(exec.a_entry)) {
	fprintf(stderr,"Invalid entry point 0x%08lx\n",exec.a_entry);
	fprintf(stderr,"Use ld -T %x\n",(unsigned int)IPCOFFSET(TEXTSTART));
	return -1;
    }
    addrp = PROGNAME;
    strncpy(addrp,file,PROGNAMEMAX);
    addrp[PROGNAMEMAX-1] = '\0';
    offset = N_TXTOFF(exec);
    /* Ld screws up */
#if 1 /* #ifndef LINUX */
    if (exec.a_magic == ZMAGIC)
	offset = sizeof(struct exec);
#endif
    start = addrp = nl[NL_START].n_value+ipcbase - offset;

    addrlen = exec.a_text + N_TXTOFF(exec);
    if (lseek(fd,0,0) == -1) {
	fprintf(stderr,"%s: lseek failed\n", file);
	close(fd);
	return -1;
    }
    while (addrlen) {
	int got;

	got = read(fd,addrp,addrlen);
	if (got <= 0) {
	    perror(file);
	    close(fd);
	    return -1;
	}
	addrlen -= got;
	addrp += got;
    }

    *(int *) FILESTART = IPCOFFSET(start);
    *(int *) TEXTSUM = checksum(start,exec.a_text+N_TXTOFF(exec));

    start = addrp = nl[NL_SDATA].n_value+ipcbase;
    addrlen = exec.a_data;
    while (addrlen) {
	int got;

	got = read(fd,addrp,addrlen);
	if (got <= 0) {
	    perror(file);
	    close(fd);
	    return -1;
	}
	addrlen -= got;
	addrp += got;
    }
    *(int *) DATASUM = checksum(start,exec.a_data);

    (void) setresetvector((int)IPCOFFSET(STACK),(int)exec.a_entry);
    close(fd);
    return 0;
}
