#-------------------------------------------------------------------------------
# 3dyne Legacy Tools GPL Source Code
# 
# Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
# 
# This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
# Tools Source Code").
#   
# 3dyne Legacy Tools Source Code is free software: you can redistribute it
# and/or modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation, either version 3 of the License,
# or (at your option) any later version.
# 
# 3dyne Legacy Tools Source Code is distributed in the hope that it will be
# useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
# Public License for more details.
# 
# You should have received a copy of the GNU General Public License along with
# 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
# 
# In addition, the 3dyne Legacy Tools Source Code is also subject to certain
# additional terms. You should have received a copy of these additional terms
# immediately following the terms and conditions of the GNU General Public
# License which accompanied the 3dyne Legacy Tools Source Code.
# 
# Contributors:
#     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
#     Simon Berger (simberger@gmail.com) - initial API and implementation
#-------------------------------------------------------------------------------
TDDIR = ../../..
CC = gcc 
COPY = cp
RM = rm -f
CFLAGS = -Wall -pipe -O2 -finline-functions -fomit-frame-pointer -mpentium
#CFLAGS = -Wall -pipe -pg -g
#CFLAGS = -Wall -pipe -pg -O2 -fexpensive-optimizations -ffast-math  -mpentium
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = -DNO_MPROF
LIBS = -lm
OBJ = light.o trace.o patch.o lighting.o lightmap.o csurface.o hashmap.o

all: polylight


polylight: Makefile light.h hashmap.h $(OBJ)
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $(OBJ) ../../csg/cbspbrush.o $(LIBS) $(TDDIR)/gs_upi/gs_upi.a 
	cp polylight $(TDDIR)/bin

#	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ light.c trace.c patch.c lighting.c lightmap.c field.c sky.c pvs.c csurface.c hashmap.c ../csg/cbspbrush.c -lm $(TDDIR)/gs_upi/gs_upi.a   3dnow_vec.o trace_3dnow.o -L/usr/X11/lib 
#	cp light $(TDDIR)/bin

%.o: %.c Makefile
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) -c $<

depend: *.c *.h
	makedepend $(INCLUDES) -p./ -f- *.c > depend


clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) polylight

include depend
