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
if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

set( CMAKE_C_FLAGS "-mfpmath=387" )

add_subdirectory(shared)

# NOTE: this assumes that the gs_upi project is included somewhere in the cmake project hierarchy, which puts the proper GS_UPI_INCLUDE into the CACHE
include_directories( ${GS_UPI_INCLUDE} shared )


## class
add_executable( class_new class/class_new.c )
add_executable( class_copy class/class_copy.c )
add_executable( class_split class/class_split.c )
add_executable( class_alter class/class_alter.c )

target_link_libraries( class_new gs_upi m process_shared)
target_link_libraries( class_copy gs_upi m)
target_link_libraries( class_split gs_upi m)
target_link_libraries( class_alter gs_upi m)

## CSG


add_executable( csg2 csg/csg2.c csg/cbspbrush.c )
add_executable( surfmerge csg/surfmerge.c)

target_link_libraries( csg2 gs_upi m)
target_link_libraries( surfmerge gs_upi m)

## BSP

set( BSP_SRC bsp/bsp2.c csg/cbspbrush.c)
add_executable( bsp2 ${BSP_SRC} )
add_executable( stripbsp bsp/stripbsp.c )

target_link_libraries( bsp2 gs_upi m)
target_link_libraries( stripbsp gs_upi m)

## W2P2

set( W2P2_SRC w2p/w2p2.c )
add_executable( w2p2 ${W2P2_SRC} )
target_link_libraries( w2p2 gs_upi m)


## portal tools

add_executable( portal portal/portal.c )
add_executable( portal2 portal/portal2.c csg/cbspbrush.c )
add_executable( mkpc portal/mkpc.c )
add_executable( leafflood portal/leafflood.c )
add_executable( leafflood2 portal/leafflood2.c )
add_executable( gather portal/gather.c )
add_executable( gather2 portal/gather2.c )
add_executable( visleaf portal/visleaf.c )
add_executable( visleaf2 portal/visleaf2.c )
add_executable( brshmrg portal/brshmrg.c )
add_executable( brshclassify portal/brshclassify.c )

target_link_libraries( portal gs_upi m process_shared)
target_link_libraries( portal2 gs_upi m)
target_link_libraries( mkpc gs_upi m)
target_link_libraries( leafflood gs_upi m)
target_link_libraries( leafflood2 gs_upi m)
target_link_libraries( gather gs_upi m)
target_link_libraries( gather2 gs_upi m)
target_link_libraries( visleaf gs_upi m)
target_link_libraries( visleaf2 gs_upi m)
target_link_libraries( brshmrg gs_upi m)
target_link_libraries( brshclassify gs_upi m)


## fface
add_executable( fface2 fface/fface2.c )
add_executable( fface3 fface/fface3.c )

target_link_libraries( fface2 gs_upi m)
target_link_libraries( fface3 gs_upi m)


## pvs
add_executable(pvs pvs/pvs.c pvs/trivial.c pvs/complex.c pvs/visclip.c pvs/gl_client.c csg/cbspbrush.c )
target_link_libraries( pvs gs_upi m)

## pvs_through
add_executable(pvs_through pvs_through/pvs.c pvs_through/trivial.c pvs_through/complex.c pvs_through/visclip.c csg/cbspbrush.c )
target_link_libraries( pvs_through gs_upi m)

# ## light
# # add_definitions( -DNO_MPROF)
# set( LIGHT_SRC light/light.c light/trace.c light/patch.c light/lighting.c light/lightmap.c light/field.c light/sky.c light/pvs.c light/csurface.c light/hashmap.c )
# add_executable( light ${LIGHT_SRC} csg/cbspbrush.c )
# SET_TARGET_PROPERTIES(light PROPERTIES COMPILE_FLAGS "-DNO_MPROF -mpentium")


## light/shapelight

set( SHAPELIGHT_SRC light/shapelight/light.c light/shapelight/trace.c light/shapelight/patch.c light/shapelight/lighting.c light/shapelight/lightmap.c light/shapelight/csurface.c light/shapelight/hashmap.c light/shapelight/balance.c light/shapelight/worldpatch.c )
add_executable( shapelight ${SHAPELIGHT_SRC} csg/cbspbrush.c )
target_link_libraries( shapelight gs_upi m)


## light/poly
add_executable(epoly light/poly/epoly.c)
add_executable(polymerge light/poly/polymerge.c )
add_executable(matlookup light/poly/matlookup.c )

target_link_libraries( epoly gs_upi m)
target_link_libraries( polymerge gs_upi m)
target_link_libraries( matlookup gs_upi m)

## render
# set( RENDER_SRC render/r_initdata.c render/render3.c render/r_plane.c render/r_lightmap.c render/r_lplayout.c render/r_texture.c render/r_texdef.c render/r_misc.c render/r_field.c render/r_math.c render/r_frustum.c render/r_facesetup.c render/r_fstmap.c render/r_fstmapmt.c render/r_fsglva.c render/r_initgl.c render/r_fsglraster.c render/r_fsglrastermt.c render/r_fstexcrd.c render/r_hvis.c render/r_hvisclip.c render/r_volume.c render/r_model.c render/r_videoscape.c render/r_screendump.c render/r_csurface.c render/r_cssetup.c render/r_fsfx.c render/r_sprite.c render/r_sky.c render/g_blockmap.c render/g_bmlink.c render/g_bmobjects.c render/g_map.c render/g_trace.c render/g_ccmap3.c render/g_map3cache.c render/ render/a_main.c render/u_expand.c )
# add_executable( render ${RENDER_SRC} )


## mapnode
add_executable(mapnode mapnode/mapnode.c)
add_executable(mapnode2 mapnode/mapnode2.c)
add_executable(fieldnode mapnode/fieldnode.c)
add_executable(blockmap mapnode/blockmap.c)

target_link_libraries( mapnode gs_upi m)
target_link_libraries( mapnode2 gs_upi m)
target_link_libraries( fieldnode gs_upi m)
target_link_libraries( blockmap gs_upi m)


## shape
add_executable(sface2shape shape/sface2shape.c )
add_executable(cface2shape shape/cface2shape.c )
add_executable(csurf2shape shape/csurf2shape.c )

target_link_libraries( sface2shape gs_upi m)
target_link_libraries( cface2shape gs_upi m)
target_link_libraries( csurf2shape gs_upi m)


## meshtiles
add_executable(vsc2class meshtiles/vsc2class.c)
add_executable(vsc2c meshtiles/vsc2c.c)
add_executable(vertex_bsp meshtiles/vertex_bsp.c )
add_executable(tridrop_bsp meshtiles/tridrop_bsp.c )
add_executable(face_link meshtiles/face_link.c )
add_executable(meshtile1 meshtiles/meshtile1.c )


target_link_libraries( vsc2class gs_upi m)
target_link_libraries( vsc2c gs_upi m)
target_link_libraries( vertex_bsp gs_upi m)
target_link_libraries( tridrop_bsp gs_upi m)
target_link_libraries( face_link gs_upi m)
target_link_libraries( meshtile1 gs_upi m)


## tjfix
add_executable( shapetjfix tjfix/shape/shapetjfix.c )
target_link_libraries( shapetjfix gs_upi m)


set( ALL_PROGRAMS shapetjfix blockmap brshclassify brshmrg stripbsp bsp2 cface2shape class_alter class_copy class_new class_split csg2 csurf2shape surfmerge epoly face_link fface2 fface3 fieldnode gather gather2 leafflood leafflood2 mapnode mapnode2 matlookup meshtile1 mkpc polymerge portal portal2 pvs pvs_through sface2shape shapelight tridrop_bsp vertex_bsp visleaf visleaf2 vsc2c vsc2class w2p2 )
INSTALL_TARGETS( /bin  ${ALL_PROGRAMS} )

INSTALL( PROGRAMS _process3 _p3cp fixhobj.sh findfixhobj.sh DESTINATION bin )
