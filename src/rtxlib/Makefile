#
# $Id$
#
LIBRARY		= librtx.a

AS		= as

AR		= ar

ARFLAGS		= crv

#TARGET_ARCH = -sun3

#TARGET_MACH = -mc68020

CC		= gcc

DEST		= .

CPPFLAGS	= -I../../include/umi-drivers -I../daemon -DIPC

CFLAGS		= -g

MAKEFILE	= Makefile

CSRC		= raw.c rtxcmds.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

all:		$(LIBRARY)

clean:
	rm -f core a.out $(LIBRARY) $(OBJS) depend

$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(DEST)/$(LIBRARY) $(OBJS)
	ranlib $(DEST)/$(LIBRARY)

DEPGEN=/lib/cpp -M $(CPPFLAGS)

depend: $(SRCS) $(MAKEFILE)
	@echo -n Making dependencies...
	@sed -n -e '/^### KEEP THIS LINE, make depend depends on it.$$/,$$!p'\
		$(MAKEFILE) > depend
	@echo '### KEEP THIS LINE, make depend depends on it.' >> depend
	@for i in $(SRCS); do $(DEPGEN) $$i | tail +2 ; done |\
		sed 's% \./% %' >> depend
	@rm -f $(MAKEFILE).OLD .depend
	@mv $(MAKEFILE) $(MAKEFILE).OLD
	@mv depend $(MAKEFILE)
	@echo done.

### KEEP THIS LINE, make depend depends on it.
raw.o: ../../include/umi-drivers/rtxcmds.h
raw.o: ../../include/umi-drivers/rtx.h
raw.o: ../../include/umi-drivers/ipcrt.h
raw.o: ../../include/umi-drivers/ipc.h
raw.o: ../../include/umi-drivers/duart.h
rtxcmds.o: ../../include/umi-drivers/rtx.h
rtxcmds.o: ../../include/umi-drivers/rtxcmds.h
rtxcmds.o: /usr/include/strings.h
