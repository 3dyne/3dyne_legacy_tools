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



// hashmap.c
#include <string.h>
#include "hashmap.h"

#ifndef NEW
#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )
#endif

int CalcHashkey( int x, int y, int z )
{
	int	key;
	key = x + (y<<5) + (z<<10);
	key &= (MAP3_HASHSIZE-1);
	return key;
}

map3_t * NewMap3Hash( void )
{
	map3_t	*map3;
	map3 = NEW( map3_t );

	return map3;
}

void Map3Dump( map3_t *map )
{
	int		i;
	for ( i = 0; i < MAP3_HASHSIZE; i++ )
		printf( "%d ", map->itemnum[i] );
	printf( "\n" );
}

veccell_t * Map3FindCell( map3_t *map, int x, int y, int z )
{
	int		key;
	int		i;
	veccell_t	*c;

	key = CalcHashkey( x, y, z );

	for ( c = map->hash[key]; c ; c=c->next )
	{
		if ( c->x == x && c->y == y && c->z == z )
			return c;
	}
	return NULL;
}

void Map3InsertCell( map3_t *map, veccell_t *c )
{
	int		key;
	
	key = CalcHashkey( c->x, c->y, c->z );
	c->next = map->hash[key];
	map->hash[key] = c;
	map->itemnum[key]++;
	map->totalitemnum++;
}

void Map3RemoveCell( map3_t *map, int x, int y, int z )
{
	int		key;
	int		i;
	veccell_t	*c;

	key = CalcHashkey( x, y, z );

	for ( c = map->hash[key]; c ; c=c->next )
	{
		if ( c->x == x && c->y == y && c->z == z )
		{
			veccell_t	*c2, *cnext, *head;
			head = NULL;
			for ( c2 = map->hash[key]; c2 ; c2=cnext )
			{
				cnext = c2->next;
				if ( c2 == c )
				{
					continue;
				}
				c2->next = head;
				head = c2;
			}
			map->hash[key] = head;	
			return;
		}
	}
//	return NULL;
	Error( "Map3RemoveCell: no cell to remove\n" );
}


void Map3Chop01( map3_t *map )
{
	int		i;
	veccell_t	*c;

	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		for ( c = map->hash[i]; c ; c=c->next )
		{
			int		j;
			
//			Vec3dPrint( c->vec );

			for( j = 0; j < 3; j++ )
			{
				if ( c->vec[j] < 0.0 )
					c->vec[j] = 0.0;
				if ( c->vec[j] > 1.0 )
					c->vec[j] = 1.0;
			}
		}
	}
}

void Map3Normalize( map3_t *map )
{
	int		i;
	veccell_t	*c;

	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		for ( c = map->hash[i]; c ; c=c->next )
		{
		
			Vec3dUnify( c->vec );
		}
	}
}

void WriteMap3( map3_t *map, FILE *h )
{
	int		i;
	veccell_t	*c;

	fwrite( &map->totalitemnum, 4, 1, h );

	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		for ( c = map->hash[i]; c ; c=c->next )
		{
			short		s;
			
			s = (short)c->x;
			fwrite( &s, 2, 1, h );
			s = (short)c->y;
			fwrite( &s, 2, 1, h );
			s = (short)c->z;
			fwrite( &s, 2, 1, h );
			
			fwrite( &c->vec[0], 4, 1, h );
			fwrite( &c->vec[1], 4, 1, h );
			fwrite( &c->vec[2], 4, 1, h );
		}
	}	
}

map3_t * ReadMap3( char *name )
{
	int		cellnum;
	int		i;
	map3_t		*map;
	FILE		*h;

	h = fopen( name, "r" );
	if ( !h )
		Error( "can't open map3 binary\n" );

	map = NewMap3Hash();

	fread( &cellnum, 4, 1, h );
	for ( i = 0; i < cellnum; i++ )
	{
		short		s;
		veccell_t	*c;
		
		c = NEW( veccell_t );
		fread( &s, 2, 1, h );
		c->x = (int)s;
		fread( &s, 2, 1, h );
		c->y = (int)s;
		fread( &s, 2, 1, h );
		c->z = (int)s;

		fread( &c->vec[0], 4, 1, h );
		fread( &c->vec[1], 4, 1, h );
		fread( &c->vec[2], 4, 1, h );

		Map3InsertCell( map, c );
	}	

	fclose( h );

	return map;
}
