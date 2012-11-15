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



// g_map.c

#include "g_map.h"


/*
  ==============================
  G_NewMap

  ==============================
*/

g_map_t * G_NewMap( void )
{
	g_map_t		*map;
	
	map = NEWTYPE( g_map_t );
}


/*
  ==================================================
  G_MapTryClassLoad

  ==================================================
*/
hmanager_t * G_MapTryClassLoad( char *path, char *name )
{
	hmanager_t	*hm;
	char		tt[256];

	sprintf( tt, "%s/%s", path, name );
	hm = NewHManagerLoadClass( tt );
	if ( !hm )
		return NULL;
	return hm;
}


/*
  ==================================================
  G_MapTryCCMap3Load

  ==================================================
*/
ccmap3_t * G_MapTryCCMap3Load( char *path, char *name )
{
	ccmap3_t	*map;
	char		tt[256];

	sprintf( tt, "%s/%s", path, name );
	map = G_LoadCCMap3Binary( tt );
	if ( !map )
		return NULL;
	return map;
}

/*
  ==============================
  G_LoadMap

  ==============================
*/

g_map_t * G_LoadMap( char *path )
{
	g_map_t		*map;

	printf( "G_LoadMap: load classes ...\n" );

	map = G_NewMap();

	//
	// load classes
	//
	
	LOGMSG( " planes ...\n" );
	map->planehm = G_MapTryClassLoad( path, MAP_CLASS_NAME_PLANES );

	LOGMSG( " vertices ...\n" );
	map->vertexhm = G_MapTryClassLoad( path, MAP_CLASS_NAME_VERTICES );

	LOGMSG( " brushes ...\n" );
	map->brushhm = G_MapTryClassLoad( path, MAP_CLASS_NAME_BRUSHES );

	LOGMSG( " mapnodes ...\n" );
	map->mapnodehm = G_MapTryClassLoad( path, MAP_CLASS_NAME_MAPNODES );

	LOGMSG( " lightdefs ...\n" );
	map->lightdefhm = G_MapTryClassLoad( path, MAP_CLASS_NAME_LIGHTDEFS );

	LOGMSG( " texdefs ...\n" );
	map->texdefhm = G_MapTryClassLoad( path, MAP_CLASS_NAME_TEXDEFS );

	LOGMSG( " textures ...\n" );
	map->texturehm = G_MapTryClassLoad( path, MAP_CLASS_NAME_TEXTURES );

	LOGMSG( " visleafs ...\n" );
	map->visleafhm = G_MapTryClassLoad( path, MAP_CLASS_NAME_VISLEAFS );

	LOGMSG( " curved surfaces ...\n" );
	map->csurfacehm = G_MapTryClassLoad( path, MAP_CLASS_NAME_CSURFACES );

	if ( !map->planehm )
		Error( "plane class load failed.\n" );
	if ( !map->brushhm )
		Error( "brush class load failed.\n" );
	if ( !map->vertexhm ) 
		Error( "vertex class load failed.\n" );
	if ( !map->mapnodehm )
		Error( "mapnode class load failed.\n" );
	if ( !map->lightdefhm )
		Error( "lightdef class load failed.\n" );
	if ( !map->texdefhm )
		Error( "texdef class load failed.\n" );
	if ( !map->texturehm )
		Error( "texture class loaf failed.\n" );
#ifndef __NO_VISLEAF
	if ( !map->visleafhm )
		Error( "visleaf class load failed.\n" );
#endif
	if ( !map->csurfacehm )
		Error( "csurface class load failed.\n" );

	printf( "G_LoadMap: load binaries ...\n" );
	LOGMSG( " volume map ...\n" );
	map->volume_ccmap3 = G_MapTryCCMap3Load( path, MAP_CCMAP3_NAME_VOLUME );
	LOGMSG( " field map ...\n" );
	map->field_ccmap3 = G_MapTryCCMap3Load( path, MAP_CCMAP3_NAME_FIELD );
	
	if ( !map->volume_ccmap3 )
		printf( "warning: volume binary load failed.\n" );
	if ( !map->field_ccmap3 )
		printf( "warning: field binary load failed.\n" );
	
	return map;
}

/*
  ==============================
  G_RejectMap

  ==============================
*/

void G_RejectMap( g_map_t *map )
{
	printf( "G_RejectMap: freeing classes ...\n" );

	if ( map->planehm )
	{
		HManagerDeepDestroyClass( map->planehm, HManagerGetRootClass( map->planehm ) );
		map->planehm = NULL;
	}

	if ( map->brushhm )
	{
		HManagerDeepDestroyClass( map->brushhm, HManagerGetRootClass( map->brushhm ) );
		map->brushhm = NULL;
	}

	if ( map->vertexhm )
	{
		HManagerDeepDestroyClass( map->vertexhm, HManagerGetRootClass( map->vertexhm ) );	
		map->vertexhm = NULL;
	}

	if ( map->mapnodehm )
	{
		HManagerDeepDestroyClass( map->mapnodehm, HManagerGetRootClass( map->mapnodehm ) );
		map->mapnodehm = NULL;
	}

	if ( map->lightdefhm )
	{
		HManagerDeepDestroyClass( map->lightdefhm, HManagerGetRootClass( map->lightdefhm ) );
		map->lightdefhm = NULL;
	}

	if ( map->texdefhm )
	{
		HManagerDeepDestroyClass( map->texdefhm, HManagerGetRootClass( map->texdefhm ) );
		map->texdefhm = NULL;
	}

	if ( map->texturehm )
	{
		HManagerDeepDestroyClass( map->texturehm, HManagerGetRootClass( map->texturehm ) );
		map->texturehm = NULL;
	}

	if ( map->visleafhm )
	{
		HManagerDeepDestroyClass( map->visleafhm, HManagerGetRootClass( map->visleafhm ) );
		map->visleafhm = NULL;
	}
}


/*
  ==============================
  G_InitData

  ==============================
*/
void G_InitData( g_map_t *map )
{
	blockmap_t	*blockmap;

	G_InitBlockmap( map );
}
