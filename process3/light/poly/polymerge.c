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



// polymerge.c

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
#include "lib_container.h"

//#include "defs.h"

#include "../../csg/cbspbrush.c"

#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

#define MAX_SURFS	( 8192 )

int		g_surfnum = 0;
u_list_t	*g_polys[MAX_SURFS];	// polygon_t list
hpair_t		*g_pairs[MAX_SURFS][2];	// 0 texdef, 1 plane


polygon_t * BuildPolygon( hobj_t *poly )
{
	polygon_t	*p;
	int		num, i;
	char		tt[256];

	EasyFindInt( &num, poly, "num" );

	p = NewPolygon( num );
	p->pointnum = num;

	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		EasyFindVec3d( p->p[i], poly, tt );
	}

	return p;
}

polygon_t * TryMerge_old( polygon_t *w1, polygon_t *w2, vec3d_t plnorm )
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


/*
  ==================================================
  common polygon stuff

  ==================================================
*/

/*
  ==============================
  Poly_CheckForCommonEdge

  ==============================
*/

bool_t Vec3dCheckSame( vec3d_t p1, vec3d_t p2 )
{
	int		k;
	
	for ( k = 0; k < 3; k++ )
	{
		if ( fabs( p1[k] - p2[k] ) > 0.1 )
			return false;
	}
	return true;
}

bool_t Poly_CheckForCommonEdge( polygon_t *p1,
				polygon_t *p2,
				int *p1_leave,
				int *p1_enter,
				int *p2_leave,
				int *p2_enter )
{
	int		i, j, k;
	fp_t		*p1_l;
	fp_t		*p1_e;
	fp_t		*p2_l;
	fp_t		*p2_e;
	
	int		i1_l;
	int		i1_e;
	int		i2_l;
	int		i2_e;

	for ( i = 0; i < p1->pointnum; i++ )
	{
		i1_l = i;
		i1_e = (i+1)%p1->pointnum;

		p1_l = p1->p[i1_l];
		p1_e = p1->p[i1_e];

		for ( j = 0; j < p2->pointnum; j++ )
		{
			i2_l = j;
			i2_e = (j+1)%p2->pointnum;

			p2_l = p2->p[i2_l];
			p2_e = p2->p[i2_e];

			for ( k = 0; k < 3; k++ )
			{
				if ( fabs( p1_l[k] - p2_e[k] ) > 0.1 )
					break;
				if ( fabs( p1_e[k] - p2_l[k] ) > 0.1 )
					break;
			}
			
			if ( k == 3 )
				break;
		}
		if ( j < p2->pointnum )
			break;
	}

	if ( i == p1->pointnum )
		return false;
	
	*p1_leave = i1_l;
	*p1_enter = i1_e;
	       
	*p2_leave = i2_l;
	*p2_enter = i2_e;

	return true;
}

/*
  ==============================
  Poly_SimpleMerge

  ==============================
*/
polygon_t * Poly_SimpleMerge( polygon_t *p1, 
			      polygon_t *p2,
			      int p1_leave,
			      int p1_enter,
			      int p2_leave,
			      int p2_enter )
{ 
	int		j, k, i;

	polygon_t		*pnew;

	pnew = NewPolygon( p1->pointnum + p2->pointnum );
	pnew->pointnum = p1->pointnum + p2->pointnum;

	// start with p1_enter and copy till p1_leave
	
	j = 0;
	k = p1_enter;
	for ( i = 0; i < p1->pointnum; i++, k = ( (k + 1) == p1->pointnum)?0:( k + 1 ) )
	{
		Vec3dCopy( pnew->p[j++], p1->p[k] );		
	}

	// start with p2_enter and copy till p2_leave
	 
	k = p2_enter;
	for ( i = 0; i < p2->pointnum; i++, k = ( (k + 1) == p2->pointnum)?0:( k + 1 ) )
	{
		Vec3dCopy( pnew->p[j++], p2->p[k] );
	}

	return pnew;
}

/*
  ==============================
  Poly_RemoveColinearEdges

  ==============================
*/

bool_t  Vec3dCheckColinear_local( vec3d_t p1, vec3d_t p2, vec3d_t t )                 
{                                  
	int		i;
	double		v1[3], v2[3];
	double		len, scale, dot;
  
	for ( i = 0; i < 3; i++ )
	{
		v1[i] = p1[i]-t[i];
		v2[i] = t[i]-p2[i];
	}
                            
	len = sqrt( v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2] );
	scale = 1.0 / len;
	
	if ( len < 0.1 ) return true;
	
	v1[0] *= scale;
	v1[1] *= scale;
	v1[2] *= scale;
        
	len = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2] );
	scale = 1.0 / len;
	
	if ( len < 0.1 ) return true;
	
	v2[0] *= scale;
	v2[1] *= scale;
	v2[2] *= scale;
        
	dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];
       
        if ( dot > 0.999 )                                
                return true;                                                    
        return false;                                                           
}                                                                               

void Poly_RemoveDuplicatePointsInPlace( polygon_t **inout )
{
	int		i, j, n;
	polygon_t	*in;
	polygon_t	*out;

	in = *inout;

	out = NewPolygon( in->pointnum );

	j = 0;
	for ( i = 0; i < in->pointnum; i++ )
	{
		n = ( i + 1 == in->pointnum )?0:(i+1);
		
		if ( Vec3dCheckSame( in->p[i], in->p[n] ) )
		{
			// remove identical points
			continue;
		}
		
		Vec3dCopy( out->p[j++], in->p[i] );		
	}

	out->pointnum = j;

	FreePolygon( in );

	*inout = out;	
}

void Poly_RemoveColinearEdgesInPlace( polygon_t **inout )
{
	int		i, j, p, n;
	polygon_t	*in;
	polygon_t	*out;

	in = *inout;

	out = NewPolygon( in->pointnum );

	j = 0;
	for ( i = 0; i < in->pointnum; i++ )
	{
		p = ( i - 1 < 0 )?(in->pointnum-1):(i-1);
		n = ( i + 1 == in->pointnum )?0:(i+1);
		
		if ( Vec3dCheckColinear_local( in->p[p], in->p[n], in->p[i] ) )
		{
			// remove colinear points
			continue;
		}
		
		Vec3dCopy( out->p[j++], in->p[i] );		
	}

	out->pointnum = j;

	FreePolygon( in );

	*inout = out;
}

/*
  ==============================
  Poly_IsConvex

  ==============================
*/


bool_t Poly_IsConvex( polygon_t *in )
{
	int		i, n, p;
	vec3d_t		sum;
	vec3d_t		norm;
	fp_t		len;

	if ( in->pointnum < 3 )
	{
//		Error( "Poly_IsConvex: degenerated polygon, pointnum = %d\n", in->pointnum );
		return false;
	}


	Vec3dInit( sum, 0, 0, 0 );
	for ( i = 0; i < in->pointnum; i++ )
	{
		vec3d_t		d1, d2;

		p = ((i-1)<0)?(in->pointnum-1):(i-1);
		n = ((i+1)==in->pointnum)?0:(i+1);

		Vec3dSub( d1, in->p[i], in->p[p] );
		Vec3dSub( d2, in->p[i], in->p[n] );
		Vec3dUnify( d1 );
		Vec3dUnify( d2 );
		Vec3dCrossProduct( norm, d1, d2 );
		Vec3dUnify( norm );
		Vec3dAdd( sum, sum, norm );
	}

	len = Vec3dLen( sum );

	if ( fabs( len - ( in->pointnum*1.0 ) ) > 0.5 )
	{
		return false;
	}

	return true;
//	return false;
}


/*
  ====================
  MergeBrushPolygons

  ====================
*/
polygon_t * TryMerge( polygon_t *p1, polygon_t *p2, vec3d_t norm, fp_t dist )
{
	int	p1_l;
	int	p1_e;
	int	p2_l;
	int	p2_e;

	polygon_t	*merge;
		
	// do they have a common edge ?

	if ( ! Poly_CheckForCommonEdge( p1, p2, &p1_l, &p1_e, &p2_l, &p2_e ) )
		return NULL;

	merge = Poly_SimpleMerge( p1, p2, p1_l, p1_e, p2_l, p2_e );

	Poly_RemoveDuplicatePointsInPlace( &merge );
	Poly_RemoveColinearEdgesInPlace( &merge );

	if ( Poly_IsConvex( merge ) )
	{
		return merge;
	}

	FreePolygon( merge );

	return NULL;

}



void RemovePolygonFromList( u_list_t *list, polygon_t *p )
{
	polygon_t	*p2;
	u_list_iter_t	iter;	

	U_ListIterInit( &iter, list );
	for ( ; ( p2 = U_ListIterNext( &iter ) ) ; )
	{
		if ( p2 == p )
		{
			U_ListIterRemoveGoNext( &iter );
			return;
		}
	}
	Error( "can't find polygon to remove\n" );
}

void MergePolygonsInList( u_list_t *list, vec3d_t norm, fp_t dist )
{
	u_list_iter_t	iter1;
	u_list_iter_t	iter2;

	polygon_t	*p1, *p2;
	polygon_t	*merge;

new_list:
	U_ListIterInit( &iter1, list );
	for ( ; ( p1 = U_ListIterNext( &iter1 ) ) ; )
	{
		U_ListIterInit( &iter2, list );
		for ( ; ( p2 = U_ListIterNext( &iter2 ) ) ; )
		{
			if ( p1 == p2 )
				continue;
			
			merge = TryMerge( p1, p2, norm, dist );
			if ( !merge )
				continue;

			// merge success
			RemovePolygonFromList( list, p1 );
			RemovePolygonFromList( list, p2 );
			FreePolygon( p1 );
			FreePolygon( p2 );
			U_ListInsertAtHead( list, merge );
			goto new_list;
		}
	}
}

/*
  ==============================
  MergeSortedPolygons

  ==============================
*/
void MergeSortedPolygons( hmanager_t *planehm )
{
	int		i;
	int		polynum;

	polynum = 0;
	for ( i = 0; i < g_surfnum; i++ )
	{
		hpair_t		*planeref;
		hobj_t		*plane;
		vec3d_t		norm;
		fp_t		dist;
		// get plane of surface
		
		planeref = g_pairs[i][1];
		plane = HManagerSearchClassName( planehm, planeref->value );
		if ( !plane )
			Error( "can't find plane '%s'\n", planeref->value );
		EasyFindVec3d( norm, plane, "norm" );
		EasyFindFloat( &dist, plane, "dist" );

		MergePolygonsInList( g_polys[i], norm, dist );		
		polynum += U_ListLength( g_polys[i] );
	}

	printf( " %d polygons after merging\n", polynum );
}



/*
  ==============================
  SortPolygonClasses

  ==============================
*/


bool_t InsertPolygonClass( hobj_t *poly )
{
	int		i;
	hpair_t		*texdef;
	hpair_t		*plane;

	texdef = FindHPair( poly, "texdef" );
	if ( !texdef )
		Error( "missing 'texdef'\n" );
	
	plane = FindHPair( poly, "plane" );
	if ( !plane )
		Error( "missing 'plane'\n" );

//	if ( strcmp( "#51645", plane->value ) )
//		return false;

	for ( i = 0; i < g_surfnum; i++ )
	{
		hpair_t		*texdef2;
		hpair_t		*plane2;
		polygon_t	*p;

		texdef2 = g_pairs[i][0];
		plane2 = g_pairs[i][1];
		
		if ( strcmp( texdef->value, texdef2->value ) ||
		     strcmp( plane->value, plane2->value ) )
		{
			// compare failed
			continue;
		}

		// found right surface, insert polygon
		p = BuildPolygon( poly );
//		if ( ! Poly_IsConvex( p ) )
//		{
//			 printf( "InsertPolygonClass: warning none convex input polygon\n" );
//		}
		U_ListInsertAtHead( g_polys[i], p  );
			
		break;		
	}

	if ( i == g_surfnum )
	{
		// no surface found for poly, create new surface
		
		if ( g_surfnum >= MAX_SURFS )
			Error( "reached MAX_SURFS\n" );

		g_polys[g_surfnum] = U_NewList();		
		U_ListInsertAtHead( g_polys[g_surfnum], BuildPolygon( poly ) );
		g_pairs[g_surfnum][0] = texdef;
		g_pairs[g_surfnum][1] = plane;

		g_surfnum++;
	}

	return true;
}

void SortPolygonClasses( hmanager_t *polyhm )
{
	hobj_search_iterator_t	iter;	
	int	polynum;
	hobj_t	*poly;

	InitClassSearchIterator( &iter, HManagerGetRootClass( polyhm ), "polygon" );
	polynum = 0;
	for ( ; ( poly = SearchGetNextClass( &iter ) ) ; )
	{
		if ( ! InsertPolygonClass( poly ) )
			continue;	// maybe it failed

		polynum++;
	}

	printf( " %d polygons on %d surfaces\n", polynum, g_surfnum );
}



/*
  ==============================
  BuildPolygonClasses

  ==============================
*/
void CreatePolygonPairs( hobj_t *polycls, polygon_t *p )
{
	hpair_t		*pair;
	int		i;
	char		tt[256];

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
}


hobj_t * BuildPolygonClasses( void )
{
	int		i;
	hobj_t		*root;
	polygon_t		*poly;
	u_list_iter_t	iter;

	root = NewClass( "polygons", "polygons0" );

	for ( i = 0; i < g_surfnum; i++ )
	{
		U_ListIterInit( &iter, g_polys[i] );

		for ( ; ( poly = U_ListIterNext( &iter ) ) ; )
		{
			hobj_t		*polycls;
			char		tt[256];

//			if ( !Poly_IsConvex( poly ) )
//			{
//				printf( "BuildPolygonClasses: warning none convex polygon after merging\n" );
//			}

			sprintf( tt, "#%u", HManagerGetFreeID() );
			polycls = NewClass( "polygon", tt );
			InsertClass( root, polycls );

			// copy texdef
			InsertHPair( polycls, CopyHPair( g_pairs[i][0] ) );
			// copy plane
			InsertHPair( polycls, CopyHPair( g_pairs[i][1] ) );
			
			// insert polygon data
			CreatePolygonPairs( polycls, poly );			
		}
	}

	return root;
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
	char	*in_poly_name;
	char	*out_poly_name;
	char	*in_plane_name;

	hmanager_t	*polyhm;
	hmanager_t	*planehm;
	hobj_t		*merged;

	FILE		*h;

	printf( "===== polymerge - merge polygons =====\n" );
	SetCmdArgs( argc, argv );

	in_poly_name = GetCmdOpt2( "-i" );
	out_poly_name = GetCmdOpt2( "-o" );
	in_plane_name = GetCmdOpt2( "-pl" );

	if ( !in_poly_name )
	{
		Error( "no input polygon class\n" );
	}
	else
	{
		printf( " input polygon class: %s\n", in_poly_name );
	}

	if ( !out_poly_name )
	{
		Error( "no output polygon class\n" );
	}
	else
	{
		printf( " output polygon class: %s\n", out_poly_name );
	}

	if ( !in_plane_name )
	{
		Error( "no input plane class\n" );
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}

	planehm = ReadPlaneClass( in_plane_name );
	
	printf( "load polygon class ...\n" );
	if ( ! (polyhm = NewHManagerLoadClass( in_poly_name ) ) )
		Error( "load failed\n" );
	     
	printf( "merging polygons ...\n" );

	SortPolygonClasses( polyhm );
	MergeSortedPolygons( planehm );
	merged = BuildPolygonClasses();

	printf( "write polygon class\n" );
	h = fopen( out_poly_name, "w" );
	if ( !h )
		Error( "open failed\n" );
	WriteClass( merged, h );
	fclose( h );

	HManagerSaveID();

	exit(0);      
}
