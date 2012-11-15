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
CFLAGS = -Wall -pipe -pedantic -O2 -g
LIBS = -ldl -lm
GSINC = -I$(TDDIR)/gs_upi -I$(TDDIR)/gs_gpi
GSLIBS = $(TDDIR)/gs_upi/gs_upi.a $(TDDIR)/gs_gpi/gs_gpi.a
TARGETS = gsfview tga2arr wal2arr mwal2arr mtga2arr arr2anim animview updatearr tdbadd tdbupdate tdbls tdbremove tdbcomment tdbchkserver tga888to555 tga888to565 tgatocia dump2tga tdb2res



all: depend $(TARGETS) install

depend: *.c
	$(CC) $(GSINC) -MM *.c > $@

%.o: %.c $(GSLIBS)
	$(CC) -o $@ $< $(CFLAGS) $(GSINC) $(GSLIBS) $(LIBS)

install:
	$(COPY) $(TARGETS) $(TDDIR)/bin/
	$(COPY) walview arrview $(TDDIR)/bin

clean:
	rm -f $(TARGETS)
	rm -f depend
	cd $(TDDIR)/bin; rm -f $(TARGETS)  walview arrview


include depend
