#
# Makefile for CHESS
#
# Copyright (C) 1986, 1987, 1988, 1989, 1990 Free Software Foundation, Inc.
#
# This file is part of CHESS.
#
# CHESS is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY.  No author or distributor
# accepts responsibility to anyone for the consequences of using it
# or for whether it serves any particular purpose or works at all,
# unless he says so in writing.  Refer to the CHESS General Public
# License for full details.
#
# Everyone is granted permission to copy, modify and redistribute
# CHESS, but only under the conditions described in the
# CHESS General Public License.   A copy of this license is
# supposed to have been given to you along with CHESS so you
# can know your rights and responsibilities.  It should be in a
# file named COPYING.  Among other things, the copyright notice
# and this notice must be preserved on all copies.
#
# gnuchess  will be the curses-based chess.
# gnuchessn will be the fancy-display-curses-based chess.
# gnuchessr will be the xchess/chesstool based chess.
#		(/usr/games/chesstool gnuchessr)
#    for use with SUN's chesstool and X-window's xchess.
#    To use with latter, add -DCHESSTOOL to the nondsp.o
#    compilation lines (e.g. cc -O -DCHESSTOOL ...)
# If you want to use the persistent transpositon table add
# -DHASHFILE=\"$(LIBDIR)/gnuchess.hash\"
# to the gnuchess.o compilation line. You also have to run gnuchess -t
# To actually create the file. -t can also be used to report
# statistics about the file or to resize it.
#
VERS=	3.1
DIST=	README DOCUMENTATION Makefile Makefile.dos version.h\
	gnuchess.h gnuchess.c\
	uxdsp.c nondsp.c nuxdsp.c gnuchess.book Xchess
BINDIR= /usr/local/bin
LIBDIR= /usr/local/lib
INSTALLDIR = /opt/stud/robotics/
INSTALLDIR = /opt/prac/robotics/
INSTALLDIR = ../..
LCURSES=-lcurses -ltermcap
HASH=	-DHASHFILE=\"$(LIBDIR)/gnuchess.hash\"
HASH=
BOOK=	-DBOOK=\"./data/gnuchess.book\"
CHESSTOOL= -DCHESSTOOL
CHESSTOOL=
#CC= cc
CC=	gcc -W
#CFLAGS= -O -Dinline=""		# for Aviion gcc-1.35
CFLAGS=	-O -finline-functions -fstrength-reduce

all :  install  


install: gnuchessr
	cp -pf gnuchess.book $(INSTALLDIR)/data
	mv gnuchessr $(INSTALLDIR)/bin

clean:
	rm -f *.o core a.out chess.lst chess.error_report

gnuchessr: gnuchess.o robdsp.o
	$(CC) $(CFLAGS) $(HASH) $(BOOK) $(CHESSTOOL) -o gnuchessr gnuchess.o robdsp.o


gnuchess.o: gnuchess.c gnuchess.h 
	$(CC) $(CFLAGS) $(BOOK) -c gnuchess.c

robdsp.o: robdsp.c gnuchess.h version.h
	$(CC) $(CFLAGS) $(HASH) $(CHESSTOOL) -c robdsp.c

