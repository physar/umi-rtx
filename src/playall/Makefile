# Makefile for Robotics course, ansi-c version
# Arnoud Visser <arnoud@science.uva.nl>,
# based on the java version of Matthijs Spaan <mtjspaan@science.uva.nl>

BINARYFILES = playall 
DATAFILES = data/board.rtx data/pieces.rtx data/umi.rtx data/endgamerook.gch data/endgamequeen.gch data/endgamepawn.gch 

ROBOTICSHOME = /opt/stud/robotics/assistance/
ROBOTICSHOME = /opt/stud/robotics
ROBOTICSHOME = /opt/prac/robotics
ROBOTICSHOME = /home/arnoud/onderwijs/ZSB/assistance/repository/robotics
SOFTWAREHOME = ${ROBOTICSHOME}/software4students
#SOFTWAREHOME = ../zsb
BINARYHOME = ${ROBOTICSHOME}/bin
DATAHOME = ${ROBOTICSHOME}/data
MACHINE = Linux
#include ../robotics/Makefile.common

CP = cp -vu

CC = gcc
CFLAGS = -O2 -g -Wall
INCLUDE = -I${ROBOTICSHOME}/include
LIBS = ${ROBOTICSHOME}/lib/$(MACHINE)/libstub.a \
 ${ROBOTICSHOME}/lib/$(MACHINE)/libumi.a \
 ${ROBOTICSHOME}/lib/$(MACHINE)/libxmlparser.a \
 ${ROBOTICSHOME}/lib/$(MACHINE)/libmatrix.a \
 ${ROBOTICSHOME}/lib/$(MACHINE)/librtxmove.a \
 ${ROBOTICSHOME}/lib/$(MACHINE)/libchess.a -lm
LIBS = ${ROBOTICSHOME}/lib/$(MACHINE)/libstub.a \
 ${ROBOTICSHOME}/robotics/lib/$(MACHINE)/libmatrix.a \
 ${ROBOTICSHOME}/robotics/lib/$(MACHINE)/libchess.a -lm
LIBS = ${ROBOTICSHOME}/lib/$(MACHINE)/libumi.a \
       ${ROBOTICSHOME}/lib/$(MACHINE)/libarm.a -lm

all: playall 

clean:
	rm -f *~ *.so *.o ${BINARYFILES}


playall: playall.o move_list.o
	${CC} -o playall playall.o move_list.o ${LIBS} 

install:
	@${CP} ${BINARYFILES} ${BINARYHOME}/

.force:

%.o: %.c
	$(CC) $(CFLAGS) $(INCLUDE) -c $<
