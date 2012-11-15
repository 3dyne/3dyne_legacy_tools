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



// lib_poly2d.c

#include "lib_poly2d.h"

/*
  ====================
  NewPolygon2d

  ====================
*/
polygon2d_t* NewPolygon2d( int pointnum )
{
	size_t		size;
	polygon2d_t	*p;

	if ( pointnum > MAX_POINTS_ON_POLYGON2D )
		Error( "NewPolygon2d: %d points are too much.\n", pointnum );

	size = (size_t)((polygon2d_t *)0)->p[pointnum];
	p = (polygon2d_t *) malloc( size );
	memset( p, 0, size );

	return p;
}

/*
  ====================
  FreePolygon2d

  ====================
*/
void FreePolygon2d( polygon2d_t *in )
{
	free( in );
}

/*
  ====================
  Polygon2dCopy

  ====================
*/
polygon2d_t* CopyPolygon2d( polygon2d_t *in )
{
	size_t		size;
	polygon2d_t	*p;

	size = (size_t)((polygon2d_t *)0)->p[in->pointnum];
	p = (polygon2d_t *) malloc( size );
	memcpy( p, in, size );

	return p;
}


/*
  ====================
  Polygon2dArea

  ====================
*/
fp_t Polygon2dArea( polygon2d_t *in )
{
	int		i;
	vec2d_t		v1, v2;
	fp_t		area;

	area = 0;
	for ( i = 2; i < in->pointnum; i++ )
	{
		Vec2dSub( v1, in->p[i-1], in->p[0] );
		Vec2dSub( v2, in->p[i], in->p[0] );
		area += 0.5*( v1[0]*v2[1] - v1[1]*v2[0] );
	}

	return area;
}

/*
  ====================
  Polygon2dCenter

  ====================
*/
void Polygon2dCenter( polygon2d_t *in, vec2d_t center )
{
	int		i;
	fp_t		scale;

	center[0] = center[1] = 0.0;

	for ( i = 0; i < in->pointnum; i++ )
		Vec2dAdd( center, center, in->p[i] );
	
	scale = 1.0 / (fp_t)in->pointnum;
	Vec2dScale( center, scale, center );
}

/*
  ====================
  SplitPolygon2d

  ====================
*/
void SplitPolygon2d( polygon2d_t *in, vec2d_t norm, fp_t dist, polygon2d_t **front, polygon2d_t **back )
{
	int		i, j;
	fp_t		dists[MAX_POINTS_ON_POLYGON2D+1];
	int		sides[MAX_POINTS_ON_POLYGON2D+1];
	int		counts[3];
	fp_t		d;
	int		maxpts;
	fp_t		*p1, *p2;
	vec2d_t		pclip;
	polygon2d_t	*f, *b;

	counts[0] = counts[1] = counts[2] = 0;

	for ( i = 0; i < in->pointnum; i++ )
	{
		d = Vec2dDotProduct( in->p[i], norm ) - dist;
		dists[i] = d;

		if ( d > ON_EPSILON )
			sides[i] = SIDE_FRONT;
		else if ( d < -ON_EPSILON )
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
		*back = CopyPolygon2d( in );
		return;
	}
	
	if ( !counts[SIDE_BACK] )
	{
		*front = CopyPolygon2d( in );
		return;
	}

	maxpts = in->pointnum+4;

	*front = f = NewPolygon2d( maxpts );
	*back = b = NewPolygon2d( maxpts );

	for ( i = 0; i < in->pointnum; i++ )
	{ 
		p1 = in->p[i];

		if ( sides[i] == SIDE_ON )
		{
			Vec2dCopy( f->p[f->pointnum++], p1 );
			Vec2dCopy( b->p[b->pointnum++], p1 );
			continue;
		}

		if ( sides[i] == SIDE_FRONT )
			Vec2dCopy( f->p[f->pointnum++], p1 );

		if ( sides[i] == SIDE_BACK )
			Vec2dCopy( b->p[b->pointnum++], p1 );

		if ( sides[i+1] == SIDE_ON || sides[i+1] == sides[i] )
			continue;

		p2 = in->p[(i+1)%in->pointnum];

		d = dists[i] / ( dists[i]-dists[i+1] );
		for ( j = 0; j < 2; j++ )
		{
			if ( norm[j] == 1.0 )
				pclip[j] = dist;
			else if ( norm[j] == -1.0 )
				pclip[j] = -dist;
			else
				pclip[j] = p1[j] + d*(p2[j]-p1[j]);
		}

		Vec2dCopy( f->p[f->pointnum++], pclip );
		Vec2dCopy( b->p[b->pointnum++], pclip );
	}

	if ( f->pointnum > maxpts || b->pointnum > maxpts )
		Error( "maxpts reached.\n" );
}


