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



// clustermap.c
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#include "tga.h"

#include "../shared/defs.h"
#include "../csg/cbspbrush.h"
#include "../light/hashmap.h"

#define TGA_FIXED_WIDTH		( 512 )
void WriteMap3ToTGA888( map3_t *map, char *name )
{
	int		width;
	int		height;
	int		pointnum;

	int		i, point;
	veccell_t	*c;
	   
	tga_t		*tga;
	FILE		*h;
	
	pointnum = map->totalitemnum;
	width = TGA_FIXED_WIDTH;
	height = (pointnum / width)+1;

	tga = TGA_Create( width, height, TGA_TYPE_TRUECOLOR );
	
	point = 0;
	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		for ( c = map->hash[i]; c ; c=c->next )
		{
			tga->image.red[point] = (unsigned char)(c->vec[0]*255.0);
			tga->image.green[point] = (unsigned char)(c->vec[1]*255.0);
			tga->image.blue[point] = (unsigned char)(c->vec[2]*255.0);
			point++;
		}
	}

	h = fopen( name, "w" );
	TGA_Write( h, tga );
	fclose( h );
	
}

veccell_t * GetAnyFieldCell( map3_t *map )
{
	int		i;

	for ( i = 0; i < MAP3_HASHSIZE; i++ )
	{
		if ( map->hash[i] )
			return map->hash[i];
	}
	return NULL;
}

typedef struct cluster_s
{
	int		x, y, z;
//	int		cluster_size;
	struct cluster_s	*next;
	vec3d_t		vecs[8];	// variable       
} cluster_t;

cluster_t * NewCluster( int cluster_size )
{
	cluster_t	*c;
	int		size;
	int		cellnum;

	cellnum = cluster_size*cluster_size*cluster_size;
	
	size = (int)((cluster_t *)0)->vecs[cellnum];
	c = (cluster_t *) malloc( size );
	memset( c, 0, size );

//	c->cluster_size = cluster_size;
	return c;	
}

cluster_t * BuildClusterMap( map3_t *map, int cluster_size )
{
	veccell_t		*c;

	int		clusternum = 0;
	int		clustercellnum = 0;

	int		cellnum;

	cluster_t	*head, *cl;
	
	cellnum = cluster_size*cluster_size*cluster_size;

	head = NULL;

	for ( c = NULL; ( c = GetAnyFieldCell( map ) ); )
	{
		int		xc, yc, zc;
		int		x, y, z;

		cl = NewCluster( cluster_size );
		cl->next = head;
		head = cl;

		cl->x = _UnitSnap(c->x,cluster_size);
		xc = cl->x*cluster_size;
		cl->y = _UnitSnap(c->y,cluster_size);
		yc = cl->y*cluster_size;
		cl->z = _UnitSnap(c->z,cluster_size);
		zc = cl->z*cluster_size;
//		printf( "(%d %d %d)->(%d %d %d) ", c->x, c->y ,c->z, xc, yc, zc );
//		Map3RemoveCell( map, c->x, c->y, c->z );
//		continue;

		for ( x = xc; x < xc+cluster_size; x++ )
		{
			for ( y = yc; y < yc+cluster_size; y++ )
			{
				for ( z = zc; z < zc+cluster_size; z++ )	
				{
					veccell_t	*c2;
//					printf( "(%d %d %d) ", x, y ,z );
					if ( ( c2 = Map3FindCell( map, x, y, z ) ) )
					{
						Vec3dCopy( cl->vecs[(x-xc)+(y-yc)*cluster_size+(z-zc)*cluster_size*cluster_size], c2->vec );

						Map3RemoveCell( map, x, y, z );
//						printf( "." );

					}
				}
			}
		}
		clusternum++;
		clustercellnum+=cellnum;
	} 

	printf( " %d cells\n", map->totalitemnum );
	printf( " %d clusters\n", clusternum );
	printf( " %d cluster cells\n", clustercellnum );

	return head;
}

void WriteClusterMap( cluster_t *list, int cluster_size, char *name )
{
	int		i, count;
	int		cellnum;
	cluster_t	*cl;
	FILE		*h;

	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file.\n" );

	cellnum = cluster_size*cluster_size*cluster_size;

	// count clusters

	for ( count = 0, cl=list; cl ; cl=cl->next, count++ )
	{ }

	// write clusternum
	fwrite( &count, 4, 1, h );
	// write clustersize
	fwrite( &cluster_size, 4, 1, h );

	for ( cl = list; cl ; cl=cl->next )
	{
		short		s;

		s = (short)cl->x;
		fwrite( &s, 2, 1, h );
		s = (short)cl->y;
		fwrite( &s, 2, 1, h );
		s = (short)cl->z;
		fwrite( &s, 2, 1, h );

		for ( i = 0; i < cellnum; i++ )
		{
			fwrite( &cl->vecs[i][0], 4, 1, h );
			fwrite( &cl->vecs[i][1], 4, 1, h );
			fwrite( &cl->vecs[i][2], 4, 1, h );
		}
	}	
	fclose( h );
}

int main( int argc, char *argv[] )
{
	char		*in_map3_name;
	char		*out_cluster_name;

	int		cluster_size;
	map3_t		*map;

	cluster_t	*list;

	printf( "===== clustermap - build map3 cell clusters and compress them =====\n" );
	SetCmdArgs( argc, argv );

	in_map3_name = GetCmdOpt2( "-i" );
	out_cluster_name = GetCmdOpt2( "-o" );


	if ( !in_map3_name )
		Error( "no input map3 binary (-i)\n" );
	if ( !out_cluster_name )
		Error( "no output cluster binary (-o)\n" );

	if ( GetCmdOpt2( "-size" ) )
	{
		cluster_size = atoi( GetCmdOpt2( "-size" ) );
		printf( "Switch: -size %d\n", cluster_size );
	}
	else
	{
		cluster_size = 4;
		printf( " defaut cluster size: %d\n", cluster_size );
	}

	printf( "load map3 ...\n" );
	map = ReadMap3( in_map3_name );
//	Map3Dump( map );
//	WriteMap3ToTGA888( map, "_clustermap_tga.tga" );
	list = BuildClusterMap( map, cluster_size );
	WriteClusterMap( list, cluster_size, out_cluster_name );
}
