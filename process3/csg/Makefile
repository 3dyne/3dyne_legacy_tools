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
TDDIR = ../..
CC = gcc 
COPY = cp
RM = rm -f
CFLAGS = -Wall -pipe -g -pg
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = 

OBJ = csg.o
OBJ2 = csg2.o cbspbrush.o

all: depend csg2 surfmerge

depend: *.c
	$(CC) $(INCLUDES) -MM *.c > $@

csg: $(OBJ)
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $(OBJ) -lm ../shared/process.a $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp csg $(TDDIR)/bin/

csg2: $(OBJ2)
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $(OBJ2) -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp csg2 $(TDDIR)/bin/

surfmerge: surfmerge.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ surfmerge.c -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp surfmerge $(TDDIR)/bin/

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) surfmerge csg2

%.o: %.c
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) $(OFLAGS) -c $<

include depend
