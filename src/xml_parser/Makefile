#
# $Id$
#
#include ../Makefile.common
include ../Makefile.std  

PRAC		= ../..

SERVER		= $(PRAC)/bin/gnuchessr

LIBRARY		= $(PRAC)/lib/libxmlparser.a

PRACLIB		= 


AS		= as

AR		= ar

ARFLAGS		= crv

LD		= ld

GLIB 		= -lm 

INCLUDE		+= -I../../include/robotics

CFLAGS		= -g $(INCLUDE)

MAKEFILE	= Makefile

CSRC		= robot.c chess_set.c chess_board.c
ASRC		=
SRCS		= $(CSRC) $(ASRC)
OBJS		= $(CSRC:.c=.o) $(ASRC:.S=.o)

#all:		 create_xml_robot_info create_xml_chess_set_info create_xml_chess_board_info
all:		 $(LIBRARY)


clean:
	rm -f core a.out $(OBJS) depend $(MAKEFILE).OLD *.d

create_xml_robot_info: robot.o
	$(CC) $(CFLAGS) -DMAIN -o $(PRAC)/bin/create_xml_robot_info robot.c $(PRACLIB) $(GLIB)
	
create_xml_chess_set_info: chess_set.o
	$(LINK.c) -o $(PRAC)/bin/create_xml_chess_set_info chess_set.o $(PRACLIB) $(GLIB)
	
create_xml_chess_board_info: chess_board.o
	$(LINK.c) -o $(PRAC)/bin/create_xml_chess_board_info chess_board.o $(PRACLIB) $(GLIB)
	
$(LIBRARY): $(OBJS)
	rm -f $(LIBRARY)
	$(AR) $(ARFLAGS) $(LIBRARY) $(OBJS)
	$(RANLIB) $(LIBRARY)

pract: $(OBJS)
	@$(AR) $(ARFLAGS) $(PRACLIB) $(OBJS)

DEPGEN=  /usr/lang/cpp -M $(CPPFLAGS)

### KEEP THIS LINE, make depend depends on it.
