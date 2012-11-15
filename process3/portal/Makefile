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


all: portal2 mkpc leafflood leafflood2 gather gather2 visleaf visleaf2 brshmrg brshclassify

portal: portal.c portal.h
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ portal.c -lm ../shared/process.a $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp portal $(TDDIR)/bin

portal2: portal2.c portal2.h ../csg/cbspbrush.o
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ portal2.c ../csg/cbspbrush.o -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp portal2 $(TDDIR)/bin

mkpc: mkpc.c mkpc.h
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ mkpc.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp mkpc $(TDDIR)/bin

leafflood: leafflood.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ leafflood.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp leafflood $(TDDIR)/bin

leafflood2: leafflood2.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ leafflood2.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp leafflood2 $(TDDIR)/bin



gather: gather.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ gather.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp gather $(TDDIR)/bin

gather2: gather2.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ gather2.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp gather2 $(TDDIR)/bin


visleaf: visleaf.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ visleaf.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp visleaf $(TDDIR)/bin

visleaf2: visleaf2.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ visleaf2.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp visleaf2 $(TDDIR)/bin


brshmrg: brshmrg.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ brshmrg.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp brshmrg $(TDDIR)/bin


brshclassify: brshclassify.c
	$(CC) $(INCLUDES) $(CFLAGS) $(OFLAGS) -o $@ brshclassify.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp brshclassify $(TDDIR)/bin

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) gather mkpc leafflood portal2 visleaf brshmrg brshclassify gather2 visleaf2 leafflood2
