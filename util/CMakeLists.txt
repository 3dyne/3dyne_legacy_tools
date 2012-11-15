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
# NOTE: this assumes that the gs_upi project is included somewhere in the cmake project hierarchy, which puts the proper GS_UPI_INCLUDE into the CACHE
include_directories( ${GS_UPI_INCLUDE} )

add_executable( cdbadd cdbtools/cdbadd.c )
add_executable( cdbrm cdbtools/cdbrm.c )
add_executable( cdbget cdbtools/cdbget.c )

target_link_libraries( cdbadd gs_upi m)
target_link_libraries( cdbrm gs_upi m)
target_link_libraries( cdbget gs_upi m)


INSTALL_TARGETS( /bin  cdbadd cdbrm cdbget )
