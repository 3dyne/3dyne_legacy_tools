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



// patch.c

#include "light.h"

patch_t * NewPatch( void )
{
	return NEW( patch_t );
}

void FreePatch( patch_t *patch )
{
	free( patch );
}

rpatch_t * NewRPatch( void )
{
	return NEW( rpatch_t );
}

void FreeRPatch( rpatch_t *p )
{
	free( p );
}

int	patch_num = 0;
int	rpatch_num = 0;

//#define PATCH_SIZE	( 16 )

void BuildFacePatches_old( face_t *f )
{
	vec3d_t		right, up;
	int		i;
	fp_t		x, y;

	// four clip planes
	vec3d_t		norms[4];
	fp_t		dists[4];
	polygon_t	*poly;
	patch_t		*patch;


	GetProjectionVecs( right, up, f->projection );

	f->patches = NULL;
	f->rpatches = NULL;
	for ( x = f->min2d[0]; x < f->max2d[0]; x+=f->patchsize )
	{
		Vec3dFlip( norms[0], right );
		Vec3dCopy( norms[1], right );
		dists[0] = -x;
		dists[1] = x+f->patchsize;
		
		for ( y = f->min2d[1]; y < f->max2d[1]; y+=f->patchsize )
		{
			Vec3dFlip( norms[2], up );
			Vec3dCopy( norms[3], up );
			dists[2] = -y;
			dists[3] = y+f->patchsize;
			
			poly = CopyPolygon( f->p );
			for ( i = 0; i < 4; i++ )
			{
				ClipPolygonInPlace( &poly, norms[i], dists[i] );
				if ( !poly )
					break;
			}
			if ( !poly )
				continue;

			patch = NewPatch();
			PolygonCenter( poly, patch->origin );
			Vec3dCopy( patch->norm, f->pl->norm );
			Vec3dMA( patch->center, 1.0, patch->norm, patch->origin );
			patch->area = PolygonArea( poly );

			patch->next = f->patches;
			f->patches = patch;

			FreePolygon( poly );

			patch_num++;
			
		}
	}
}

#define RPATCH_SIZE		( 128.0 )

void BuildFacePatches( face_t *f )
{
	vec3d_t		right, up;
	int		i;
	fp_t		x, y;
	fp_t		x2, y2;

	// four clip planes
	vec3d_t		norms[4];
	fp_t		dists[4];
	vec3d_t		norms2[4];
	fp_t		dists2[4];
	polygon_t	*rpoly, *poly;
	patch_t		*patch;
	rpatch_t	*rpatch;

	GetProjectionVecs( right, up, f->projection );

	f->patches = NULL;
	f->rpatches = NULL;
	for ( x = f->min2d[0]; x < f->max2d[0]; x+=RPATCH_SIZE )
	{
		Vec3dFlip( norms[0], right );
		Vec3dCopy( norms[1], right );
		dists[0] = -x;
		dists[1] = x+RPATCH_SIZE;
		
		for ( y = f->min2d[1]; y < f->max2d[1]; y+=RPATCH_SIZE )
		{
			Vec3dFlip( norms[2], up );
			Vec3dCopy( norms[3], up );
			dists[2] = -y;
			dists[3] = y+RPATCH_SIZE;
			
			// chop rpatch
			
			rpoly = CopyPolygon( f->p );
			for ( i = 0; i < 4; i++ )
			{
				ClipPolygonInPlace( &rpoly, norms[i], dists[i] );
				if ( !rpoly )
					break;
			}
			if ( !rpoly )
				continue;

			rpatch = NewRPatch();
			PolygonCenter( rpoly, rpatch->origin );
			Vec3dMA( rpatch->origin, 1.0, f->pl->norm, rpatch->origin );

			rpatch->next = f->rpatches;
			f->rpatches = rpatch;
			rpatch_num++;


			// chop real patches
			
			for ( x2 = x; x2 < x+RPATCH_SIZE; x2+=f->patchsize )
			{
				Vec3dFlip( norms2[0], right );
				Vec3dCopy( norms2[1], right );
				dists2[0] = -x2;
				dists2[1] = x2+f->patchsize;	

				for ( y2 = y; y2 < y+RPATCH_SIZE; y2+=f->patchsize )
				{
					Vec3dFlip( norms2[2], up );
					Vec3dCopy( norms2[3], up );
					dists2[2] = -y2;
					dists2[3] = y2+f->patchsize;
					
					poly = CopyPolygon( rpoly );
					for ( i = 0; i < 4; i++ )
					{
						ClipPolygonInPlace( &poly, norms2[i], dists2[i] );
						if ( !poly )
							break;
					}
					if ( !poly )
						continue;
					
					
					patch = NewPatch();
					PolygonCenter( poly, patch->origin );
					Vec3dCopy( patch->norm, f->pl->norm );
					Vec3dMA( patch->center, 1.0, patch->norm, patch->origin );
					patch->area = PolygonArea( poly );
					
					patch->rpatch = rpatch;
					
					patch->next = f->patches;
					f->patches = patch;
					
					FreePolygon( poly );
					
					patch_num++;
					
				}
			}	
		}
	}
}

int	face16_num = 0;
int	face32_num = 0;
int	face64_num = 0;

void ClassifyFace( face_t *f, bool_t fixedsize )
{
	int		i;
	vec2d_t		v;
	vec2d_t		min, max;
	projectionType		type;
	fp_t		dx, dy, big;

	Vec3dInitBB( f->min3d, f->max3d, 999999.9 );
//	f->patchwidth = 16.0;
//	f->patchheight = 16.0;

	// get projection plane for lightmap
	type = GetProjectionTypeOfPlane( f->pl );
	f->projection = type;

	Vec2dInitBB( min, max, 999999.9 );	
	for ( i = 0; i < f->p->pointnum; i++ )
	{
		ProjectVec3d( v, f->p->p[i], type );
		Vec2dAddToBB( min, max, v );
		Vec3dAddToBB( f->min3d, f->max3d, f->p->p[i] );
	}

	Vec2dCopy( f->min2d, min );
	Vec2dCopy( f->max2d, max );

#if 0
	if ( fixedsize )
	{
		// force patchsize of 16
		f->patchsize = 16.0;
		return;
	}
#endif

	dx = max[0] - min[0];
	dy = max[1] - min[1];

	if ( dx >= dy )
		big = dx;
	else
		big = dy;
	

	// max lightmap size is 128*128
	// - bilinear filter boarder => 127*127 patches
	
	// patch size 16, 16*127 up to edge length 2032
	if ( big < 2032 )
	{
		f->patchsize = 16.0;
		face16_num++;
	}
	// patch size 32, 32*127 up to edge length 4064
	else if ( big < 4064 )
	{
		f->patchsize = 32.0;
		face32_num++;
	}
	// patch size 64, 64*127 up to edge length 8128
	else if ( big < 8128 )
	{
		f->patchsize = 64.0;
		face64_num++;
	}
	else
	{
		Error( "ClassifyFace: face is too big.\n" );
	}
//	f->patchsize = 64.0;
}

void SetupPatches( face_t *list, bool_t fixedsize )
{
	face_t		*f;

	printf( "setup face patches ...\n" );
	
	for ( f = list; f ; f=f->next )
	{
		ClassifyFace( f, fixedsize );
		BuildFacePatches( f );
	}

	printf( " %d patches\n", patch_num );
	printf( " %d faces with patchsize 16.0\n", face16_num );
	printf( " %d faces with patchsize 32.0\n", face32_num );
	printf( " %d faces with patchsize 64.0\n", face64_num );
	printf( " %d radiosity patches\n", rpatch_num );
}
