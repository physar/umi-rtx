#
# $Id$
#
#include ../../Makefile.common
include ../Makefile.std

LIBRARY		= ../../lib/libmatrix.a

PRACLIB         = ../../lib/libpract.a

CSRC		= matrix.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

CFLAGS		+= -g -I../../include/robotics

all:		$(LIBRARY) pract

clean:
	rm -f core a.out $(OBJS) *.d

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

pract: $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)
