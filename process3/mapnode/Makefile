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
CFLAGS = -Wall -O2
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = 


all: mapnode mapnode2 fieldnode blockmap

mapnode: mapnode.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ mapnode.c -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp mapnode $(TDDIR)/bin

mapnode2: mapnode2.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ mapnode2.c -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp mapnode2 $(TDDIR)/bin

fieldnode: fieldnode.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ fieldnode.c -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp fieldnode $(TDDIR)/bin

blockmap: blockmap.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ blockmap.c -lm  $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp blockmap $(TDDIR)/bin

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) mapnode mapnode2 fieldnode blockmap

%.o: %.c
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) $(OFLAGS) -c $<

