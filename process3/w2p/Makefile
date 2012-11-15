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
CFLAGS = -Wall -pipe -O2
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = 

OBJ2 = w2p2.o

all: depend w2p2 pump

depend: *.c
	$(CC) $(INCLUDES) -MM *.c > $@


w2p2: $(OBJ2)
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $(OBJ2) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp w2p2 $(TDDIR)/bin

pump: pump.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ pump.c -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp pump $(TDDIR)/bin

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) w2p2
	$(RM) pump

%.o: %.c
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) $(OFLAGS) -c $<

include depend
