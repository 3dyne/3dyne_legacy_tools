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
#CFLAGS = -Wall -pipe -O2
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = 


all: depend vsc2class vertex_bsp tridrop_bsp face_link meshtile1 vsc2c

depend: *.c
	$(CC) $(INCLUDES) -MM *.c > $@


vsc2class: vsc2class.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ vsc2class.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp vsc2class $(TDDIR)/bin

vsc2c: vsc2c.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ vsc2c.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp vsc2c $(TDDIR)/bin

vertex_bsp: vertex_bsp.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ vertex_bsp.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp vertex_bsp $(TDDIR)/bin

tridrop_bsp: tridrop_bsp.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ tridrop_bsp.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp tridrop_bsp $(TDDIR)/bin

face_link: face_link.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ face_link.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp face_link $(TDDIR)/bin

meshtile1: meshtile1.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ meshtile1.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp meshtile1 $(TDDIR)/bin



clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) vsc2class
	$(RM) vsc2c
	$(RM) vertex_bsp
	$(RM) face_link
	$(RM) meshtile1
	$(RM) tridrop_bsp

include depend
