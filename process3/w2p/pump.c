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



// pump.c

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

#include "defs.h"

#define NEW( x )        ( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

typedef struct face_s
{
	hobj_t		*self;
	fp_t		dist;
	vec3d_t		norm;

	polygon_t	*p;

	struct face_s	*next;
} face_t;

typedef struct brush_s
{
	hobj_t		*self;
	unsigned int		contents;
	struct face_s	*faces;
	struct brush_s	*next;
} brush_t;

void BuildBrushPolygons( brush_t *list )
{
	brush_t		*b;
	face_t		*f, *f2;

	for ( b = list; b ; b=b->next )
	{
		// build brush polygons
		for ( f = b->faces; f ; f=f->next )
		{
			f->p = BasePolygonForPlane( f->norm, f->dist );
		}
		// clip brush polygons
		for ( f = b->faces; f ; f=f->next )
		{
			for ( f2 = b->faces; f2 ; f2=f2->next )
			{
				if ( f == f2 )
					continue;
				if ( f->p )
					ClipPolygonInPlace( &f->p, f2->norm, f2->dist );
			}
		}		
	}
}

brush_t * BoundBrushList( brush_t *inlist, unsigned int bound_brush_contents )
{
	int		i;
	brush_t		*b;
	face_t		*f, *f2;
	brush_t		*b2;
	brush_t		*outlist;

	vec3d_t		min, max;

	outlist = NULL;
	for ( b = inlist; b ; b=b->next )
	{
		// build brush polygons
		for ( f = b->faces; f ; f=f->next )
		{
			f->p = BasePolygonForPlane( f->norm, f->dist );
		}
		// clip brush polygons
		for ( f = b->faces; f ; f=f->next )
		{
			for ( f2 = b->faces; f2 ; f2=f2->next )
			{
				if ( f == f2 )
					continue;
				if ( f->p )
					ClipPolygonInPlace( &f->p, f2->norm, f2->dist );
			}
		}
		// brush bound box
		Vec3dInitBB( min, max, 999999.9 );
		for ( f = b->faces; f ; f=f->next )
		{
			for ( i = 0; i < f->p->pointnum; i++ )
			{
				Vec3dAddToBB( min, max, f->p->p[i] );
			}
		}
		// build brush from bound box
		b2 = NEW( brush_t );
		b2->contents = bound_brush_contents;
		b2->self = b->self;
		b2->next = outlist;
		outlist = b2;
		for ( i = 0; i < 3; i++ )
		{
			f = NEW( face_t );
			f->next = b2->faces;
			b2->faces = f;
			Vec3dInit( f->norm, 0, 0, 0 );
			f->norm[i] = 1.0;
			f->dist = max[i];

			f = NEW( face_t );
			f->next = b2->faces;
			b2->faces = f;
			Vec3dInit( f->norm, 0, 0, 0 );
			f->norm[i] = -1.0;
			f->dist = -min[i];
		}
	}
	return outlist;
}

brush_t * PumpBrushList( brush_t *inlist, fp_t pump_brush_dist, unsigned int pump_brush_contents )
{
	brush_t		*b, *list, *b2;
	face_t		*f, *f2;
	
	list = NULL;
	for ( b = inlist; b ; b=b->next )
	{
		b2 = NEW( brush_t );
		b2->contents = pump_brush_contents;
		b2->self = b->self;
		b2->next = list;
		list = b2;

		for ( f = b->faces; f ; f=f->next )
		{
			vec3d_t		p;

			f2 = NEW( face_t );
			f2->self = f->self;
			f2->next = b2->faces;
			b2->faces = f2;

			Vec3dScale( p, f->dist, f->norm );
			Vec3dMA( p, pump_brush_dist, f->norm, p );
			f2->dist = Vec3dDotProduct( f->norm, p );
			Vec3dCopy( f2->norm, f->norm );
		}
	}

	return list;
}

#define	MAX_POINTS		( 128 )
#define MAX_FACES_PER_POINT	( 16 )
#define MAX_EDGES		( 128 )
#define MAX_FACES_PER_EDEG	( 2 )

bool_t	CheckSame( vec3d_t v1, vec3d_t v2 )
{
	int		i;

	for ( i = 0; i < 3; i++ )
	{
		if ( fabs( v1[i]-v2[i] ) > 0.5 )
			return false;
	}
	return true;
}

void BevelBrush( brush_t *b )
{
	int			pointnum;
	vec3d_t			points[MAX_POINTS];

	int			edgenum;
	vec3d_t			edges[MAX_EDGES][2];
//	face_t			*edges[MAX_EDGES][MAX_FACES_PER_EDGE];

	int			i, j, k;
	face_t		*f1, *f2;
    

	int			addplanenum;
	vec3d_t			norms[128];
	fp_t			dists[128];

	pointnum = 0;
	edgenum = 0;

	addplanenum = 0;

	//
	// find common points
	//
	for ( f1 = b->faces; f1 ; f1=f1->next )
	{
		for ( j = 0; j < f1->p->pointnum; j++ )
		{
			fp_t		*p1, *p2;
			int		count;
			vec3d_t		vec;

			p1 = f1->p->p[j];

			//
			// is there allready a bevel plane for this point ?
			//
			for ( i = 0; i < pointnum; i++ )
			{
				if ( CheckSame( p1, points[i] ) )
					break;	// yes
			}
			if ( i != pointnum )
				continue;

			count = 1;
			Vec3dCopy( vec, f1->norm );
			for ( f2 = b->faces; f2 ; f2=f2->next )
			{
				if ( f1 == f2 )
					continue;
				
				for ( k = 0; k < f2->p->pointnum; k++ )
				{
					p2 = f2->p->p[k];
					if ( CheckSame( p1, p2 ) )
					{
						Vec3dAdd( vec, vec, f2->norm );
						count++;
					}					     
				}
			}

			Vec3dCopy( points[pointnum], p1 );
			pointnum++;
			
//			Vec3dScale( norms[addplanenum], 1.0/(fp_t)count, vec );
			Vec3dUnify2( norms[addplanenum], vec );
			dists[addplanenum] = Vec3dDotProduct( norms[addplanenum], p1 );
			addplanenum++;			
		}
	}

#if 1
	//
	// find common edges
	//
	for ( f1 = b->faces; f1 ; f1=f1->next )
	{
		for ( j = 0; j < f1->p->pointnum; j++ )
		{
			fp_t		*p1, *p2, *p3, *p4;
			int		count;
			vec3d_t		vec;

			p1 = f1->p->p[j];
			p2 = f1->p->p[((j+1)==f1->p->pointnum) ? 0 : (j+1)];

			//
			// is there allready a bevel plane for this edge ?
			//
			for ( i = 0; i < edgenum; i++ )
			{
				if ( (CheckSame( p1, edges[i][0] ) && CheckSame( p2, edges[i][1] )) ||
				     (CheckSame( p1, edges[i][1] ) && CheckSame( p2, edges[i][0] )) )
				{
					break;	// yes
				}
			}
			if ( i != edgenum )
				continue;

			count = 1;
			Vec3dCopy( vec, f1->norm );
			for ( f2 = b->faces; f2 ; f2=f2->next )
			{
				if ( f1 == f2 )
					continue;

				for ( k = 0; k < f2->p->pointnum; k++ )
				{
					p3 = f2->p->p[k];
					p4 = f2->p->p[((k+1)==f2->p->pointnum) ? 0 : (k+1)];
					
					if ( CheckSame( p1, p4 ) && CheckSame( p2, p3 ) )
					{
						Vec3dAdd( vec, vec, f2->norm );
						count++;
					}
				}
			}

			Vec3dCopy( edges[edgenum][0], p1 );
			Vec3dCopy( edges[edgenum][1], p2 );
			edgenum++;
			
//			Vec3dScale( norms[addplanenum], 1.0/(fp_t)count, vec );
			Vec3dUnify2( norms[addplanenum], vec );
			dists[addplanenum] = Vec3dDotProduct( norms[addplanenum], p1 );
			addplanenum++;	
			
		}
	}
#endif		

	//
	// build bevel faces
	//
	for ( i = 0; i < addplanenum; i++ )
	{
		face_t		*f;

		f = NEW( face_t );
		f->next = b->faces;
		b->faces = f;

		Vec3dCopy( f->norm, norms[i] );
		f->dist = dists[i];
	}

	printf( "%d ", addplanenum );
}

brush_t * CompileBrushClasses( hobj_t *inbrushes, unsigned int accpet_contents )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	faceiter;
	hobj_t		*brush;
	hobj_t		*face;
	hpair_t		*pair;

	brush_t		*b, *list;
	face_t		*f;

	list = NULL;
	InitClassSearchIterator( &iter, inbrushes, "brush" );
	for ( ; ( brush = SearchGetNextClass( &iter ) ) ; )
	{
		int		contents;

		pair = FindHPair( brush, "content" );
		if ( !pair )
			Error( "missing 'content' in brush.\n" );
		HPairCastToInt_safe( &contents, pair );

		if ( ! ( contents & accpet_contents ) )
			continue;

		b = NEW( brush_t );
		b->self = brush;
		b->next = list;
		list = b;

		b->contents = contents;

		InitClassSearchIterator( &faceiter, brush, "face" );
		for ( ; ( face = SearchGetNextClass( &faceiter ) ) ; )
		{
			f = NEW( face_t );
			f->next = b->faces;
			b->faces = f;

			pair = FindHPair( face, "dist" );
			if ( !pair )
				Error( "missing 'dist' in face.\n" );
			HPairCastToFloat_safe( &f->dist, pair );

			pair = FindHPair( face, "norm" );
			if ( !pair )
				Error( "missing 'norm' in face.\n" );
			HPairCastToVec3d_safe( f->norm, pair );
		}		
	}	

	return list;
}

void BuildBrushClasses( hobj_t *outbrushes, brush_t *list )
{
	brush_t		*b;
	face_t		*f;
	hobj_t		*outbrush;
	hobj_t		*outface;
	hpair_t		*pair;
	char		tt[256];

	for ( b = list; b ; b=b->next )
	{
		sprintf( tt, "#%u", HManagerGetFreeID() );
		outbrush = NewClass( "brush", tt );
		InsertClass( outbrushes, outbrush );	

		sprintf( tt, "%u", b->contents );
		pair = NewHPair2( "int", "content", tt );
		InsertHPair( outbrush, pair );

		for ( f = b->faces; f ; f=f->next )
		{
			sprintf( tt, "#%u", HManagerGetFreeID() );
			outface = NewClass( "face", tt );
			InsertClass( outbrush, outface );
		
			sprintf( tt, "%f", f->dist );
			pair = NewHPair2( "float", "dist", tt );
			InsertHPair( outface, pair );
			
			sprintf( tt, "%f %f %f", f->norm[0], f->norm[1], f->norm[2] );
			pair = NewHPair2( "vec3d", "norm", tt );
			InsertHPair( outface, pair );
		}
	}
}


int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_brush_name;

	fp_t			pump_brush_dist;
	unsigned int		pump_brush_contents;
	unsigned int		accept_contents;
	bool_t			use_brush_bounds;

	hmanager_t	*brushhm;
	hobj_t		*brushes;

	brush_t		*brushlist;
	brush_t		*pumplist;
	brush_t		*boundlist;

	FILE		*h;

	printf( "===== pump - manipulate brushes =====\n" );
	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-i" );
	out_brush_name = GetCmdOpt2( "-o" );

	if ( !in_brush_name )
	{
		in_brush_name = "sbrush.hobj";
		printf( " default input brush class: %s\n", in_brush_name );
	}
	else
	{
		printf( " input brush class: %s\n", in_brush_name );
	}

	if ( !out_brush_name )
	{
		out_brush_name = "_pump_brush.hobj";
		printf( " default output brush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output brush class: %s\n", out_brush_name );
	}

	if ( GetCmdOpt2( "--pump-brush-dist" ) ) 
	{
		pump_brush_dist = atof( GetCmdOpt2( "--pump-brush-dist" ) );
		printf( "Switch: --pump-brush-dist %f\n", pump_brush_dist );

	}
	else
	{
		pump_brush_dist = 0.0;
	}

	if ( GetCmdOpt2( "--pump-brush-contents" ) )
	{
		pump_brush_contents = atoi( GetCmdOpt2( "--pump-brush-contents" ) );
		printf( "Switch: --pump-brush-contents %d\n", pump_brush_contents );
	}
	else
	{
		pump_brush_contents = BRUSH_CONTENTS_HULL;
	}	

	if ( GetCmdOpt2( "--accept-contents" ) )
	{
		accept_contents = atoi( GetCmdOpt2( "--accept-contents" ) );
		printf(" Switch: --accept-contents %d\n", accept_contents );
	}
	else
	{
		accept_contents = 1+2+4+8+16;
	}

	if ( CheckCmdSwitch2( "--use-brush-bounds" ) )
	{
		use_brush_bounds = true;
	}
	else
	{
		use_brush_bounds = false;
	}

	brushhm = NewHManagerLoadClass( in_brush_name );
	if ( !brushhm )
		Error( "can't load input brush class.\n" );




//       PumpBrushes( brushes, HManagerGetRootClass( brushhm ), pump_brush_dist, pump_brush_contents );
	
	brushlist = CompileBrushClasses( HManagerGetRootClass( brushhm ), accept_contents );
 	BuildBrushPolygons( brushlist );

	if ( use_bound_brushes )
	{
		boundlist = BoundBrushList( brushlist, 8 );
	}
	else
	{

	}
//	boundlist = BoundBrushList( brushlist, 8 );
//	pumplist = PumpBrushList( boundlist, pump_brush_dist, pump_brush_contents );


//	BuildBrushPolygons( boundlist );
	{
		brush_t		*b;
		for ( b = brushlist; b ; b=b->next )
		{
			BevelBrush( b );
		}
	}

//	pumplist = PumpBrushList( boundlist, pump_brush_dist, pump_brush_contents );
	pumplist = PumpBrushList( brushlist, pump_brush_dist, pump_brush_contents );
	
	brushes = NewClass( "sbrush", "sbrush1" );
	BuildBrushClasses( brushes, pumplist );
//	BuildBrushClasses( brushes, boundlist );

	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "can't open output class.\n" );
	WriteClass( brushes, h );
	fclose( h );

	HManagerSaveID();
}
