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



// surfmerge.c

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

//#include "defs.h"

#include "./cbspbrush.c"

#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )


typedef struct listnode_s
{
	void		*data;
	struct listnode_s *next;
} listnode_t;

listnode_t * NewListnode( void )
{
	listnode_t *ln;

	ln = NEW( listnode_t );
	return ln;
}

void FreeListnode( listnode_t *ln )
{
	free( ln );
}

polygon_t * BuildPolygon( hobj_t *poly )
{
	hpair_t		*pair;
	polygon_t	*p;
	int		num, i;
	char		tt[256];

	pair = FindHPair( poly, "num" );
	if ( !pair )
		Error( "missing 'num' in polygon '%s'.\n", poly->name );
	HPairCastToInt_safe( &num, pair );

	p = NewPolygon( num );
	p->pointnum = num;

	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( poly, tt );
		if ( !pair )
			Error( "missing point '%s' in polygon '%s'.\n", tt, poly->name );
		HPairCastToVec3d_safe( p->p[i], pair );
	}

	return p;
}

hobj_t * BuildPolygonClass( polygon_t *p )
{
	hobj_t		*polycls;
	hpair_t		*pair;
	int		i;
	char		tt[256];

	sprintf( tt, "#%u", HManagerGetFreeID() );
	polycls = NewClass( "polygon", tt );	

	sprintf( tt, "%d", p->pointnum );
	pair = NewHPair2( "int", "num", tt );
	InsertHPair( polycls, pair );
	
	for ( i = 0; i < p->pointnum; i++ )
	{
		pair = NewHPair();
		sprintf( pair->type, "vec3d" );
		sprintf( pair->key, "%d", i );
		sprintf( pair->value, "%f %f %f", p->p[i][0], p->p[i][1], p->p[i][2] );

		InsertHPair( polycls, pair );			
	}

	return polycls;
}

/*
  ====================
  MergeBrushPolygons

  ====================
*/
polygon_t * TryMerge( polygon_t *w1, polygon_t *w2, vec3d_t plnorm )
{
	fp_t		*p1, *p2;
	fp_t		*p3, *p4;
	fp_t		*back;
	int		i, j, k, l;

	vec3d_t         norm, delta, planenorm;
	fp_t		d;
	int		keep1, keep2;


	polygon_t	*poly;
	
	p1 = p2 = NULL;
	j = 0;
	for ( i = 0; i < w1->pointnum; i++ )
	{
		p1 = w1->p[i];
		p2 = w1->p[(i+1)%w1->pointnum];

		for ( j = 0; j < w2->pointnum; j++ ) 
		{
			p3 = w2->p[j];
			p4 = w2->p[(j+1)%w2->pointnum];

			for ( k = 0; k < 3; k++ ) 
			{
				if ( fabs( p1[k] - p4[k] ) > 0.1 )
					break;
				if ( fabs( p2[k] - p3[k] ) > 0.1 )
					break;
			}

			if ( k == 3 )
				break;
		}
		if ( j < w2->pointnum )
			break;
	}

	if ( i == w1->pointnum )
		return NULL;

	back = w1->p[(i+w1->pointnum-1)%w1->pointnum];
	Vec3dSub( delta, p1, back );

	Vec3dCrossProduct( norm, plnorm, delta );
	Vec3dUnify( norm );

	back = w2->p[(j+2)%w2->pointnum];
	Vec3dSub( delta, back, p1 );
	d = Vec3dDotProduct( delta, norm );
	if ( d < -0.001 )
		return NULL;
	keep1 = d > 0.001;

	back = w1->p[(i+2)%w1->pointnum];
	Vec3dSub( delta, back, p2 );
	Vec3dCrossProduct( norm, planenorm, delta );
	Vec3dUnify( norm );

	back = w2->p[(j+w2->pointnum-1)%w2->pointnum];
	Vec3dSub( delta, back, p2 );
	d = Vec3dDotProduct( delta, norm );
	if ( d < -0.001 )
		return NULL;
	keep2 = d > 0.001;

	
	poly = NewPolygon( w1->pointnum + w2->pointnum );

	for ( k = (i+1)%w1->pointnum; k!=i; k = (k+1)%w1->pointnum ) 
	{
		if ( k==(i+1)%w1->pointnum && !keep2 )
			continue;
		Vec3dCopy( poly->p[poly->pointnum++], w1->p[k] );
	}

	for ( l = (j+1)%w2->pointnum; l!=j; l = (l+1)%w2->pointnum ) 
	{
		if ( l==(j+1)%w2->pointnum && !keep1 )
			continue;
		Vec3dCopy( poly->p[poly->pointnum++], w2->p[l] );
	}

	return poly;
}

listnode_t * MergePolygons( listnode_t *list, vec3d_t norm )
{
	listnode_t	*l1, *l2, *ln;
	
	polygon_t	*merge;

new_list:
	for ( l1 = list; l1 ; l1=l1->next )
	{
		if ( !l1->data )
			continue;

		for ( l2 = list; l2 ; l2=l2->next )
		{
			if ( !l2->data )
				continue;

			if ( l1->data == l2->data )
				continue;

			merge = TryMerge( l1->data, l2->data, norm );
			if ( !merge )
				continue;

			FreePolygon( l1->data );
			FreePolygon( l2->data );
			l1->data = NULL;
			l2->data = NULL;
	
			ln = NewListnode();
			ln->data = merge;

			ln->next = list;
			list = ln;
			goto new_list;
		}
	}

	return list;
}

int		polygon_num;
int		merge_num;

void MergeBrushPolygons( hmanager_t *brushhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;	
	hobj_search_iterator_t	surfiter;	
	hobj_search_iterator_t	polyiter;	
	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*plane;
	hobj_t		*poly;
	hpair_t		*pair;

	cplane_t	*pl;
	polygon_t	*p;
	listnode_t	*head, *ln, *lnext;

	InitClassSearchIterator( &iter, HManagerGetRootClass( brushhm ), "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &iter ) ) ; )
	{
		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ); )
		{
			pair = FindHPair( surface, "plane" );
			if ( !pair )
				Error( "missing 'plane' in surface '%s'.\n", surface->name );
			plane = HManagerSearchClassName( planehm, pair->value );
			pl = GetClassExtra( plane );

			head = NULL;
			InitClassSearchIterator( &polyiter, surface, "polygon" );
			for ( ; ( poly = SearchGetNextClass( &polyiter ) ) ; )
			{
				polygon_num++;
				p = BuildPolygon( poly );
				ln = NewListnode();
				ln->data = p;
				ln->next = head;
				head = ln;
			}

			head = MergePolygons( head, pl->norm );
			HManagerRemoveAndDestroyAllClassesOfType( brushhm, surface, "polygon" );
			
			for ( ln = head; ln ; ln=lnext )
			{
				lnext = ln->next;
				if ( !ln->data )
				{
					FreeListnode( ln );
					continue;
				}
				merge_num++;
				InsertClass( surface, BuildPolygonClass( ln->data ) );
				FreePolygon( ln->data );
				FreeListnode( ln );
			}
		}
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
		pl->count = 0;

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


int main( int argc, char *argv[] )
{
	char	*in_brush_name;
	char	*out_brush_name;
	char	*in_plane_name;

	hmanager_t	*brushhm;
	hmanager_t	*planehm;

	tokenstream_t	*ts;
	FILE		*h;

	printf( "===== surfmerge - merge polygons on a bspbrush surface =====\n" );
	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-i" );
	out_brush_name = GetCmdOpt2( "-o" );
	in_plane_name = GetCmdOpt2( "-pl" );

	if ( !in_brush_name )
	{
		in_brush_name = "_gather_bspbrush.hobj";
		printf( " default input bspbrush class: %s\n", in_brush_name );
	}
	else
	{
		printf( " input bspbrush class: %s\n", in_brush_name );
	}

	if ( !out_brush_name )
	{
		out_brush_name = "_surfmerge_bspbrush.hobj";
		printf( " default output bspbrush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output bspbrush class: %s\n", out_brush_name );
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

	planehm = ReadPlaneClass( in_plane_name );

	printf( "load brush class ...\n" );
	brushhm = NewHManager();
	ts = BeginTokenStream( in_brush_name );
	if ( !ts )
		Error( "can't open file.\n" );
	HManagerSetRootClass( brushhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( brushhm );

	DeepDumpClass( HManagerGetRootClass( brushhm ) );

	printf( "merging polygons ...\n" );
	polygon_num = 0;
	merge_num = 0;
	MergeBrushPolygons( brushhm, planehm );
	printf( " %d input polygons.\n", polygon_num );
	printf( " %d merged polygons.\n", merge_num );

	printf( "write bspbrush class ...\n" );
	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( brushhm ), h );
	fclose( h );

	HManagerSaveID();
}
