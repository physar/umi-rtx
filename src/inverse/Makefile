#
# $Id$
#
#include ../Makefile.common
#include ../Makefile.std

PRAC		= ../..

LIBRARY		= $(PRAC)/lib/libinverse.a

PRACLIB         = $(PRAC)/lib/libpract.a

AS		= as

AR		= ar

ARFLAGS		= crv

LD		= ld

CFLAGS		+= -DARNOUD -I$(PRAC)/include/robotics

MAKEFILE	= Makefile

CSRC		= inverse.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

all:		$(LIBRARY)


clean:
	rm -f core a.out $(OBJS) depend $(MAKEFILE).OLD *.d

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
#	$(RANLIB) $(LIBRARY)

pract: $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)

invinit:
	$(CC) -H $(CFLAGS) $(SRCS) $(INCLUDE) -DMAIN -DSIMULATION -lm -o ../../bin/invinit ../../lib/libarm.a ../../lib/libxmlparser.a $(PRACLIB)

DEPGEN=  /usr/lang/cpp -M $(CPPFLAGS)

depend: $(SRCS) $(MAKEFILE)
	@echo -n Making dependencies...
	@sed -n -e '/^### KEEP THIS LINE, make depend depends on it.$$/,$$!p'\
		$(MAKEFILE) > depend
	@echo '### KEEP THIS LINE, make depend depends on it.' >> depend
	@for i in $(SRCS); do $(DEPGEN) $$i | tail +2 ; done |\
		sed 's% \./% %' | grep -v ' /usr/include' | sort -u >> depend
	@rm -f $(MAKEFILE).OLD .depend
	@mv $(MAKEFILE) $(MAKEFILE).OLD
	@mv depend $(MAKEFILE)
	@echo done.

### KEEP THIS LINE, make depend depends on it.
