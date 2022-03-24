#
# $Id$
#
LIBRARY		= libarm.a

CC		= cc

AS		= as

AR		= ar

ARFLAGS		= crv

LD		= ld

DEST		= ../..

CPPFLAGS	= -I../../include/umi-drivers

CFLAGS		= -g -fPIE ${CPPFLAGS}

MAKEFILE	= Makefile

CSRC		= armcmds.c armraw.c armerror.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

all:		$(LIBRARY)

include ../Makefile.std

clean:
	rm -f core a.out $(LIBRARY) $(OBJS) depend

$(LIBRARY): $(OBJS)
	rm -f $(DEST)/lib/$(LIBRARY)
	$(AR) $(ARFLAGS) $(DEST)/lib/$(LIBRARY) $(OBJS)
	$(RANLIB) $(DEST)/lib/$(LIBRARY)

### KEEP THIS LINE, make depend depends on it.
armcmds.o: ../../include/umi-drivers/comm.h
armcmds.o: ../../include/umi-drivers/duart.h
armcmds.o: ../../include/umi-drivers/ipc.h
armcmds.o: ../../include/umi-drivers/ipcrt.h
armcmds.o: ../../include/umi-drivers/rtx.h
armerror.o: ../../include/umi-drivers/rtx.h
armraw.o: ../../include/umi-drivers/comm.h
armraw.o: ../../include/umi-drivers/rtx.h
armraw.o: ../../include/umi-drivers/rtxd.h
