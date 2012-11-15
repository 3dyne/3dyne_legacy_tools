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
#CFLAGS = -Wall -pipe -g -pg
CFLAGS = -Wall -pipe -O2 -finline-functions -fomit-frame-pointer -fexpensive-optimizations
INCLUDES = -I. -I$(TDDIR)/gs_upi/
DEFINES = 

OBJ = 	type_simple.o \
	type_plane.o \
	type_wtexture.o \
	type_bspbrush.o \
	type_bspnode.o \
	type_portal.o

all: $(OBJ) process.a

depend: *.c *.h
	$(CC) $(INCLUDES) -MM *.c > $@

clean:
	$(RM) *.o *.a
	$(RM) depend

%.o: %.c %.h
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) -c $<

process.a: $(OBJ)
	$(RM) $@
	ar -r $@ $(OBJ)
	ranlib $@
#	ln -sf gs_upi.a libgs_upi.a

include depend
