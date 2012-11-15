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



// lib_poly.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"

static int	stat_polygonnum = 0;
static int	stat_polygontotal = 0;

static fp_t	front_epsilon = ON_EPSILON;
static fp_t	back_epsilon = -ON_EPSILON;

void SetEpsilon( fp_t front, fp_t back )
{
	printf( "lib_poly: front_epsilon from %f to %f\n", front_epsilon, front );
	printf( "lib_poly: back_epsilon from %f to %f\n", back_epsilon, back );
	front_epsilon = front;
	back_epsilon = back;
		
}

/*
  ===================
  DumpPolygonStat

  ===================
*/
void DumpPolygonStat( void )
{
	printf( " stat_polygonnum: %d (%d)\n", stat_polygonnum, stat_polygontotal );
}


//
// NewPolygon
//
polygon_t* NewPolygon( int points )
{
	size_t		size;
	polygon_t	*p;

	if ( points > MAX_POINTS_ON_POLYGON )
		Error( "%d points are too much.\n", points );

	stat_polygonnum++;
	stat_polygontotal++;
	size = (size_t)((polygon_t *)0)->p[points];
	p = (polygon_t *) malloc( size );
	memset( p, 0, size );

	return p;
}

/*
  ==============================
  CreatePolygonFromClass

  ==============================
*/
polygon_t * CreatePolygonFromClass( hobj_t *polygon )
{
	int		pointnum;
	hpair_t		*pair;
	polygon_t	*p;
	int		i;
	char		tt[256];
	
	if ( strcmp( polygon->type, "polygon" ) )
		Error( "class [%s,%s] is not of type 'polygon'\n", polygon->type, polygon->name );

	pair = FindHPair( polygon, "pointnum" );
	if ( !pair )
		pair = FindHPair( polygon, "num" );
	if ( !pair )
		Error( "can't get pointnum of [%s,%s]\n", polygon->type, polygon->name );

	HPairCastToInt( &pointnum, pair );

	p = NewPolygon( pointnum );
	p->pointnum = pointnum;

	for ( i = 0; i < pointnum; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( polygon, tt );
		if ( !pair )
			Error( "can't get point '%s' of [%s,%s]\n", tt, polygon->type, polygon->name );

		HPairCastToVec3d( p->p[i], pair );
	}

	return p;
}

/*
  ==============================
  CreateClassFromPolygon

  ==============================
*/
hobj_t * CreateClassFromPolygon( polygon_t *p )
{
	int		i;
	hobj_t		*polygon;
	char		tt[256];

	polygon = EasyNewClass( "polygon" );
	
	EasyNewInt( polygon, "num", p->pointnum );
	for ( i = 0; i < p->pointnum; i++ )
	{
		sprintf( tt, "%d", i );
		EasyNewVec3d( polygon, tt, p->p[i] );
	}

	return polygon;
}


//
// FreePolygon
//
void FreePolygon( polygon_t *in )
{
	stat_polygonnum--;
	free( in );
}


//
// CopyPolygon
//
polygon_t* CopyPolygon( polygon_t *in )
{
	size_t		size;
	polygon_t	*p;

	stat_polygonnum++;
	stat_polygontotal++;
	size = (size_t)((polygon_t *)0)->p[in->pointnum];
	p = (polygon_t *) malloc( size );
	memcpy( p, in, size );

	return p;	
}


//
// PolygonArea
//
fp_t PolygonArea( polygon_t *in )
{
	int		i;
	vec3d_t		v1, v2, norm;
	fp_t		area;

	area = 0;
	for ( i = 2; i < in->pointnum; i++ )
	{
		Vec3dSub( v1, in->p[i-1], in->p[0] );
		Vec3dSub( v2, in->p[i], in->p[0] );
		Vec3dCrossProduct( norm, v1, v2 );
		area += 0.5 * Vec3dLen( norm );
	}
	return area;
}
	

//
// PolygonCenter
//
void PolygonCenter( polygon_t *in, vec3d_t center )
{
	int		i;
	fp_t		scale;

	Vec3dInit( center, 0, 0, 0 );
	for ( i = 0; i < in->pointnum; i++ )
		Vec3dAdd( center, center, in->p[i] );

	scale = 1.0 / in->pointnum;
	Vec3dScale( center, scale, center );
}

//
// PlaneFromPolygon
//

void	PlaneFromPolygon( polygon_t *in, vec3d_t norm, fp_t *dist )
{

	Vec3dInitPlane( norm, dist, in->p[0], in->p[1], in->p[2] );
}


//
// PolygonFlip
//
polygon_t*	PolygonFlip( polygon_t *in )
{
	int		i;
	polygon_t	*flip;

	flip = NewPolygon( in->pointnum );
	flip->pointnum = in->pointnum;

	for ( i = 0; i < in->pointnum; i++ )
		Vec3dCopy( flip->p[(in->pointnum-1)-i], in->p[i] );

	return flip;
}


//
// BasePolygonForPlane
//

polygon_t*  BasePolygonForPlane( vec3d_t norm, fp_t dist )
{
	int		i;
	vec3d_t		absnorm;
	vec3d_t		org, vup, vright;
	fp_t		d;
	polygon_t	*p;

	
	Vec3dInit( vup, 0,0,0 );
	p = NewPolygon( 4 );
	
	for ( i=0; i<3; i++ )
		absnorm[i] = fabs( norm[i] );

	if ( absnorm[0] >= absnorm[1] && absnorm[0] >= absnorm[2] )
                vup[2] = 1.0;
        else if ( absnorm[1] >= absnorm[0] && absnorm[1] >= absnorm[2] )
                vup[2] = 1.0;
        else if ( absnorm[2] >= absnorm[0] && absnorm[2] >= absnorm[1] )
                vup[0] = 1.0;
	
	d = Vec3dDotProduct( vup, norm );
	Vec3dMA( vup, -d, norm, vup );
	Vec3dUnify( vup );
	
	Vec3dScale( org, dist, norm );
	
	Vec3dCrossProduct( vright, norm, vup );
	Vec3dScale( vup, BASE_POLYGON_SIZE, vup );
	Vec3dScale( vright, BASE_POLYGON_SIZE, vright );


	Vec3dSub( p->p[0], org, vright );
	Vec3dSub( p->p[0], p->p[0], vup );
	
	Vec3dSub( p->p[1], org, vright );
	Vec3dAdd( p->p[1], p->p[1], vup );
	
	Vec3dAdd( p->p[2], org, vright );
	Vec3dAdd( p->p[2], p->p[2], vup );
	
	Vec3dAdd( p->p[3], org, vright );
	Vec3dSub( p->p[3], p->p[3], vup );   	

	p->pointnum = 4;
	return p;	
}


/*
  ====================
  CheckPolygonWithPlane

  ====================
*/
int CheckPolygonWithPlane( polygon_t *in, vec3d_t norm, fp_t dist )
{
	int		i;
	bool_t		front, back;
	fp_t		d;

	front = back = false;

	for ( i = 0; i < in->pointnum; i++ )
	{
		d = Vec3dDotProduct( in->p[i], norm ) - dist;

		// is it back ?
		if ( d < back_epsilon )
		{
			if ( front )
				return POLY_SPLIT;
			back = true;
			continue;
		}
		
		// is it front ?
		if ( d > front_epsilon )
		{
			if ( back )
				return POLY_SPLIT;
			front = true;
			continue;
		}
	}
	if ( back )
		return POLY_BACK;
	if ( front )
		return POLY_FRONT;
	return POLY_ON;
}

//
// ClipPolygon
//
/*
void ClipPolygon( )
{

}
*/

// 
// SplitPolygon
//
void SplitPolygon( polygon_t *in, vec3d_t norm, fp_t dist, polygon_t **front, polygon_t **back )
{  
	int		i, j;
	fp_t		dists[MAX_POINTS_ON_POLYGON+1];
	int		sides[MAX_POINTS_ON_POLYGON+1];
	int		counts[3];
	fp_t		d;
	int		maxpts;
	fp_t		*p1, *p2;
	vec3d_t		pclip;
	polygon_t	*f, *b;

	counts[0] = counts[1] = counts[2] = 0;

	for ( i = 0; i < in->pointnum; i++ ) {
		d = Vec3dDotProduct( in->p[i], norm ) - dist;
		dists[i] = d;

		if ( d > front_epsilon )
			sides[i] = SIDE_FRONT;
		else if ( d < back_epsilon )
			sides[i] = SIDE_BACK;
		else 
			sides[i] = SIDE_ON;

		counts[sides[i]]++;
	}

	dists[i] = dists[0];
	sides[i] = sides[0];
	
	*front = *back = NULL;
	
	if ( !counts[SIDE_FRONT] ) 
	{
		*back = CopyPolygon( in );
		return;
	}

	if ( !counts[SIDE_BACK] )
	{
		*front = CopyPolygon( in );
		return;
	}

	maxpts = in->pointnum+4;

	*front = f = NewPolygon( maxpts );
	*back = b = NewPolygon( maxpts );

	for ( i = 0; i < in->pointnum; i++ )
	{
		p1 = in->p[i];
		
		if ( sides[i] == SIDE_ON )
		{
			Vec3dCopy( f->p[f->pointnum++], p1 );
			Vec3dCopy( b->p[b->pointnum++], p1 );
			continue;
		}

		if ( sides[i] == SIDE_FRONT )
			Vec3dCopy( f->p[f->pointnum++], p1 );

		if ( sides[i] == SIDE_BACK )
			Vec3dCopy( b->p[b->pointnum++], p1 );

		if ( sides[i+1] == SIDE_ON || sides[i+1] == sides[i] )
			continue;

		p2 = in->p[(i+1)%in->pointnum];

		d = dists[i] / ( dists[i]-dists[i+1] );
		for ( j = 0; j < 3; j++ )
		{
			if ( norm[j] == 1.0 )
				pclip[j] = dist;
			else if ( norm[j] == -1.0 )
				pclip[j] = -dist;
			else
				pclip[j] = p1[j] + d*(p2[j]-p1[j]);
		}

		Vec3dCopy( f->p[f->pointnum++], pclip );
		Vec3dCopy( b->p[b->pointnum++], pclip );
	}

	if ( f->pointnum > maxpts || b->pointnum > maxpts )
		Error( "maxpts reached.\n" );
}


/*
  ====================
  ClipPolygonInPlace

  ====================
*/

void ClipPolygonInPlace( polygon_t **inout, vec3d_t norm, fp_t dist )
{
	polygon_t	*front, *back;

	SplitPolygon( *inout, norm, dist, &front, &back );
	
	if ( front )
		FreePolygon( front );

	FreePolygon( *inout );

	*inout = back;
}
