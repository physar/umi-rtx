#
# $Id$
#
#include ../Makefile.common
include ../Makefile.std

PRAC		= ../..
LIBRARY		= $(PRAC)/lib/libchess.a

PRACLIB         = $(PRAC)/lib/libpract.a

CSRC		= chessraw.c chesscmds.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= chessraw.o                 

INCLUDE+=-I../pathplan -I../inverse
INCLUDE+=-I../../include/robotics

CPPFLAGS+=$(INCLUDE) -g

all:		$(LIBRARY) pract chess

clean:
	rm -f core a.out *.o *.d

chess: chessraw.o chesscmds_main.o ../../lib/libstub.a $(PRACLIB)
	$(LINK.c) -o chess $(OBJS) chesscmds_main.o \
		 $(PRAC)/lib/libumi.a $(GLIB) \
		 $(PRAC)/lib/libinverse.a \
		 $(PRAC)/lib/libpathplan.a \
		 $(PRAC)/lib/libmatrix.a \
	         $(PRAC)/lib/libstub.a \
		 $(PRAC)/lib/libarm.a \
		 $(PRAC)/lib/libxmlparser.a \
		 -lm
	mv chess $(PRAC)/bin


$(LIBRARY): $(OBJS) chesscmds_lib.o
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS) chesscmds_lib.o
	$(RANLIB) $(LIBRARY)

pract: $(OBJS) chesscmds_lib.o
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS) chesscmds_lib.o

chesscmds_main.o: chesscmds.c
	$(CC) -DMAIN1 $(CPPFLAGS) $(CFLAGS) -c chesscmds.c -o chesscmds_main.o

chesscmds_lib.o: chesscmds.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c chesscmds.c -o chesscmds_lib.o

chessraw.o: chessraw.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c chessraw.c -o chessraw.o

