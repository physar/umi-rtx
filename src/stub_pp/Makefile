#
# $Id$
#
#include ../Makefile.common
include ../Makefile.std

LIBRARY		= ../../lib/libstub.a

PRACLIB         = ../../lib/libpract.a

CSRC		= pp.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

CFLAGS		+= -I../../include/robotics

all:		$(LIBRARY) pract

clean:
	rm -f core a.out $(OBJS) *.d

$(LIBRARY): $(OBJS)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

pract: $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)
