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



// sky.c

#include "light.h"

static vec3d_t		sky_dir;
static vec3d_t		sky_color;

typedef struct lightflow_s
{
	rpatch_t		*flowto;
	fp_t		flow;
} lightflow_t;

#define MAX_LIGHT_FLOWS		( 1024*1024 )

static int		lightflownum;
static lightflow_t	lightflows[MAX_LIGHT_FLOWS];

bool_t HitSky( vec3d_t from )
{
	vec3d_t		to;
	int		splitnum;
	vec3d_t		split;
	node_t		*n;
	face_t		*f;
	int		i, k;

	// a point in the sky
	Vec3dMA( to, -32000.0, sky_dir, from );
	splitnum = TraceLine2( from, to );

	if ( splitnum == 0 )
		return false;

	n = NULL;
 	TraceLine2_GetNearestSplit( from, split, &n );

	if ( n->is_sky )
		return true;
	return false;
}

void SortFaces( face_t *list )
{
	face_t *f;
	
	// insert face into list of its plane
	printf( " sort faces ...\n" );

	for ( f = list; f ; f=f->next )
	{
		cplane_t	*pl;
		pl = f->pl;
		f->next2 = pl->facehead;
		pl->facehead = f;
	}
}


void MakePatchLightFlow( rpatch_t *pd, face_t *fd, face_t *list )
{
	face_t		*fg;
	rpatch_t		*pg;

	fp_t		totalscale;
	int		i;

	mapnode_t	*mapnode, *leaf;
	listnode_t	*n;
	int		totalpatchnum;

	static int	count = 1;

	totalscale = 0.0;
	lightflownum = 0;
	totalpatchnum = 0;

	mapnode = PVS_FindMapnode( pd->origin );
	if ( !mapnode->visinfo )
		Error( "no visinfo for patch\n" );

	for ( i = 0; i < r_visleafnum; i++ )
	{
		if ( mapnode->can_see[i>>3] & 1<<(i&7) )
		{
			leaf = &r_mapnodes[r_leafbitpos[i]];
		}
		else
			continue;
		
		for ( n = leaf->faces; n ; n=n->next )
		{
			fg = n->face;

			if ( fg->count == count )
				continue;
			fg->count++;
			
//			if ( fd == fg )
//				continue;
			if ( fg->mat.no_light || fg->mat.self_light )
				continue;
			
			for ( pg = fg->rpatches; pg ; pg=pg->next )
			{
				if ( pg == pd )
					continue;
				if ( pg->rad_distribute > pd->rad_distribute  )
					continue;

				if ( Vec3dDotProduct( fd->pl->norm, pg->origin ) - fd->pl->dist <= 0.0 )
					continue;
				
				if ( !TraceLine( pd->origin, pg->origin ) )
				{
					vec3d_t		vec;
					fp_t		d;
					fp_t		scale;
					Vec3dSub( vec, pd->origin, pg->origin );
					d = Vec3dLen( vec );
					scale = /*pg->area*/ 1.0 / (d*d);
					
					if ( lightflownum == MAX_LIGHT_FLOWS )
						Error( "reached MAX_LIGHT_FLOWS\n" );
					lightflows[lightflownum].flowto = pg;
					lightflows[lightflownum].flow = scale;
					lightflownum++;
					
					Vec3dAdd( pg->color, pg->color, pd->color );
					Vec3dScale( pg->color, 0.5, pg->color );
					
					totalscale += scale;
					totalpatchnum++;
				}
			}
		}
	}

//	printf( "%d ", lightflownum );
	for ( i = 0; i < lightflownum; i++ )
	{
		lightflows[i].flow = (lightflows[i].flow*(pd->rad_distribute*1.0))/totalscale;
		lightflows[i].flowto->rad_gather += lightflows[i].flow;
	}

	count++;

//	printf( "%d ", totalpatchnum );
//s	pd->rad_distribute = 0.0;

}


void FlowLight( face_t *list )
{
	face_t		*fd, *fg;
	rpatch_t		*pd, *pg;

	lightflownum = 0;

	for ( fd = list; fd ; fd=fd->next )
	{
		fprintf( stderr, "." );
		if ( fd->mat.no_light || fd->mat.self_light )
			continue;
		for ( pd = fd->rpatches; pd ; pd=pd->next )
		{
			if ( pd->rad_distribute == 0.0 )
				continue;
			MakePatchLightFlow( pd, fd, list );
		}
		
	}	
}

void SkyLight( face_t *list, vec3d_t dir, vec3d_t color )
{
	face_t		*f;
	patch_t		*p;
	rpatch_t		*rp;
	int		i;

	Vec3dCopy( sky_dir, dir );
	Vec3dUnify( sky_dir );
	Vec3dCopy( sky_color, color );

	printf( "sky light ...\n" );
//	SortFaces( list );

	for ( f = list; f ; f=f->next )
	{

		if ( f->mat.no_light || f->mat.self_light )
			continue;

		for ( p = f->patches; p ; p=p->next )
		{
			if ( HitSky( p->center ) )
			{
//				p->rad_distribute = 5.0;
//				p->rad_gather = 0.0;
				Vec3dAdd( p->color, p->color, sky_color );
				if ( p->color[0] > 1.0 )
					p->color[0] = 1.0;
				if ( p->color[1] > 1.0 )
					p->color[1] = 1.0;
				if ( p->color[2] > 1.0 )
					p->color[2] = 1.0;
			}
			else
			{
//				Vec3dInit( p->color, 0.3, 0.3, 0.6 );
//				p->rad_distribute = 0.0;
//				p->rad_gather = 0.0;
			}
		}
	}

//	return;

	//
	// init rpatches from patches
	//
	for ( f = list; f ; f=f->next )
	{
		for ( p = f->patches; p ; p=p->next )
		{
			if ( p->color[0] != 0.0 || p->color[1] != 0.0 || p->color[2] != 0.0 )
				p->rpatch->rad_distribute+=((16.0*16.0)/(128.0*128.0))*0.2;
//			Vec3dCopy( p->rpatch->color, p->rpatch->color, p->color );
//			Vec3dScale( p->rpatch->color, 0.5, p->rpatch->color );
		}

		for ( rp = f->rpatches; rp; rp=rp->next )
		{
			Vec3dCopy( rp->color, sky_color );
		}
	}


//	return;
	
	printf( "flow light ...\n" );
	for ( i = 0; i < 0; i++ )
	{
		printf( "%d\n", i );
		FlowLight( list );
		for ( f = list; f ; f=f->next )
		{
			for ( rp = f->rpatches; rp ; rp=rp->next )
			{
				rp->rad_distribute += rp->rad_gather;
				rp->rad_gather = 0.0;
				if ( rp->rad_peak < rp->rad_distribute )
					rp->rad_peak = rp->rad_distribute;
			}
		}
	}
	


	printf( "calc colors ...\n" );
	for ( f = list; f ; f=f->next )
	{
		for ( rp = f->rpatches; rp ; rp=rp->next )
		{
			if ( rp->rad_distribute > 0.0 )
			{
				if ( rp->rad_distribute < 1.0 )
					Vec3dScale( rp->color, rp->rad_distribute, rp->color );
			}
			else if ( rp->rad_distribute == 0.0 )
				Vec3dInit( rp->color, 0, 0, 0 );
		}

		for ( p = f->patches; p ; p=p->next )
		{
			fp_t		rb, b;
		
			rb = p->rpatch->color[0] + p->rpatch->color[1] + p->rpatch->color[1];
			rb /= 3;
			b = p->color[0] + p->color[1] + p->color[2];
			b /= 3;

			if ( b == 0.0 )
			{
				Vec3dCopy( p->color, p->rpatch->color );	
			}
#if 0
			Vec3dAdd( p->color, p->color, p->rpatch->color );
//			Vec3dScale( p->color, 0.5, p->color );
//			Vec3dCopy( p->color, p->rpatch->color );
				if ( p->color[0] > 1.0 )
					p->color[0] = 1.0;
				if ( p->color[1] > 1.0 )
					p->color[1] = 1.0;
				if ( p->color[2] > 1.0 )
					p->color[2] = 1.0;
#endif			
		}
	}

	//
	// copy color of rpatch to all its patches
	//

}
