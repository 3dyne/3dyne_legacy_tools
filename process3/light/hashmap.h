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



// hashmap.h

#ifndef __hashmap
#define __hashmap

#include <stdio.h>
#include <stdlib.h> 

#include "lib_math.h"
#include "lib_error.h"

typedef struct veccell_s
{
	int		x, y, z;
	vec3d_t		vec;
	struct veccell_s	*next;	// hash
} veccell_t;

#define MAP3_HASHSIZE	( 4096 )
typedef struct map3_s
{
	int		totalitemnum;
	veccell_t	*hash[MAP3_HASHSIZE];
	int		itemnum[MAP3_HASHSIZE];
} map3_t;

int CalcHashkey( int x, int y, int z );
map3_t * NewMap3Hash( void );
void Map3Dump( map3_t *map );
veccell_t * Map3FindCell( map3_t *map, int x, int y, int z );
void Map3InsertCell( map3_t *map, veccell_t *c );
void Map3RemoveCell( map3_t *map, int x, int y, int z );

void Map3Chop01( map3_t *map );
void Map3Normalize( map3_t *map );

void WriteMap3( map3_t *map, FILE *h );
map3_t * ReadMap3( char *name );
#endif
