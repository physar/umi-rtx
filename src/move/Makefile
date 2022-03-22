#include ../Makefile.common # included in Makefile.std
include ../Makefile.std

PRACLIB         = ../../lib/libpract.a

INCLUDE += -I../../include/robotics

CFLAGS += -DFREE_LISTS

MYOBJ	=	move.o   

SRCS    =       $(MYOBJ:.o=.c)

MYLIB   =       ../../lib/librtxmove.a

all: $(MYLIB)

$(MYLIB):	$(MYOBJ)
	ar crv $(MYLIB) $(MYOBJ)
	$(RANLIB) $(MYLIB)

clean:
	rm -f core a.out $(MYOBJ) *.d

pract:
	@ar crv $(PRACLIB) $(MYOBJ)

rtxinit:
	$(CC) -H $(CFLAGS) $(SRCS) $(INCLUDE) -DMAIN  -o ../../bin/rtxinit ../../lib/libarm.a $(PRACLIB)  ../../lib/libinverse.a ../../lib/libpathplan.a ../../lib/libxmlparser.a ../../lib/libmatrix.a ../../lib/libstub.a -lm
