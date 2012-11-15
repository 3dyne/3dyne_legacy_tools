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

/*
  ==================================================
  create patches

  ==================================================
*/

patch_t * NewPatch( void )
{
	return NEWTYPE( patch_t );
}

void FreePatch( patch_t *patch )
{
	free( patch );
}

int	patch_num = 0;

void BuildWorldScraps( face_t *f, patch_t *p, polygon_t *poly )
{
	int		i;
	int		xofs, yofs;
	vec2d_t		min, max;
	vec2d_t		v;

	vec2d_t		split_pos;

	// split plane
	vec3d_t			right, up;

	polygon_t		*polys[4];
	polygon_t		*front, *back;

	world_scrap_t		*head;
	world_scrap_t		*scrap;

	head = NULL;

	GetProjectionVecs( right, up, f->projection );

	if ( !poly )
		Error( "(null) polygon in patch\n" );
	
	Vec2dInitBB( min, max, 999999.9 );
	
	for ( i = 0; i < poly->pointnum; i++ )
	{
		ProjectVec3d( v, poly->p[i], f->projection );
		Vec2dAddToBB( min, max, v );
	}
	
	// world patch x,y offset
	xofs = (int) floor( min[0]/f->patchsize );
	yofs = (int) floor( min[1]/f->patchsize );
	
	split_pos[0] = (fp_t)(xofs) * f->patchsize + f->patchsize;
	split_pos[1] = (fp_t)(yofs) * f->patchsize + f->patchsize;
	
	for ( i = 0; i < 4; i++ )
		polys[i] = NULL;
	
	SplitPolygon( poly, right, split_pos[0], &front, &back );

	// 0: front-front
	// 1: front-back
	if ( front )
	{
		SplitPolygon( front, up, split_pos[1], &polys[0], &polys[1] );	
		FreePolygon( front );
	}
	
	// 2: back-front
	// 3: back-back
	if ( back )
	{
		SplitPolygon( back, up, split_pos[1], &polys[2], &polys[3] );	
		FreePolygon( back );
	}
	
	if ( polys[0] )
	{
//		Balance_AddWorldPatchScrap( PolygonArea( polys[0] ), f->pl, xofs+1, yofs+1, f->patchsize, p->color, p->spec );
		scrap = NEWTYPE( world_scrap_t );
		scrap->x = xofs+1;
		scrap->y = yofs+1;
		scrap->area =  PolygonArea( polys[0] );
		scrap->next = head;
		head = scrap;
		FreePolygon( polys[0] );
	}
	
	if ( polys[1] )
	{
//		Balance_AddWorldPatchScrap( PolygonArea( polys[1] ), f->pl, xofs+1, yofs, f->patchsize, p->color, p->spec );
		scrap = NEWTYPE( world_scrap_t );
		scrap->x = xofs+1;
		scrap->y = yofs;
		scrap->area =  PolygonArea( polys[1] );
		scrap->next = head;
		head = scrap;
		FreePolygon( polys[1] );
	}
	
	if ( polys[2] )
	{
//		Balance_AddWorldPatchScrap( PolygonArea( polys[2] ), f->pl, xofs, yofs+1, f->patchsize, p->color, p->spec );
		scrap = NEWTYPE( world_scrap_t );
		scrap->x = xofs;
		scrap->y = yofs+1;
		scrap->area =  PolygonArea( polys[2] );
		scrap->next = head;
		head = scrap;
		FreePolygon( polys[2] );
	}
	
	if ( polys[3] )
	{
//		Balance_AddWorldPatchScrap( PolygonArea( polys[3] ), f->pl, xofs, yofs, f->patchsize, p->color, p->spec );
		scrap = NEWTYPE( world_scrap_t );
		scrap->x = xofs;
		scrap->y = yofs;
		scrap->area =  PolygonArea( polys[3] );
		scrap->next = head;
		head = scrap;
		FreePolygon( polys[3] );
	}			              

	p->scrap_head = head;
}

void BuildFacePatches( face_t *f )
{
	vec3d_t		right, up;
	int		i;
	fp_t		x2, y2;

	// four clip planes
	vec3d_t		norms2[4];
	fp_t		dists2[4];
	polygon_t	*trace_poly, *light_poly;
	patch_t		*patch;
	vec3d_t		center;

	GetProjectionVecs( right, up, f->projection );

	// chop real patches
	
	for ( x2 = f->min2d[0]; x2 < f->max2d[0]; x2+=f->patchsize )
	{
		Vec3dFlip( norms2[0], right );
		Vec3dCopy( norms2[1], right );
		dists2[0] = -x2;
		dists2[1] = x2+f->patchsize;	
		
		for ( y2 = f->min2d[1]; y2 < f->max2d[1]; y2+=f->patchsize )
		{
			Vec3dFlip( norms2[2], up );
			Vec3dCopy( norms2[3], up );
			dists2[2] = -y2;
			dists2[3] = y2+f->patchsize;
			
			trace_poly = CopyPolygon( f->p );
			for ( i = 0; i < 4; i++ )
			{
				ClipPolygonInPlace( &trace_poly, norms2[i], dists2[i] );
				if ( !trace_poly )
					break;
			}
			if ( !trace_poly )
			{
				continue;
			}
		
			patch = NewPatch();
//			patch->p = trace_poly;
			patch->x = 0;
			patch->y = 0;
			Vec3dCopy( patch->norm, f->pl->norm );
			
			PolygonCenter( trace_poly, center );

			Vec3dMA( patch->trace_origin, 1.0, patch->norm, center );
			Vec3dMA( patch->light_origin, 1.0, patch->norm, center );
		    			
			patch->next = f->patches;
			f->patches = patch;		       
			
			patch_num++;
			
			BuildWorldScraps( f, patch, trace_poly );

			FreePolygon( trace_poly );
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
	if ( big < 16*126 )
	{
		f->patchsize = 16.0;
		face16_num++;
	}
	// patch size 32, 32*127 up to edge length 4064
	else if ( big < 32*126 )
	{
		f->patchsize = 32.0;
		face32_num++;
	}
	// patch size 64, 64*127 up to edge length 8128
	else if ( big < 64*126 )
	{
		f->patchsize = 64.0;
		face64_num++;
	}
	else if ( big < 128*126 )
	{
		f->patchsize = 128.0;
	}
	else
	{
		Error( "ClassifyFace: face is too big.\n" );
	}
//	f->patchsize = 64.0;


	{
		vec2d_t		wmin, wmax;

#if 0
		wmin[0] = min[0] - f->patchsize/2.0;
		wmin[1] = min[1] - f->patchsize/2.0;
		wmax[0] = max[0] + f->patchsize/2.0;
		wmax[1] = max[1] + f->patchsize/2.0;
#else
		wmin[0] = floor(min[0]/f->patchsize)*f->patchsize - f->patchsize/2.0;
		wmin[1] = floor(min[1]/f->patchsize)*f->patchsize - f->patchsize/2.0;
		wmax[0] = ceil(max[0]/f->patchsize)*f->patchsize + f->patchsize/2.0;
		wmax[1] = ceil(max[1]/f->patchsize)*f->patchsize + f->patchsize/2.0;
#endif

#if 0
		printf( "(%.2f,%.2f)-(%.2f,%.2f) -> (%.2f,%.2f)-(%.2f,%.2f): (%.2f,%.2f)\n", 
			min[0], min[1], max[0], max[1],
			wmin[0], wmin[1], wmax[0], wmax[1], 
			wmax[0]-wmin[0], wmax[1]-wmin[1]);
#endif

		Vec2dCopy( f->wmin, wmin );
		Vec2dCopy( f->wmax, wmax );
	}
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

	Balance_Dump();
}

/*
  ==================================================
  init patches

  ==================================================
*/

void PatchInit( patch_t *p )
{
//	Vec3dInit( p->color, 0, 0, 0 );
//	Vec3dInit( p->spec, 0, 0, 0 );

	p->intens_diffuse = 0.0;
	p->intens_specular = 0.0;
}

void PatchListInit( patch_t *p_head )
{
	patch_t		*p;

	for ( p = p_head; p ; p=p->next )
	{
		PatchInit( p );
	}
}

void FaceListInitAllPatches( u_list_t *face_list )
{
	u_list_iter_t		iter;
	face_t			*f;

	U_ListIterInit( &iter, face_list );
	for ( ; ( f = U_ListIterNext( &iter ) ) ; )
	{
		PatchListInit( f->patches );
	}	
}

void CSurfListInitAllPatches( u_list_t *csurf_list )
{
	u_list_iter_t		iter;
	bsurface_t		*cs;

	U_ListIterInit( &iter, csurf_list );
	for ( ; ( cs = U_ListIterNext( &iter ) ) ; )
	{
		PatchListInit( cs->patches );	 
	}
}
