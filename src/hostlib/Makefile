MAKEFILE	= Makefile

DEST		= ../..

CPPFLAGS	= -I../../include/umi-drivers -I../../include/bsd

CFLAGS		= -g -D LINUX

LDFLAGS		= 

LIB		= libhost.a

SRCS		= alive.c openipc.c runcpu.c stopcpu.c checksum.c download.c \
		  validate.c uptime.c duartset.c duartrst.c

OBJS		= $(SRCS:.c=.o)

all: $(LIB) install

.PHONY: clean
clean:
	rm -f $(LIB) $(OBJS)


$(LIB):	$(OBJS)
	rm -f $(LIB)
	$(AR) $(ARFLAGS) $(LIB) $(OBJS)
	ranlib $(LIB)

.PHONY: install
install:
	install $(LIB) $(DEST)/lib

DEPGEN=/lib/cpp -M $(CPPFLAGS)

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
download.o: ../../include/umi-drivers/duart.h
download.o: ../../include/umi-drivers/ipc.h
download.o: ../../include/umi-drivers/ipcrt.h
duartrst.o: ../../include/umi-drivers/duart.h
duartrst.o: ../../include/umi-drivers/ipc.h
duartrst.o: ../../include/umi-drivers/ipcrt.h
duartset.o: ../../include/umi-drivers/duart.h
duartset.o: ../../include/umi-drivers/ipc.h
duartset.o: ../../include/umi-drivers/ipcrt.h
openipc.o: ../../include/umi-drivers/duart.h
openipc.o: ../../include/umi-drivers/ipc.h
openipc.o: ../../include/umi-drivers/ipcrt.h
runcpu.o: ../../include/umi-drivers/duart.h
runcpu.o: ../../include/umi-drivers/ipc.h
runcpu.o: ../../include/umi-drivers/ipcrt.h
stopcpu.o: ../../include/umi-drivers/duart.h
stopcpu.o: ../../include/umi-drivers/ipc.h
stopcpu.o: ../../include/umi-drivers/ipcrt.h
uptime.o: ../../include/umi-drivers/duart.h
uptime.o: ../../include/umi-drivers/ipc.h
uptime.o: ../../include/umi-drivers/ipcrt.h
validate.o: ../../include/umi-drivers/duart.h
validate.o: ../../include/umi-drivers/ipc.h
validate.o: ../../include/umi-drivers/ipcrt.h
