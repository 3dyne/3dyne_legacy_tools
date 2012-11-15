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
add_definitions( -Dlinux_i386 )

# set( CMAKE_C_FLAGS "-m32" )

set( GS_UPI_SRC arr.c tga.c pal.c tdb.c cmdpars.c cdb_service.c lib_pfiles.c wire.c lib_math.c lib_poly.c lib_poly2d.c lib_error.c lib_unique.c lib_token.c lib_arche.c lib_bezier.c lib_endian.c lib_packed.c lib_hobj.c lib_mprof.c lib_container.c lib_gldbg.c lib_trimesh.c )

SET( GS_UPI_INCLUDE ${CMAKE_CURRENT_SOURCE_DIR} CACHE STRING "GS UPI include dir" FORCE)

add_library( gs_upi STATIC ${GS_UPI_SRC} ) 
