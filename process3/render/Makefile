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
CFLAGS = -Wall -pipe -O2 -finline-functions -fomit-frame-pointer -fexpensive-optimizations -fschedule-insns2 -ffast-math -funroll-loops
#CFLAGS = -Wall -pipe -pg -O2 -finline-functions -fexpensive-optimizations -mpentium
#OFLAGS = 
INCLUDES = -I. -I$(TDDIR)/gs_upi -I../shared -I/usr/local/include
#DEFINES = -D__NO_VISLEAF
DEFINES =
#LIBS =  -L/usr/X11/lib -lm -lGL -lMesaGLU -lglut -lX11 -lXmu -lXext -lXi  
LIBS =  -L$(TDDIR)/lib -lGL  -L/usr/X11/lib -lm -lMesaGLU -lglut -lglide2x -lX11 -lXmu -lXext -lXi  
#LIBS =  -lm -lglide2x -lX11 -lXmu -lXext -lXi -L$(TDDIR)/lib $(TDDIR)/lib/libglut.a $(TDDIR)/lib/libGLU.a $(TDDIR)/lib/libGL.a $(TDDIR)/lib/libGLX.a -L/usr/X11/lib   

OBJ = r_initdata.o render3.o r_plane.o r_lightmap.o r_lplayout.o r_texture.o r_texdef.o r_misc.o r_field.o r_math.o r_frustum.o r_facesetup.o r_fstmap.o r_fstmapmt.o r_fsglva.o r_initgl.o r_fsglraster.o r_fsglrastermt.o r_fstexcrd.o r_hvis.o r_hvisclip.o r_volume.o r_model.o r_videoscape.o r_screendump.o r_csurface.o r_cssetup.o r_fsfx.o r_sprite.o r_sky.o g_blockmap.o g_bmlink.o g_bmobjects.o g_map.o g_trace.o g_ccmap3.o g_map3cache.o  a_main.o u_expand.o

all: $(OBJ) render3

render3: $(OBJ) Makefile
	$(CC) $(CFLAGS) $(OFLAGS) -o $@ $(OBJ) ../csg/cbspbrush.o $(LIBS) $(TDDIR)/gs_upi/gs_upi.a 
	cp render3 $(TDDIR)/bin

%.o: %.c Makefile
	$(CC) $(INCLUDES) $(DEFINES) $(CFLAGS) -c $<

depend: *.c *.h
	makedepend $(INCLUDES) -p./ -f- *.c > depend

clean:
	$(RM) *.o *.a
	$(RM) depend
	$(RM) render3

include depend
