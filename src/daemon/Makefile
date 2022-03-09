#
# $Id: makefile,v 1.6 2004/02/16 18:09:47 arnoud Exp $
#

ifeq ($(shell uname -s),Linux)
DEPGEN		= cpp -M $(CPPFLAGS)
DEPGEN		= cpp -M $(CPPFLAGS)
LDLIBS		= $(LIBS) -ldl -lm
LDLIBS		= $(LIBS) -ldl -lelf -lm
else
CFLAGS+=-DNOLOCKING
DEPGEN		= /lib/cpp -M $(CPPFLAGS)
DEPGEN		= /usr/ccs/lib/cpp -M $(CPPFLAGS)
LDLIBS		= $(LIBS) -ldl -lsocket -lnsl -lelf -lposix4 -lm
LDLIBS		= $(LIBS) -ldl -lsocket -lnsl -lposix4 -lm
endif

CC = gcc -ggdb  -Wall 

INCLUDE= -I . -I ../../include/umi-drivers -I ../../include/bsd

DEST		= ../..

CFLAGS		= -DRTXD -DNOLOCKING -DDEBUG -Wno-unused-but-set-variable -g

LDFLAGS		=  

LIBS		= ../../lib/libhost.a ../../lib/librtx.a

MAKEFILE	= makefile

SRCS		= rtxd.c vrtx.c ipcio.c ttyio.c 

OBJS		= $(SRCS:.c=.o)

PROGRAM		= rtxd

LIB		= libvrtx.a

MAKELIBS	= true

CHOWN		= sudo chgrp dialout

MODE		= 2755

all: install

include ../Makefile.std

.PHONY: install
install: $(PROGRAM) $(LIB)
	/usr/bin/getent group dailout 2>&1 > /dev/null || sudo /usr/sbin/groupadd dailout
	install $(PROGRAM) $(DEST)/bin
	install $(LIB) $(DEST)/lib
	chmod -x $(DEST)/lib/$(LIB)
	$(CHOWN) $(DEST)/bin/$(PROGRAM)
	sudo usermod -a -G dailout $(USERNAME)

.PHONY: clean
clean:
	rm -f $(PROGRAM) $(LIB) *.o core a.out rtxcmds.c

$(PROGRAM): $(OBJS) $(LIBS)
	@rm -f $(PROGRAM)
	$(LINK.c) $(LDFLAGS) -o $@ $(OBJS) $(LDLIBS)
	@-$(CHOWN) $@ && chmod $(MODE) $@

$(LIB): vrtx.o ipcio.o ttyio.o 
	rm -f $(LIB)
	$(AR) $(ARFLAGS) $(LIB) $(OBJS)
	ranlib $(LIB)

$(LIBS): 
	@if [ "$(MAKELIBS)" -o ! -f $@ ]; then cd `dirname $@`/.. ; $(MAKE);fi

rtxcmds.c : ../rtxlib/rtxcmds.c
	ln -s ../rtxlib/rtxcmds.c .

%.o: %.c
	$(CC) -c $(INCLUDE) $(CFLAGS) $<

### KEEP THIS LINE, make depend depends on it.
ipcio.o: ../../include/umi-drivers/comm.h
ipcio.o: ../../include/umi-drivers/duart.h
ipcio.o: ../../include/umi-drivers/ipc.h
ipcio.o: ../../include/umi-drivers/ipcrt.h
ipcio.o: ../../include/umi-drivers/rtx.h
ipcio.o: ../../include/umi-drivers/rtxd.h
ipcio.o: vrtx.h
rtxcmds.o: ../../include/umi-drivers/rtx.h
rtxcmds.o: ../../include/umi-drivers/rtxcmds.h
rtxd.o: ../../include/umi-drivers/comm.h
rtxd.o: ../../include/umi-drivers/rtx.h
rtxd.o: ../../include/umi-drivers/rtxd.h
rtxd.o: vrtx.h
ttyio.o: ../../include/umi-drivers/comm.h
ttyio.o: ../../include/umi-drivers/rtx.h
ttyio.o: ../../include/umi-drivers/rtxcmds.h
ttyio.o: vrtx.h
vrtx.o: vrtx.h
