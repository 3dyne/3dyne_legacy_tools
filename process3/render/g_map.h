/* 
 * 3dyne Legacy Tools GPL Source Code
 * 
 * Copyright (C) 1996-2012 Matthias C. Berger & Simon Berger.
 * 
 * This file is part of the 3dyne Legacy Tools GPL Source Code ("3dyne Legacy
 * Tools Source Code").
 *   
 * 3dyne Legacy Tools Source Code is free software: you can redistribute it
 * and/or modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation, either version 3 of the License,
 * or (at your option) any later version.
 * 
 * 3dyne Legacy Tools Source Code is distributed in the hope that it will be
 * useful, but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General
 * Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * 3dyne Legacy Tools Source Code.  If not, see <http://www.gnu.org/licenses/>.
 * 
 * In addition, the 3dyne Legacy Tools Source Code is also subject to certain
 * additional terms. You should have received a copy of these additional terms
 * immediately following the terms and conditions of the GNU General Public
 * License which accompanied the 3dyne Legacy Tools Source Code.
 * 
 * Contributors:
 *     Matthias C. Berger (mcb77@gmx.de) - initial API and implementation
 *     Simon Berger (simberger@gmail.com) - initial API and implementation
 */ 



// g_map.h

#ifndef __g_map
#define __g_map

#include "shared.h"
#include "defs.h"
#include "g_mapdefs.h"

#define MAP_CLASS_NAME_PLANES		( "_plane.hobj" )
#define MAP_CLASS_NAME_VERTICES		( "_fface_vertex.hobj" )
#define MAP_CLASS_NAME_BRUSHES		( "_fface_bspbrush.hobj" )
#define MAP_CLASS_NAME_MAPNODES		( "_mapnode.hobj" )
#define MAP_CLASS_NAME_LIGHTDEFS	( "_light_lightdef.hobj" )
#define MAP_CLASS_NAME_TEXDEFS		( "_texdef.hobj" )
#define MAP_CLASS_NAME_TEXTURES		( "_texture.hobj" )
#define MAP_CLASS_NAME_VISLEAFS		( "_pvsout_visleaf.hobj" )

#define MAP_CLASS_NAME_CSURFACES	( "_light_csurfaces.hobj" )

#define MAP_CCMAP3_NAME_VOLUME		( "_clustermap_volume.bin" )
#define MAP_CCMAP3_NAME_FIELD		( "_clustermap_field.bin" )

g_map_t * G_NewMap( void );
g_map_t * G_LoadMap( char *path );

#endif
