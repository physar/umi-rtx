
DEST		= ../..

CPPFLAGS	= -I../../include/umi-drivers -I. -I../daemon -DREADLINE -D__USE_GNU -D__USE_BSD 

CFLAGS		= -g

LDFLAGS		= 

LIBS		= ../../lib/libarm.a  ../../lib/libvrtx.a ../../lib/libhost.a

LDLIBS		= $(LIBS) -L/usr/local/gnu/lib -lreadline -ltermcap

MAKEFILE	= Makefile

DEPGEN=$(CPP) -M $(CPPFLAGS)

SRCS		= automaton.c alias.c shell.c moves.c misc.c names.c param.c \
		  daemon.c arm.c rtxtop.c readline.c 

OBJS		= $(SRCS:.c=.o)

PROGRAM		= rtxsh

MAKELIBS	= false

# should be before the include, otherwise depend is the default target,
# which doesn't seem to work any more, Matthijs
all: install

.PHONY: install
install: $(PROGRAM)
	install $(PROGRAM) $(DEST)/bin

include ../Makefile.std

#.PRECIOUS: $(LIBS)

.proxy: clean
clean:
	rm -f $(PROGRAM) *.o core a.out rtxcmds.c vrtx.c

#$(PROGRAM): $(OBJS) $(LIBS)
$(PROGRAM): $(OBJS)
	$(LINK.c) -o $@ $(OBJS) $(LDLIBS)

$(LIBS): FRC
	@if [ "$(MAKELIBS)" -o ! -f $@ ]; then cd `dirname $@` ; $(MAKE);fi

FRC:

#rtxcmds.c : ../rtxlib/rtxcmds.c
#	ln -s ../rtxlib/rtxcmds.c .
#
#vrtx.c : ../daemon/vrtx.c
#	ln -s ../daemon/vrtx.c .

### KEEP THIS LINE, make depend depends on it.
