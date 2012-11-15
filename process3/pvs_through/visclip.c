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



// visclip.c

#include "visclip.h"
#include "lib_gldbg.h"

extern gld_session_t *gldbg;

visclip_t * NewVisclip( int planenum )
{
	visclip_t	*vis;
	size_t		size;

	size = (size_t)&(((visclip_t *)0)->planes[planenum]);
	vis = (visclip_t *) malloc( size );
	memset( vis, 0, size );

	return vis;
}

void FreeVisclip( visclip_t *vis )
{
	free( vis );
}

#define MAX_VISCLIP_PLANES	( 64 ) 
visclip_t * SetupNewVisclip_old( polygon_t *from, polygon_t *through, bool_t flip_from )
{
#if 1
	int		i, j, k, l;

	fp_t		*p1, *p2, *p3;
	vec3d_t		norm;
	fp_t		dist;
	fp_t		d;
	
	int		planenum;
	plane_t		tmpp[MAX_VISCLIP_PLANES];
	visclip_t	*vc;

	planenum = 0;

	for ( i = 0; i < from->pointnum; i++ )
	{
		l = (i+1)%from->pointnum;
		if ( flip_from )
		{
			p1 = from->p[i];	
			p2 = from->p[l];
		}
		else
		{
			p1 = from->p[l];	
			p2 = from->p[i];	
		}
		for ( j = 0; j < through->pointnum; j++ )
		{
			p3 = through->p[j];

			if ( !Vec3dInitPlane( norm, &dist, p1, p2, p3 ) )
			{
				// can't get valid plane
//				printf( " plane failed.\n" );
				continue;
			}

#if 1
			for ( k = 0; k < through->pointnum; k++ )
			{
				d = Vec3dDotProduct( through->p[k], norm ) - dist;
				
				if ( d > ON_EPSILON )
					break;
			}

			if ( k == through->pointnum )
				break;
		}

		if ( j == through->pointnum )
		{
//			printf( "no valid plane found.\n" );
			return NULL;
		}

#else
			// on which side of plane is from ?
			fliptest = false;
			for ( k = 0; k < from->pointnum; k++ )
			{
				if ( k == i || k == l )
					continue;
				
				d = Vec3dDotProduct( through->p[k], norm ) - dist;

				if ( d < -ON_EPSILON )
				{
					// from is backside
					fliptest = false
					break;
				}
				else if ( d > ON_EPSILON )
				{
					// from is frontside
					fliptest = true;
				}
			}
			if ( k == from->pointnum )
				continue;

			if ( fliptest )
			{
				Vec3dFlip( norm );
				dist = -dist;
			}
#endif
			
			
		// points of <from> frontside
		for ( k = 0; k < from->pointnum; k++ )
		{
			d = Vec3dDotProduct( from->p[k], norm ) - dist;

			// no point backside
			if ( d < -ON_EPSILON )
				break;
		}
		if ( k != from->pointnum )
		{
//			printf( "no valid plane found ( from on front ).\n" );
//			return NULL;			
			continue;
		}

		if ( planenum == MAX_VISCLIP_PLANES )
			Error( "SetupNewVisclip: reached MAX_VISCLIP_PLANES.\n" );

		Vec3dCopy( tmpp[planenum].norm, norm );
		tmpp[planenum].dist = dist;
		planenum++;
	}
		
		
//	if ( planenum < 3 )
//		Error( "Visclip: planenum < 3\n" );
//		return NULL;
     
	vc = NewVisclip( planenum );
	vc->planenum = planenum;
   
	
	for ( i = 0; i < planenum; i++ )
	{
		memcpy( &vc->planes[i], &tmpp[i], sizeof( plane_t ) );
	}
	
	return vc;			

#endif
}

polygon_t * PolygonVisclip( visclip_t *vc, polygon_t *to )
{
	int		i;
	polygon_t	*p;

	if ( !to )
		Error( "PolygonVisclip: no valid polygon.\n" );

	p = CopyPolygon( to );

	for ( i = 0; i < vc->planenum; i++ )
	{
		ClipPolygonInPlace( &p, vc->planes[i].norm, vc->planes[i].dist );
		if ( !p )
			return NULL;
	}

//	FreePolygon( p );
	return p;
}

void PolygonVisclipInPlace( visclip_t *vc, polygon_t **inout )
{
	int		i;

	if ( !*inout )
		Error( "PolygonVisclip: no valid polygon.\n" );

	for ( i = 0; i < vc->planenum; i++ )
	{
		ClipPolygonInPlace( inout, vc->planes[i].norm, vc->planes[i].dist );
		if ( !*inout )
			break;
	}
}

visclip_t * SetupNewVisclip_2( polygon_t *from, polygon_t *through, bool_t flipclip )
{
	int		i, j, k, l;
	int		planenum = 0;
	vec3d_t		v1, v2;
	fp_t		d;
	fp_t		length;
	int		counts[3];
	bool_t		fliptest;
	vec3d_t		norms[256];
	fp_t		dists[256];
	visclip_t	*vis;

	for ( i = 0; i < from->pointnum; i++ )
	{
		l = (i+1) % from->pointnum;
		Vec3dSub( v1, from->p[l], from->p[i] );

		for ( j = 0; j < through->pointnum; j++ )
		{
			Vec3dSub( v2, through->p[j], from->p[i] );
			
			norms[planenum][0] = v1[1]*v2[2] - v1[2]*v2[1];
			norms[planenum][1] = v1[2]*v2[0] - v1[0]*v2[2];
			norms[planenum][2] = v1[0]*v2[1] - v1[1]*v2[0];

			length = norms[planenum][0]*norms[planenum][0]
				+norms[planenum][1]*norms[planenum][1] 
				+norms[planenum][2]*norms[planenum][2];

			if ( length < ON_EPSILON )
				continue;

			length = 1.0/sqrt(length);

			norms[planenum][0] *= length;
			norms[planenum][1] *= length;
			norms[planenum][2] *= length;

			dists[planenum] = Vec3dDotProduct( through->p[j], norms[planenum] );

			fliptest = false;
			for ( k = 0; k < from->pointnum; k++ )
			{
				if ( k == i || k == l )
					continue;

				d = Vec3dDotProduct( from->p[k], norms[planenum] ) - dists[planenum];
				if ( d < -ON_EPSILON )
				{
					fliptest = false;
					break;
				}
				else if ( d > ON_EPSILON )
				{
					fliptest = true;
					break;
				}
			}
			if ( k == from->pointnum )
				continue;

			if ( fliptest )
			{
				Vec3dFlip( norms[planenum], norms[planenum] );
				dists[planenum] = -dists[planenum];
			}

			counts[0] = counts[1] = counts[2] = 0;
			for ( k = 0; k < through->pointnum; k++ )
			{
				if ( k == j )
					continue;
				d = Vec3dDotProduct( through->p[k], norms[planenum] ) - dists[planenum];
				if ( d < -ON_EPSILON )
					break;
				else if ( d > ON_EPSILON )
					counts[0]++;
				else
					counts[2]++;
			}

			if ( k != through->pointnum )
				continue;
			if ( !counts[0] )
				continue;

			if ( flipclip )
			{
				Vec3dFlip( norms[planenum], norms[planenum] );
				dists[planenum] = -dists[planenum];
			}

			planenum++;
		}
	}

	vis = NewVisclip( planenum );
	vis->planenum = planenum;
	for( i = 0; i < planenum; i++ )
	{
		Vec3dCopy( vis->planes[i].norm, norms[i] );
		vis->planes[i].dist = dists[i];
	}
	return vis;	
}

typedef enum
{
	SideType_on = 0,
	SideType_front,
	SideType_back,
} sideType;

visclip_t * SetupNewVisclip( polygon_t *from, polygon_t *through, bool_t flip )
{
	int		i, j, k, l;
	int		planenum;
	fp_t		*p1, *p2, *p3;
	plane_t		planes[128];
	vec3d_t		norm;
	fp_t		dist, d;

	int		count[3];
	sideType	through_side;
	sideType	from_side;

	visclip_t	*vis;

	planenum = 0;
	for ( i = 0; i < through->pointnum; i++ )
	{
		// get two points from 'through'

		l = ( i+1 == through->pointnum ) ? 0 : ( i + 1 );
		p1 = through->p[i];
		p2 = through->p[l];

		for ( j = 0; j < from->pointnum; j++ )
		{
			// get one point from 'from'

			p3 = from->p[j];

			// make these points a valid plane
			if ( !Vec3dInitPlane( norm, &dist, p1, p2, p3 ) )
			{
				// no
				continue;
			}

			// on which side is 'through'
			count[0] = count[1] = count[2] = 0;		     
			for ( k = 0; k < through->pointnum; k++ )
			{
				if ( k == i || k == l )
					continue;
				d = Vec3dDotProduct( through->p[k], norm ) - dist;
				if ( d < -ON_EPSILON )
				{
					// backside
					count[SideType_back]++;
				}
				else if ( d > ON_EPSILON )
				{
					// frontside
					count[SideType_front]++;
				}
				else
				{
					// on
					count[SideType_on]++;
				}
			}

			if ( !count[SideType_front] && count[SideType_back] )
			{
				through_side = SideType_back;
			}
			else if ( count[SideType_front] && !count[SideType_back] )
			{
				through_side = SideType_front;
			}
//			else if ( !count[SideType_front] && !count[SideType_back] && count[SideType_on] )
			else
			{
				through_side = SideType_on;
			}

			// on which side is 'from'
			count[0] = count[1] = count[2] = 0;		     
			for ( k = 0; k < from->pointnum; k++ )
			{
				if ( k == j )
					continue;
				d = Vec3dDotProduct( from->p[k], norm ) - dist;
				if ( d < -ON_EPSILON )
				{
					// backside
					count[SideType_back]++;
				}
				else if ( d > ON_EPSILON )
				{
					// frontside
					count[SideType_front]++;
				}
				else
				{
					// on
					count[SideType_on]++;
				}
			}
		 
			if ( count[SideType_front] && count[SideType_back] )
			{
				// from got split
				continue;
			}

			if ( !count[SideType_front] && count[SideType_back] )
			{
				from_side = SideType_back;
			}
			else if ( count[SideType_front] && !count[SideType_back] )
			{
				from_side = SideType_front;
			}
//			else if ( !count[SideType_front] && !count[SideType_back] && count[SideType_on] )
			else
			{
				from_side = SideType_on;
			}

			// test
		      

			if ( from_side == through_side )
			{
				// same side
				continue;
			}

			else if ( from_side == SideType_back && through_side == SideType_front )
			{
				// plane needs flip
				Vec3dFlip( norm, norm );
				dist = -dist;
			}
			else if ( through_side == SideType_on && from_side == SideType_back )
			{
				Vec3dFlip( norm, norm );
				dist = -dist;
			}
			else if ( from_side == SideType_on && through_side == SideType_front )
			{
				Vec3dFlip( norm, norm );
				dist = -dist;
			}
			

			Vec3dCopy( planes[planenum].norm, norm );
			planes[planenum].dist = dist;
			planenum++;

		}		
	}

	if ( planenum < 3 )
	{
//		printf( "planenum less than three.\n" );
		return NULL;
	}

	vis = NewVisclip( planenum );
	vis->planenum = planenum;
	for( i = 0; i < planenum; i++ )
		memcpy( &vis->planes[i], &planes[i], sizeof( plane_t ) );

	return vis;

}

void DrawVisclip( visclip_t *vis )
{
	int             i, j;
	polygon_t	*p;

	// speedup hack
	if ( !gldbg->record_active )
		return;

	for ( i = 0; i < vis->planenum; i++ )
	{
		p = BasePolygonForPlane( vis->planes[i].norm, vis->planes[i].dist );

		for ( j = 0; j < vis->planenum; j++ )
		{
			if ( i == j )
				continue;
			if ( p )
				ClipPolygonInPlace( &p, vis->planes[j].norm, vis->planes[j].dist );
		}

		if ( !p )
		{
			printf( "draw visclip, null polygon\n" );
		}
		else
		{
			GLD_EasyPolygon( gldbg, p );
			FreePolygon( p );
		}
	}
}

