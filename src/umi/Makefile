#
# $Id$
#
include ../Makefile.std
#include ../Makefile.common # included by Makefile.std

LIBRARY		= ../../lib/libumi.a

PRACLIB         = ../../lib/libpract.a

CSRC		= umiinit.c umicmds.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)
INCLUDE         += -I../../include/umi-drivers -I../../include/robotics

all:		$(LIBRARY) pract
# all:		umiinit

clean:
	rm -f core a.out $(OBJS) *.d

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

pract: $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)

umiinit: 
	$(CC) -H $(CFLAGS) $(CSRC) $(INCLUDE) -DMAIN -o ../../bin/umiinit ../../lib/libarm.a 
	
