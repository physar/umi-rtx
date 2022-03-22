#
# $Id$
#
#include ../Makefile.common
include ../Makefile.std

PRAC            = ../..

LIBRARY		= $(PRAC)/lib/libpathplan.a

PRACLIB         = $(PRAC)/lib/libpract.a

CFLAGS		+= -g -DARNOUD -I$(PRAC)/include/robotics

CSRC		= init_pathplan.c pathplan.c Path_plan.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

all:		$(LIBRARY) $(PRACLIB)

clean:
	rm -f core a.out $(OBJS) *.d

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

$(PRACLIB): $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)

pathinit: main.o
	$(CC) -H $(CFLAGS) main.c $(INCLUDE) -DMAIN -DSIMULATION -lm -o ../../bin/pathinit ../../lib/libarm.a ../../lib/libxmlparser.a $(PRACLIB)

