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



// field2.c

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

#include "../shared/defs.h"
#include "../csg/cbspbrush.h"
#include "../light/hashmap.h"


#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )



/*
  ====================
  misc

  ====================
*/
typedef enum
{
	ProjectionType_X = 0,
	ProjectionType_Y = 1,
	ProjectionType_Z = 2
} projectionType;


projectionType GetProjectionTypeOfPlane( cplane_t *pl )
{
	int	type;
	type = pl->type & PLANE_AXIS_MASK;

	if ( type == PLANE_X || type == PLANE_ANYX )
		return ProjectionType_X;
	else if ( type == PLANE_Y || type == PLANE_ANYY )
		return ProjectionType_Y;
	else if ( type == PLANE_Z || type == PLANE_ANYZ )
		return ProjectionType_Z;

	Error( "GetProjectionTypeOfPlane: can't get type.\n" );
	return 0;
}

void GetProjectionVecs( vec3d_t right, vec3d_t up, projectionType type )
{
	if ( type == ProjectionType_X )
	{
		Vec3dInit( right, 0, 0, 1 );
		Vec3dInit( up, 0, 1, 0 );
	}
	else if ( type == ProjectionType_Y )
	{
		Vec3dInit( right, 1, 0, 0 );
		Vec3dInit( up, 0, 0, 1 );
	}
	else if ( type == ProjectionType_Z )
	{
		Vec3dInit( right, 1, 0, 0 );
		Vec3dInit( up, 0, 1, 0 );
	}
}

void ProjectVec3d( vec2d_t out, vec3d_t in, projectionType type )
{
	if ( type == ProjectionType_X )
	{
		out[0] = in[2];
		out[1] = in[1];		
	}
	else if ( type == ProjectionType_Y )
	{
		out[0] = in[0];
		out[1] = in[2];
	}
	else if ( type == ProjectionType_Z )
	{
		out[0] = in[0];
		out[1] = in[1];
	}
	else
	{
		Error( "ProjectVec3d: unkown projection type.\n" );
	}
}


/*
  ====================
  ReadPlaneClass

  ====================
*/
hmanager_t * ReadPlaneClass( char *name )
{
	tokenstream_t	*ts;
	hobj_t		*planecls;
	hmanager_t	*hm;
	hobj_search_iterator_t	iter;
	hobj_t		*plane;
	hobj_t		*flipplane;
	cplane_t		*pl;
	int		num;
	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	//
	// create compiled planes
	//

	fprintf( stderr, "load plane class and compile ...\n" );

	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		pl = NewCPlane();

		// plane norm
		pair = FindHPair( plane, "norm" );
		if ( !pair )
			Error( "missing plane normal.\n" );
		HPairCastToVec3d_safe( pl->norm, pair );

		// plane dist
		pair = FindHPair( plane, "dist" );
		if ( !pair )
			Error( "missing plane distance.\n" );
		HPairCastToFloat_safe( &pl->dist, pair );
		
		// plane type
		pair = FindHPair( plane, "type" );
		if ( !pair )
			Error( "missing plane type.\n" );
		HPairCastToInt_safe( &pl->type, pair );

		pl->self = plane;
		SetClassExtra( plane, pl );
		
	}

	//
	// resolve clsref_flipplane
	//
	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		// plane flipplane clsref
		pair = FindHPair( plane, "flipplane" );
		if ( !pair )
			Error( "missinig clsref flipplane" );

		flipplane = HManagerSearchClassName( hm, pair->value );
		if ( !flipplane )
			Error( "can't resolve clsref flipplane.\n" );

		pl = GetClassExtra( plane );
		pl->flipplane = GetClassExtra( flipplane );
	}

	printf( " %d planes\n", num );

	return hm;
}

#define FIELD_CELL_SIZE		( 16.0 )

int		fieldpatchnum = 0;
int		fieldcellnum = 0;

void DistributePolygon( map3_t *map, polygon_t *p, cplane_t *pl )
{
	int		i;
	vec2d_t		v;
	vec2d_t		min, max;
	projectionType		type;
	vec3d_t		right, up;
	fp_t		x, y;
	
	vec3d_t		norms[4];
	fp_t		dists[4];

	polygon_t	*poly;

	type = GetProjectionTypeOfPlane( pl );
	GetProjectionVecs( right, up, type );
	
	Vec2dInitBB( min, max, 999999.9 );
	for ( i = 0; i < p->pointnum; i++ )
	{
		ProjectVec3d( v, p->p[i], type );
		Vec2dAddToBB( min, max, v );
	}

	min[0] = floor(min[0]/FIELD_CELL_SIZE)*FIELD_CELL_SIZE;
	min[1] = floor(min[1]/FIELD_CELL_SIZE)*FIELD_CELL_SIZE;
	
	for ( x = min[0]; x < max[0]; x+=FIELD_CELL_SIZE )
	{
		Vec3dFlip( norms[0], right );
		Vec3dCopy( norms[1], right );
		dists[0] = -x;
		dists[1] = x+FIELD_CELL_SIZE;

		for ( y = min[1]; y < max[1]; y+=FIELD_CELL_SIZE )
		{
			Vec3dFlip( norms[2], up );
			Vec3dCopy( norms[3], up );
			dists[2] = -y;
			dists[3] = y+FIELD_CELL_SIZE;	

			
			poly = CopyPolygon( p );
			for ( i = 0; i < 4; i++ )
			{ 
				ClipPolygonInPlace( &poly, norms[i], dists[i] );
				if ( !poly )
					break;
			}
			if ( !poly )
				continue;

			{
				int		xc, yc, zc;
				veccell_t	*c;
				vec3d_t			center;
				ivec3d_t		icenter;
				vec3d_t		shift;
				fp_t		scale;

				PolygonCenter( poly, center );

//				xc = floor(center[0]/FIELD_CELL_SIZE);
//				yc = floor(center[1]/FIELD_CELL_SIZE);
//				zc = floor(center[2]/FIELD_CELL_SIZE);

				for ( scale = -8.0; scale <= 0.0; scale+=8.0 )
				{
					Vec3dMA( shift, scale, pl->norm, center );
					
					IVec3dRint( icenter, shift );

					xc = _UnitSnap( icenter[0], FIELD_CELL_SIZE );
					yc = _UnitSnap( icenter[1], FIELD_CELL_SIZE );
					zc = _UnitSnap( icenter[2], FIELD_CELL_SIZE );				

					
					if ( ( c = Map3FindCell( map, xc, yc, zc ) ) )
					{
						Vec3dAdd( c->vec, c->vec, pl->norm );
					}
					else
					{
						c = NEW( veccell_t );
						c->x = xc;
						c->y = yc;
						c->z = zc;
						Map3InsertCell( map, c );
						Vec3dCopy( c->vec, pl->norm );
						fieldcellnum++;
					} 
				}			
			}
			fieldpatchnum++;
		}
	}
}


int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*in_plane_name;
	char		*out_field_name;

	hmanager_t	*planehm;
	hmanager_t	*brushhm;

	map3_t		*map;
	FILE		*h;

	printf( "===== field2 - build field clusters =====\n" );
	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-b" );
	in_plane_name = GetCmdOpt2( "-pl" );
	out_field_name = GetCmdOpt2( "-o" );

	if ( !in_brush_name )
	{
		in_brush_name = "_gather1_bspbrush.hobj";
		printf( " default input brush class: %s\n", in_brush_name );
	}
	else
	{
		printf( " input brush class: %s\n", in_brush_name );
	}

	if ( !in_plane_name )
	{
		in_plane_name = "_plane.hobj";
		printf( " default input plane class: %s\n", in_plane_name );
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}

	if ( !out_field_name )
	{
		out_field_name = "_fieldmap.bin";
		printf( " default output field binary: %s\n", out_field_name );
	}
	else
	{
		printf( " output field binary: %s\n", out_field_name );
	}

	printf( "loading plane class ...\n" );
	planehm = ReadPlaneClass( in_plane_name );	

	printf( "loading brush class ...\n" );
	if ( !(brushhm = NewHManagerLoadClass( in_brush_name ) ) )
		Error( "failed\n" );

	map = NewMap3Hash();

	{
		hobj_search_iterator_t	brushiter;
		hobj_search_iterator_t	surfiter;
		hobj_search_iterator_t	polyiter;
		
		hobj_t		*brush;
		hobj_t		*surface;
		hobj_t		*plane;
		hobj_t		*texdef;
		hobj_t		*poly;
		hpair_t		*pair;

		cplane_t	*pl;
	
		polygon_t	*p;
		int		i, num;		
		char		tt[256];
	
		InitClassSearchIterator( &brushiter, HManagerGetRootClass( brushhm ), "bspbrush" );
		for ( ; ( brush = SearchGetNextClass( &brushiter ) ); )
		{
			InitClassSearchIterator( &surfiter, brush, "surface" );
			for ( ; ( surface = SearchGetNextClass( &surfiter ) ); )
			{
				
				pair = FindHPair( surface, "plane" );
				if ( !pair )
					Error( "missing 'plane' in surface '%s'.\n", surface->name );
				plane = HManagerSearchClassName( planehm, pair->value );
				if ( !plane )
					Error( "surface '%s' can't find plane '%s'.\n", surface->name, pair->value );
				pl = GetClassExtra( plane );
				
				InitClassSearchIterator( &polyiter, surface, "polygon" );
				for ( ; ( poly = SearchGetNextClass( &polyiter ) ); )
				{
					pair = FindHPair( poly, "num" );
					if ( !pair )
						Error( "missing pointnum 'num' of polygon '%s'.\n", poly->name );
					HPairCastToInt_safe( &num, pair );
					
					p = NewPolygon( num );
					p->pointnum = num;
					
					for ( i = 0; i < num; i++ )
					{
						sprintf( tt, "%d", i );
						pair = FindHPair( poly, tt );
						if ( !pair )
							Error( "missing point '%s' of polygon '%s'.\n", tt, poly->name );
						HPairCastToVec3d( p->p[i], pair );
					}

					DistributePolygon( map, p, pl );					
				}
			}
		}
	}	

	Map3Normalize( map );

	printf( " %d field patches\n", fieldpatchnum );
	printf( " %d field cells\n", fieldcellnum );

	h = fopen( out_field_name, "w" );
	if ( !h )
		Error( "can't open output file.\n" );
	WriteMap3( map, h );
	fclose( h );
	
}
