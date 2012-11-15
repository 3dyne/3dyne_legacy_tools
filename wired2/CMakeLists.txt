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
cmake_minimum_required(VERSION 2.8)

find_package(Qt3 REQUIRED)

MESSAGE( STATUS "QT_INCLUDE_DIR: " ${QT_INCLUDE_DIR} )
MESSAGE( STATUS "QT_LIBRARIES: " ${QT_LIBRARIES} )
MESSAGE( STATUS "QT_MOC_EXECUTABLE: " ${QT_MOC_EXECUTABLE} )

INCLUDE_DIRECTORIES(
# ${CMAKE_BINARY_DIR}
${QT_INCLUDE_DIR}
${QT_INCLUDE_PATH}
${GS_UPI_INCLUDE}
)

SET(CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -Wall")

SET( WIRED_MOC_HEADERS 
    Wired.hh
    XZView.hh
    YView.hh
    CameraView.hh
    TexBrowser.hh
    MoveTool.hh
    ClipperTool.hh
    ATBrowser.hh
    BrushEdit.hh
    ATEdit.hh
)

set( WIRED_SRCS
    Wired.cc      
    XZView.cc     
    YView.cc      
    CameraView.cc 
    WWM.cc        
    TexBrowser.cc 
    MoveTool.cc   
    ClipperTool.cc
    ATBrowser.cc  
    brush.c       
    render.c      
    texture.c     
    archetype.c   
    draw_portal.cc 
    BrushEdit.cc  
    Customize.cc  
    EditAble.cc   
    TestBox.cc    
    CtrlPoint.cc  
    shock.c       
    lib_mesh.c    
    CSurface.cc   
    Draw3d.cc     
    CPoly.cc      
    ATEdit.cc     
    TexRes.cc
)

# and finally this will run moc:
QT_WRAP_CPP( xxx WIRED_MOC_SRCS ${WIRED_MOC_HEADERS} )


# we need this to be able to include headers produced by uic in our code
# (CMAKE_BINARY_DIR holds a path to the build directory, while INCLUDE_DIRECTORIES() works just like INCLUDEPATH from qmake)
INCLUDE_DIRECTORIES( ${CMAKE_BINARY_DIR} )

# here we instruct CMake to build "sample" executable from all of the source files

add_executable( wired WIN32 ${WIRED_SRCS} ${WIRED_MOC_SRCS})

target_link_libraries(wired ${QT_LIBRARIES} gs_upi  )

INSTALL_TARGETS( /bin wired )
