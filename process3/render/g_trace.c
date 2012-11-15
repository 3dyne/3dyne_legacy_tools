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



// g_trace.c

#include "defs.h"
#include "interfaces.h"
#include "g_blockmap.h"
#include "g_trace.h"

#define TRACE_MAX_MAPOBJECTS		( 128 )

static int		 mobjnum;
static int		 testnum;
static mapobject_t	*mobjs[TRACE_MAX_MAPOBJECTS];

static ivec3d_t		 min, max;
static ivec3d_t		 trymin, trymax;

static ivec3d_t		 btrymin, btrymax;

static blockmap_t	*blockmap;

static int		tracecount = 0;

static g_trace_t	trace;

static void FindMapobjectIntersections( mapblock_t *block )
{
	mapln_t		*n;
	int		i;

	for ( n = block->head.nextb; n != &block->tail; n=n->nextb )
	{
		mapobject_t	*obj;

		obj = n->ref;

		if ( obj->lasttrace == tracecount )
			continue;
		obj->lasttrace = tracecount;
		
		testnum++;

		for ( i = 0; i < 3; i++ )
		{

			if ( obj->min[i] > trymax[i] ||
			     obj->max[i] < trymin[i] )
				break;
		}

		if ( i == 3 )
		{
			if ( mobjnum == TRACE_MAX_MAPOBJECTS )
				Error( "reached TRACE_MAX_MAPOBJECTS\n" );
			mobjs[mobjnum] = obj;
			mobjnum++;
		}
	}
}

static void FindMapblockIntersections( void )
{
	ivec3d_t	bpos;

	//
	// get all mapblocks, intersecting with btry bb
	//

	for ( bpos[0] = btrymin[0]; bpos[0] <= btrymax[0]; bpos[0]++ )
	{
		for ( bpos[1] = btrymin[1]; bpos[1] <= btrymax[1]; bpos[1]++ )
		{
			for ( bpos[2] = btrymin[2]; bpos[2] <= btrymax[2]; bpos[2]++ )
			{
				mapblock_t	*block;

				block = G_FindMapblock( blockmap, bpos );
				if ( !block )
				{
					printf( "FindIntersections: no mapblock for this position.\n" );
					continue;
				}
	
				FindMapobjectIntersections( block );
			}
		}
	}
}

void BoundBoxPlaneCheck( vec3d_t min, vec3d_t max, vec3d_t norm, fp_t dist, fp_t *dmin, fp_t *dmax )
{
	int		i;
	vec3d_t		bb[2];

	for ( i = 0; i < 3; i++ )
	{
		if ( norm[i] < 0.0 )
		{
			bb[0][i] = min[i];
			bb[1][i] = max[i];
		}
		else
		{
			bb[1][i] = min[i];
			bb[0][i] = max[i];
		}
	}
	
	*dmin = Vec3dDotProduct( bb[0], norm ) - dist;
	*dmax = Vec3dDotProduct( bb[1], norm ) - dist;
}

void TestMapobject( mapobject_t *obj, vec3d_t minf, vec3d_t maxf, vec3d_t mint, vec3d_t maxt )
{
	int		i;
	bmplane_t	*pl;
	fp_t		d1, d2;
	fp_t		t1, t2;
	fp_t		near;

	for ( i = 0; i < obj->planenum; i++ )
	{
		pl = &obj->planes[i];
		
		BoundBoxPlaneCheck( minf, maxf, pl->norm, pl->dist, &d1, &d2 );

		if ( d1 >= 0.0 && d2 >= 0.0 )
		{
			BoundBoxPlaneCheck( mint, maxt, pl->norm, pl->dist, &t1, &t2 );

			if ( t1 >= 0.0 && t2 >= 0.0 )
				continue;
			
			else if ( t1 < 0.0 && t2 < 0.0 )
				if ( t1 > t2 )
					near = t1;
				else
					near = t2;

			else if ( t1 >= 0.0 && t2 < 0.0 )
				near = t2;
			else if ( t2 >= 0.0 && t1 < 0.0 )
				near = t1;
			else
				Error( "TestMapobject: odd case\n" );

			if ( near < 0.0 )
			{
				if ( trace.nearest < near )
				{
					trace.nearest = near;
					trace.plane = pl;
				}
			}
		}		
	}
}

/*
  ==============================
  G_TraceBoundBox

  ==============================
*/

g_trace_t * G_TraceBoundBox( blockmap_t *map, vec3d_t minf, vec3d_t maxf, vec3d_t mint, vec3d_t maxt )
{
	int			i;

	tracecount++;
	blockmap = map;
	mobjnum = 0;
	testnum = 0;
	trace.nearest = -9999.9;
	trace.plane = NULL;

	IVec3dRint( trymin, mint );
	IVec3dRint( trymax, maxt );

	IVec3dPrint( trymin );
	IVec3dPrint( trymax );

	G_IVec3dToBlockmapUnits( btrymin, trymin );
	G_IVec3dToBlockmapUnits( btrymax, trymax );

	FindMapblockIntersections();
	
	for ( i = 0; i < mobjnum; i++ )
	{
		TestMapobject( mobjs[i], minf, maxf, mint, maxt );
	}

	printf( "G_TraceBoundBox: %d mapobjects tested, %d intersects\n", testnum, mobjnum  );

	return &trace;
}
