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


all: depend class_new class_copy class_split class_alter

depend: *.c
	$(CC) $(INCLUDES) -MM *.c > $@


class_new: class_new.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ class_new.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp class_new $(TDDIR)/bin

class_copy: class_copy.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ class_copy.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp class_copy $(TDDIR)/bin

class_split: class_split.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ class_split.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp class_split $(TDDIR)/bin

class_alter: class_alter.c Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ class_alter.c $(INCLUDES) -lm  $(TDDIR)/gs_upi/gs_upi.a
	cp class_alter $(TDDIR)/bin

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) class_new
	$(RM) class_copy
	$(RM) class_split
	$(RM) class_alter

include depend
