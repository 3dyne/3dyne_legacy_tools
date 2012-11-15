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
CFLAGS = -Wall -pipe -O2 -finline-functions -fomit-frame-pointer -fexpensive-optimizations -mpentium
#CFLAGS = -Wall -pipe -g -pg
OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared
DEFINES = 


all: pvs pvs_mpi_master pvs_mpi_slave


pvs: Makefile pvs.h pvs.c trivial.c complex.c visclip.c ../csg/cbspbrush.c
	$(CC) $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ pvs.c trivial.c complex.c visclip.c gl_client.c ../csg/cbspbrush.c -lm $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib 
	cp pvs $(TDDIR)/bin

pvs_mpi_master: Makefile pvs_mpi_master.c pvs_mpi.h
	hcc $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ pvs_mpi_master.c ../csg/cbspbrush.c -lm -lmpi $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp pvs_mpi_master $(TDDIR)/lam/bin/pvs_mpi

pvs_mpi_slave: Makefile pvs_mpi_slave.c pvs_mpi.h pvs_mpi_complex.c visclip.c
	hcc $(CFLAGS) $(OFLAGS) $(INCLUDES) -o $@ pvs_mpi_slave.c pvs_mpi_complex.c visclip.c gl_client.c ../csg/cbspbrush.c -lm -lmpi $(TDDIR)/gs_upi/gs_upi.a  -L/usr/X11/lib
	cp pvs_mpi_slave $(TDDIR)/lam/bin/pvs_mpi 

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) pvs pvs_mpi_master pvs_mpi_slave

