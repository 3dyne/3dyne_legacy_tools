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



// brush.c

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <math.h>
#include <memory.h>

#include "vec.h"
#include "brush.h"
#include "shock.h"
//#include "shock.h"

static int	stat_polygons = 0;
static int	stat_faces = 0;
static int	stat_brushes = 0;

polygon_t* NewPolygon( int points ) 
{
	int		size;
	polygon_t	*p;

	if ( points > MAX_POINTS_ON_POLYGON )
		__error( "%d points are too much.\n", points );

	stat_polygons++;
	size = (int) (long int)((polygon_t *)0)->p[points];
	p = (polygon_t*) malloc( size );
	memset( p, 0, size );

	return p;	
}


void FreePolygon( polygon_t *p )
{
	stat_polygons--;
	free( p );
}


face_t* NewFace( void )
{

	face_t		*f;

	stat_faces++;

	f = (face_t*) malloc( sizeof( face_t ));
	memset( f, 0, sizeof( face_t ));

//	strcpy( f->texdef.ident, "e1u1/box3_2" );
	strcpy( f->texdef.ident, "default" );

	f->texdef.scale[0] = 1.0;
	f->texdef.scale[1] = 1.0;

	return f;
}

void FreeFace( face_t *f )
{
	stat_faces--;
	free( f );
}

brush_t* NewBrush( void )
{
	brush_t		*b;

	stat_brushes++;

	b = (brush_t*) malloc( sizeof( brush_t ));
	memset( b, 0, sizeof( brush_t ));

	return b;
}

void FreeBrush( brush_t *b )
{
	stat_brushes--;
	free( b );
}


void DumpStat( void )
{
	printf("DumpStat:\n");
	printf(" stat_polygons = %d\n", stat_polygons );
	printf(" stat_faces = %d\n", stat_faces );
	printf(" stat_brushes = %d\n", stat_brushes );
}

// ===========================================

void PolygonCenter( polygon_t *in, vec3d_t center )
{
       int             i;                                                      
       fp_t            scale;                                                  
                                                                                
       Vec3dInit( center, 0, 0, 0 );                                           
       for ( i = 0; i < in->pointnum; i++ )                                    
	       Vec3dAdd( center, center, in->p[i] );                           
       
       scale = 1.0 / in->pointnum;                                             
       Vec3dScale( center, scale, center );	
}

#if 0
static vec3d_t baseaxis[18] = { {0,0,1}, {-1,0,0}, {0,-1,0},     // z
				{0,0,-1}, {1,0,0}, {0,-1,0},
				{1,0,0}, {0,0,1}, {0,-1,0},
				{-1,0,0}, {0,0,1}, {0,-1,0},
				{0,1,0}, {1,0,0}, {0,0,-1},
				{0,-1,0}, {1,0,0}, {0,0,-1} };


#else
static vec3d_t baseaxis[18] = { {0,0,1}, {1,0,0}, {0,-1,0},     // z
				{0,0,-1}, {1,0,0}, {0,-1,0},
				{1,0,0}, {0,0,1}, {0,-1,0},
				{-1,0,0}, {0,0,1}, {0,-1,0},
				{0,1,0}, {1,0,0}, {0,0,-1},
				{0,-1,0}, {1,0,0}, {0,0,-1} };
#endif

void TextureAxisFromPlane( plane_t *p, float *xv, float *yv )
{
	int		type;

	type = LibMath_GetNormType( p->norm );

	if ( (type&libMathNormType_axis_mask) == libMathNormType_x ) 
	{
		Vec3dInit( xv, 0, 0, 1 );
		Vec3dInit( yv, 0, 1, 0 ); 
	}
	else if ( (type&libMathNormType_axis_mask) == libMathNormType_y )
	{
		Vec3dInit( xv, 1, 0, 0 );
		Vec3dInit( yv, 0, 0, 1 );
	}
	else if ( (type&libMathNormType_axis_mask) == libMathNormType_z )
	{
		Vec3dInit( xv, 1, 0, 0 );
		Vec3dInit( yv, 0, 1, 0 );
	}

}

void TextureAxisFromPlane_old( plane_t *p, float*xv, float *yv )
{
	int		i;
	float		d, best;
	int		bestaxis;

	best = 0;
	bestaxis = 0;

	for ( i = 0; i < 6; i++ ) {
		
		d = Vec3dDotProduct( p->norm, baseaxis[i*3] );
		if ( d > best ) {
			best = d;
			bestaxis = i;
		}
	}
	Vec3dCopy( xv, baseaxis[bestaxis*3+1] );
	Vec3dCopy( yv, baseaxis[bestaxis*3+2] );
}


polygon_t* BasePolygonForPlane( plane_t *plane, texturedef_t *texdef )
{

	int		i;
	vec3d_t		absnorm;
	vec3d_t		org, vup, vright;
	float		d;
	polygon_t	*p;

	vec3d_t		xaxis, yaxis;
	float		angle, cosv, sinv;
	float		s, t, ns, nt;       
	
	Vec3dInit( vup, 0,0,0 );

	p = NewPolygon( 4 );
	
	for ( i=0; i<3; i++ )
		absnorm[i] = fabs( plane->norm[i] );

	if ( absnorm[0] >= absnorm[1] && absnorm[0] >= absnorm[2] )
                vup[2] = 1.0;
        else if ( absnorm[1] >= absnorm[0] && absnorm[1] >= absnorm[2] )
                vup[2] = 1.0;
        else if ( absnorm[2] >= absnorm[0] && absnorm[2] >= absnorm[1] )
                vup[0] = 1.0;
	
	d = Vec3dDotProduct( vup, plane->norm );
	Vec3dMA( vup, -d, plane->norm, vup );
	Vec3dUnify( vup );
	
	Vec3dScale( org, plane->dist, plane->norm );
	
	Vec3dCrossProduct( vright, plane->norm, vup );
	Vec3dScale( vup, 256.0*128.0, vup );
	Vec3dScale( vright, 256.0*128.0, vright );


	Vec3dSub( p->p[0], org, vright );
	Vec3dSub( p->p[0], p->p[0], vup );
	
	Vec3dSub( p->p[1], org, vright );
	Vec3dAdd( p->p[1], p->p[1], vup );
	
	Vec3dAdd( p->p[2], org, vright );
	Vec3dAdd( p->p[2], p->p[2], vup );
	
	Vec3dAdd( p->p[3], org, vright );
	Vec3dSub( p->p[3], p->p[3], vup );   	

	p->pointnum = 4;

	// texturedef

	if ( !texdef ) {
		printf("WARNING: BasePolygonForPlane no texturedef given.\n");
		return p;
	}

	TextureAxisFromPlane( plane, xaxis, yaxis );

	angle = -texdef->rotate / 180*M_PI;
	sinv = sin( angle );
	cosv = cos( angle );

	for ( i = 0; i < 4; i++ ) {

		s = Vec3dDotProduct( p->p[i], xaxis );
		t = Vec3dDotProduct( p->p[i], yaxis );

		ns = cosv*s - sinv*t;
		nt = sinv*s + cosv*t;

		p->p[i][3] = ns/texdef->scale[0] - texdef->shift[0];
		p->p[i][4] = nt/texdef->scale[1] - texdef->shift[1];
//		printf(" ns = %f, nt = %f\n", ns, nt );
	}
	
	return p;
}


polygon_t* ClipPolygonByPlane( polygon_t *polygon, plane_t *plane )
{
	// the polygon on the BACK_SIDE is kept.
	// if the input polygon is not clipped, it is returned.
	// if it got clipped, the input polygon is freed and a new is returned.
	// if the polygon is clipped away ( all points FRONT_SIDE ) a NULL pointer is returned

	int		i, j;
	float		dist[MAX_POINTS_ON_POLYGON+1];
	int		side[MAX_POINTS_ON_POLYGON+1];
	int		count[3];
	float		d;
	int		maxpts;
	float		*p0, *p1;
	vec5d_t		pclip;
	polygon_t	*pnew;

	count[0] = count[1] = count[2] = 0;

	for ( i=0; i<polygon->pointnum; i++ ) {
		d = Vec3dDotProduct( polygon->p[i], plane->norm ) - plane->dist;
		dist[i] = d;

		if ( d > ON_EPSILON )
			side[i] = SIDE_FRONT;
		else if ( d < -ON_EPSILON )
			side[i] = SIDE_BACK;
		else 
			side[i] = SIDE_ON;

		count[side[i]]++;
	}

	dist[i] = dist[0]; // close the circle
	side[i] = side[0];

	if ( !count[SIDE_BACK] && !count[SIDE_FRONT] && count[SIDE_ON] ) {
		printf(" *** polygon on clipplane *** ");
		FreePolygon( polygon );
		return NULL;
	}

	// the polygon was clipped away
	if ( !count[SIDE_BACK] ) {
		FreePolygon( polygon );
		return NULL;
	}

	// the polygon isn't clipped at all
	if ( !count[SIDE_FRONT] )
		return polygon;

	maxpts = polygon->pointnum + 4;
	pnew = NewPolygon( maxpts );

	for ( i = 0; i < polygon->pointnum; i++ ) {

		p0 = polygon->p[i];
		
		if ( side[i] == SIDE_ON ) {
			Vec5dCopy( pnew->p[pnew->pointnum++], p0 );
			continue;
		}

		if ( side[i] == SIDE_BACK )
			Vec5dCopy( pnew->p[pnew->pointnum++], p0 );

		if ( side[i+1] == SIDE_ON || side[i+1] == side[i] )
			continue;

		p1 = polygon->p[(i+1)%polygon->pointnum];

		d = dist[i] / ( dist[i] - dist[i+1] );
		for ( j = 0; j < 5; j++ ) {
			if ( plane->norm[j] == 1.0 )
				pclip[j] = plane->dist;
			else if ( plane->norm[j] == -1.0 )
				pclip[j] = -plane->dist;
			else
				pclip[j] = p0[j] + d*(p1[j] - p0[j]);
		}
		Vec5dCopy( pnew->p[pnew->pointnum++], pclip );
	}
	if ( maxpts < pnew->pointnum )
		__error("maxpts reached.\n");
	
	FreePolygon( polygon );
	return pnew;
}

void SplitPolygonByPlane( polygon_t *polygon, plane_t *plane, polygon_t **front, polygon_t **back )
{
	// if the polygon is not split, it is returned in front or back.
	// if the polygon got split, two new polygons are returned.
	// the input polygon is not freed.

	int		i, j;
	float		dist[MAX_POINTS_ON_POLYGON+1];
	int		side[MAX_POINTS_ON_POLYGON+1];
	int		count[3];
	float		d;
	int		maxpts;
	float		*p0, *p1;
	vec5d_t		pclip;
	polygon_t	*f, *b;
	
	count[0] = count[1] = count[2] = 0;

	for ( i=0; i<polygon->pointnum; i++ ) {
		d = Vec3dDotProduct( polygon->p[i], plane->norm ) - plane->dist;
		dist[i] = d;

		if ( d > ON_EPSILON )
			side[i] = SIDE_FRONT;
		else if ( d < -ON_EPSILON )
			side[i] = SIDE_BACK;
		else 
			side[i] = SIDE_ON;

		count[side[i]]++;
	}

	dist[i] = dist[0]; // close the circle
	side[i] = side[0];
      
	*front = *back = NULL;

	// the polygon is on the back side
	if ( !count[SIDE_FRONT] ) {
		*back = polygon;
		return;
	}

	// the polygon is on the front side
	if ( !count[SIDE_BACK] ) {
		*front = polygon;
		return;
	}

	maxpts = polygon->pointnum + 4;

	*front = f = NewPolygon( maxpts );
	*back = b = NewPolygon( maxpts );

	for ( i = 0; i < polygon->pointnum; i++ ) {
		
		p0 = polygon->p[i];

		if ( side[i] == SIDE_ON ) {
			Vec5dCopy( f->p[f->pointnum++], p0 );
			Vec5dCopy( b->p[b->pointnum++], p0 );
			continue;
		}

		if ( side[i] == SIDE_FRONT )
			Vec5dCopy( f->p[f->pointnum++], p0 );
	
		if ( side[i] == SIDE_BACK )
			Vec5dCopy( b->p[b->pointnum++], p0 );

		if ( side[i+1] == SIDE_ON || side[i+1] == side[i] )
			continue;

		p1 = polygon->p[(i+1)%polygon->pointnum];

		d = dist[i] / ( dist[i]-dist[i+1] );
		for ( j = 0; j < 5; j++ ) {
			pclip[j] = p0[j] + d*(p1[j]-p0[j]);
		}

		Vec5dCopy( f->p[f->pointnum++], pclip );
		Vec5dCopy( b->p[b->pointnum++], pclip );

	}
	if ( f->pointnum > maxpts || b->pointnum > maxpts )
		__error("maxpts reached.\n");

}

void ClipBrushFaces( brush_t *brush )
{
	// all faceplanes are clipped against every other
	// and generates the polygones for all faces.
	// no care is taken about still allocated polygones
	// the bounding box is updated

	face_t		*f, *clipface;
	face_t		*facehead;

	facehead = NULL;

	printf("ClipBrushFaces\n");


	for ( f = brush->faces; f ; f=f->next ) {
		f->polygon = BasePolygonForPlane( &f->plane, &f->texdef );
	}
	
	for ( f = brush->faces; f ; f=f->next ) {
		for ( clipface = brush->faces; clipface ; clipface=clipface->next ) {
			if ( f == clipface )
				continue;

			if ( f->polygon )
				f->polygon = ClipPolygonByPlane( f->polygon, &clipface->plane );
		}
	}
	
	CalcBrushBounds( brush );
}


void CalcBrushBounds( brush_t *brush )
{
	face_t		*f;
	int	i,j;

	for ( j = 0; j < 3; j++ ) {
		brush->min[j] = 999999;
		brush->max[j] = -999999;
	}

	for ( f = brush->faces; f ; f=f->next ) {

		if ( !f->polygon )
			continue;
				
		for ( i = 0; i < f->polygon->pointnum; i++ ) {
			for ( j = 0; j < 3; j++ ) {
				if ( f->polygon->p[i][j] < brush->min[j] )
					brush->min[j] = f->polygon->p[i][j];
				if ( f->polygon->p[i][j] > brush->max[j] )
					brush->max[j] = f->polygon->p[i][j];
			}
		}
	}
}

void SplitBrushByPlane( brush_t *brush, plane_t *plane, brush_t **front, brush_t **back )
{
	// if the brush is not split, it is returned in front or back.
	// if the brush got split, two new brushes are returned.
	// the input brush is not freed.


	int		i;
	float		d;
	
	face_t		*f;
	face_t		*frontface;
	face_t		*backface;
	face_t		*fnew;

	int		count[3];
	int		side[MAX_POINTS_ON_POLYGON+1];

	brush_t		*bnew;

	printf("SplitBrushByPlane\n");

	frontface = backface = NULL;

	for ( f = brush->faces; f ; f=f->next ) {
		
		count[0] = count[1] = count[2] = 0;

		for ( i = 0; i < f->polygon->pointnum; i++ ) {
			d = Vec3dDotProduct( f->polygon->p[i], plane->norm ) - plane->dist;

			if ( d > ON_EPSILON )
				side[i] = SIDE_FRONT;
			else if ( d < -ON_EPSILON )
				side[i] = SIDE_BACK;
			else
				side[i] = SIDE_ON;
			
			count[side[i]]++;
		}

		// face is on splitplane, ignore it
		if ( !count[SIDE_BACK] && !count[SIDE_FRONT] )
			continue;
		
		if ( !count[SIDE_BACK] ) {
			printf(" face only on the frontside.\n" );
			
			// duplicate face
			fnew = NewFace();
			fnew->plane.dist = f->plane.dist;
			Vec3dCopy( fnew->plane.norm, f->plane.norm );

			// to the list
			fnew->next = frontface;
			frontface = fnew;

			continue;
		}

		if ( !count[SIDE_FRONT] ) {
			printf(" face only on the backside.\n" );

			// duplicate face
			fnew = NewFace();
			fnew->plane.dist = f->plane.dist;
			Vec3dCopy( fnew->plane.norm, f->plane.norm );

			// to the list
			fnew->next = backface;
			backface = fnew;

			continue;
		}

		printf(" face on both sides.\n" );
		// duplicate face
		fnew = NewFace();
		fnew->plane.dist = f->plane.dist;
		Vec3dCopy( fnew->plane.norm, f->plane.norm );
		
		// to the list
		fnew->next = frontface;
		frontface = fnew;
		
		// duplicate face
		fnew = NewFace();
		fnew->plane.dist = f->plane.dist;
		Vec3dCopy( fnew->plane.norm, f->plane.norm );
		
		// to the list
		fnew->next = backface;
		backface = fnew;
	}	

	if ( !backface ) {
		printf(" brush is only on the front side.\n");
		return;			
	}

	if ( !frontface ) {
		printf(" brush is only on the back side.\n");
		return;
	}

	printf(" brush was splitted into frontbrush and backbrush.\n" );	

	// add the splitplane to the new backbrush
	fnew = NewFace();
	fnew->plane.dist = plane->dist;
	Vec3dCopy( fnew->plane.norm, plane->norm );
	
	// to the list
	fnew->next = backface;
	backface = fnew;
	
	// create a new brush for the faces
	bnew = NewBrush();
	bnew->faces = backface;
	
	*back = bnew;

	// add the flipped splitplane to the new frontbrush
	
	fnew = NewFace();
	fnew->plane.dist = -plane->dist;
	Vec3dCopy( fnew->plane.norm, plane->norm );
	Vec3dScale( fnew->plane.norm, -1, fnew->plane.norm );
	
	// to the list
	fnew->next = frontface;
	frontface = fnew;
	
	// create a new brush for the faces
	bnew = NewBrush();
	bnew->faces = frontface;
	
	*front = bnew;
}


int AddFaceToBrush( brush_t *brush, face_t *fnew )
{
	// add face to brush
	// the face is not added if a face with the same plane allready exists ( return false ) !

//	face_t		*fnew;
	face_t		*f;

	printf("AddPlaneToBrush\n");

	for ( f = brush->faces; f ; f=f->next ) {

		if ( fabs( f->plane.norm[0] - fnew->plane.norm[0] ) < ON_EPSILON &&
		     fabs( f->plane.norm[1] - fnew->plane.norm[1] ) < ON_EPSILON &&
		     fabs( f->plane.norm[2] - fnew->plane.norm[2] ) < ON_EPSILON &&
		     fabs( f->plane.dist - fnew->plane.dist ) < ON_EPSILON ) {

			printf(" face allready exists !\n");
			return 0;
		}

		if ( fabs( f->plane.norm[0] + fnew->plane.norm[0] ) < ON_EPSILON &&
		     fabs( f->plane.norm[1] + fnew->plane.norm[1] ) < ON_EPSILON &&
		     fabs( f->plane.norm[2] + fnew->plane.norm[2] ) < ON_EPSILON &&
		     fabs( f->plane.dist + fnew->plane.dist ) < ON_EPSILON ) {

			printf(" flipped face allready exists !\n");
//			return 1;
		}		
	}

//	fnew = NewFace();
	
//	Vec3dCopy( fnew->plane.norm, plane->norm );
//	fnew->plane.dist = plane->dist;

	fnew->next = brush->faces;
	brush->faces = fnew;
	return 1;
}

int CheckBrushAndPlane( brush_t *brush, plane_t *plane )
{
	// checks wether all polygons are on the back side of the plane

	face_t		*f;
	int		i;
	float		d;

	for ( f = brush->faces; f ; f=f->next ) {
		
		if ( !f->polygon )
			continue;

		for ( i = 0; i < f->polygon->pointnum; i++ ) {
			d = Vec3dDotProduct( f->polygon->p[i], plane->norm ) - plane->dist;
			
			if ( d < -ON_EPSILON )
				return 1;
		}			
	}

	printf("CheckBrushAndPlane: plane clips away all polygons.\n");

	return 0;
}
	
void CleanUpBrush( brush_t *brush )
{
	// all faceplanes are clipped against every other
	// and generates the polygones for all faces.
	// no care is taken about still allocated polygones
	// the bounding box is updated


	face_t		*f, *fnext, *clipface;
	face_t		*facehead;

//	printf("CleanUpBrush\n");
       
	for ( f = brush->faces; f ; f=f->next ) {
		f->polygon = BasePolygonForPlane( &f->plane, &f->texdef );
	}
	

	for ( f = brush->faces; f ; f=f->next ) {
		for ( clipface = brush->faces; clipface ; clipface=clipface->next ) {
			if ( f == clipface )
				continue;

			// something left of the polygon ?
			if ( f->polygon )
				f->polygon = ClipPolygonByPlane( f->polygon, &clipface->plane );	       
		}
	}


	facehead = NULL;

	for ( f = brush->faces; f ; f=fnext ) {
		
		fnext = f->next;

		if ( f->polygon ) {
			f->next = facehead;
			facehead = f;
		}
		else {
			printf(" CleanUpBrush: null polygon.\n");
			FreeFace( f );
		}
	}

	if ( !facehead ) {
		printf(" CleanUpBrush: degenerated brush.\n");
	}

	brush->faces = facehead;
	CalcBrushBounds( brush );
}

void CopyBrush( brush_t **out, brush_t *in )
// copy the plane information of a brush into a new generated
{
	brush_t		*bnew;
	face_t		*f;
	face_t		*fnew;
	face_t		*facehead;
	
	facehead = NULL;
	for ( f = in->faces; f ; f=f->next ) {

		fnew = NewFace();
		
//		Vec3dCopy( fnew->plane.norm, f->plane.norm );
//		fnew->plane.dist = f->plane.dist;
		memcpy( fnew, f, sizeof( face_t ) );

		fnew->polygon = NULL;
		fnew->next = facehead;
		facehead = fnew;
	}

	bnew = NewBrush();
	bnew->faces = facehead;	
	bnew->contents = in->contents;
	bnew->id = in->id;
	*out = bnew;
}

void FreeBrushPolygons( brush_t *brush )
{
	// frees polygons of a brush

	face_t	*f;

	for ( f = brush->faces; f ; f=f->next ) {
		
		if ( f->polygon ) {
			FreePolygon( f->polygon );
			f->polygon = NULL;
		}
	}	
}


void FreeBrushFaces( brush_t *brush )
{
	// frees faces _and_ polygons of a brush

	face_t	*f;
	face_t	*fnext;

	for ( f = brush->faces; f ; f=fnext ) {
		
		fnext = f->next;
		if ( f->polygon )
			FreePolygon( f->polygon );
		FreeFace( f );
	}	
}


face_t* ClipRayByBrush( brush_t *brush, vec3d_t from, vec3d_t to )
{
	int		i;
	face_t		*f;
	float		d0, d1, m;
	face_t		*hitface, *leaveface;
       
	hitface = NULL;
	leaveface = NULL; 

	for ( f = brush->faces; f ; f=f->next ) {
		
		d0 = Vec3dDotProduct( f->plane.norm, from ) - f->plane.dist;
		d1 = Vec3dDotProduct( f->plane.norm, to ) - f->plane.dist;


		if ( d0 > 0 && d1 > 0 ) {
			return NULL;
		}

		if ( d0 > 0 && d1 < 0 ) {
			hitface = f;		
			// front plane
			m = d0 / (d0-d1);
			for ( i = 0; i < 3; i++ )
				from[i] = from[i] + m*(to[i]-from[i]);
			
		}

		if ( d0 < 0 && d1 > 0 ) {
			leaveface = f;
			// back face
			m = d0 / (d0-d1);
			for ( i = 0; i < 3; i++ )
				to[i] = from[i] + m*(to[i]-from[i]);
			
		}
	}

	// leaveface ???
	return hitface;

}

void	PlaneFromPolygon( polygon_t *in, vec3d_t norm, fp_t *dist )
{

	Vec3dInitPlane( norm, dist, in->p[0], in->p[1], in->p[2] );
}
