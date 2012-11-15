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



// g_blockmap.c

#include "g_blockmap.h"
#include "interfaces.h"
#include "defs.h"
#include "g_mapdefs.h"

/*
  ==================================================
  Blockmap hash stuff

  ==================================================
*/

/*
  ==============================
  G_NewBlockmap

  ==============================
*/

blockmap_t * G_NewBlockmap( void )
{
	blockmap_t	*map;
	
	map = NEW( blockmap_t );
	return map;
}


/*
  ==============================
  G_FreeBlockmap

  ==============================
*/

void G_FreeBlockmap( blockmap_t *map )
{
	FREE( map );
}


/*
  ==============================
  G_CalcBlockmapHashkey

  ==============================
*/

int G_CalcBlockmapHashkey( ivec3d_t pos )
{
	int		key;

	key = pos[0] + (pos[1]<<3) + (pos[2]<<6);
	key &= (BLOCKMAP_HASHSIZE-1);
	
	return key;
}

/*
  ==============================
  G_FindMapblock

  ==============================
*/
mapblock_t * G_FindMapblock( blockmap_t *map, ivec3d_t pos )
{
	int		key;
	mapblock_t	*block;

	key = G_CalcBlockmapHashkey( pos );
	
	for ( block = map->hash[key]; block ; block=block->next )
	{
		if ( block->pos[0] == pos[0] &&
		     block->pos[1] == pos[1] &&
		     block->pos[2] == pos[2] )
		{
			return block;
		}
	}

	return NULL;
}


/*
  ==============================
  G_InsertMapblock

  ==============================
*/
void G_InsertMapblock( blockmap_t *map, mapblock_t *block )
{
	int		key;

	key = G_CalcBlockmapHashkey( block->pos );

	block->next = map->hash[key];
	map->hash[key] = block;

	map->mapblocknum++;
}


/*
  ==============================
  G_RemoveMapblock

  ==============================
*/
void G_RemoveMapblock( blockmap_t *map, mapblock_t *block )
{
	int		key;
	mapblock_t		*b, *bnext, *head;

	key = G_CalcBlockmapHashkey( block->pos );
	head = NULL;
	for ( b = map->hash[key]; b ; b=bnext )
	{
		bnext = b->next;
		if ( b == block )
		{
			map->mapblocknum--;
			continue;
		}
		b->next = head;
		head = b;
	}
	map->hash[key] = head;
}



/*
  ==================================================
  Blockmap linknode stuff

  ==================================================
*/

/*
  ==============================
  G_NewBlockmapLinknode

  ==============================
*/

static int	linknodenum = 0;

mapln_t * G_NewBlockmapLinknode( void )
{
	mapln_t		*n;

	n = NEW( mapln_t );
	linknodenum++;
	return n;
}

/*
  ==============================
  G_FreeBlockmapLinknode

  ==============================
*/

void G_FreeBlockmapLinknode( mapln_t *n )
{
	linknodenum--;
	FREE( n );
}

/*
  ==============================
  G_InsertLN

  ==============================
*/
void G_InsertLN( mapln_t *prev, mapln_t *next, mapln_t *insert )
{
	prev->nextb = insert;
	insert->prevb = prev;

	next->prevb = insert;
	insert->nextb = next;
}

/*
  ==============================
  G_RemoveLN

  ==============================
*/
void G_RemoveLN( mapln_t *n )
{
	n->prevb->nextb = n->nextb;
	n->nextb->prevb = n->prevb;
	n->prevb = NULL;
	n->nextb = NULL;
}



/*
  ==================================================
  Mapblock stuff

  ==================================================
*/

/*
  ==============================
  G_NewMapblock

  ==============================
*/
mapblock_t * G_NewMapblock( ivec3d_t pos )
{
	mapblock_t	*block;

	block = NEW( mapblock_t );
	
	IVec3dCopy( block->pos, pos );

	block->head.prevb = NULL;
	block->head.nextb = &block->tail;
	block->tail.nextb = NULL;
	block->tail.prevb = &block->head;

//	printf( "head.prevb %p, head.nextb %p, tail.nextb %p, tail.prevb %p\n", 
//		block->head.prevb, block->head.nextb, block->tail.nextb, block->tail.prevb );

	return block;
}

/*
  ==============================
  G_FreeMapblock

  ==============================
*/
void G_FreeMapblock( mapblock_t *block )
{
	FREE( block );
}

/*
  ==============================
  G_GiveMapblock

  ==============================
*/
mapblock_t * G_GiveMapblock( blockmap_t *map, ivec3d_t pos )
{
	mapblock_t	*block;

	block = G_FindMapblock( map, pos );
	
	if ( block )
		return block;

	// if block does not exist create it
	block = G_NewMapblock( pos );
	G_InsertMapblock( map, block );

	return block;
}

/*
  ==============================
  G_CalcBlockmapUnits

  ==============================
*/
void G_IVec3dToBlockmapUnits( ivec3d_t out, ivec3d_t in )
{
#if 0
	out[0] = (int)floor( 1.0*in[0]/MAPBLOCK_SIZE );      
	out[1] = (int)floor( 1.0*in[1]/MAPBLOCK_SIZE );
	out[2] = (int)floor( 1.0*in[2]/MAPBLOCK_SIZE );
#else
//	out[0] = in[0] / MAPBLOCK_SIZE;
//	out[1] = in[1] / MAPBLOCK_SIZE;
//	out[2] = in[2] / MAPBLOCK_SIZE;
	IVec3dUnitSnap( out, in, MAPBLOCK_SIZE );
#endif
}

/*
  ==============================
  G_InsertLNIntoMapblock

  ==============================
*/
void G_InsertLNIntoMapblock( mapblock_t *block, mapln_t *n )
{
//	printf( "prev: %p, next: %p\n", &block->head, block->head.nextb );
	G_InsertLN( &block->head, block->head.nextb, n );
}


/*
  ==================================================
  Init blockmap from a game map

  ==================================================
*/

#define MAX_PLANES_PER_BLOCKMAPBRUSH		( 128 )

void G_InitBlockmap( g_map_t *map )
{
	blockmap_t		*blockmap;
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	surfiter;
	hobj_search_iterator_t	polyiter;
	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*plane;
	hobj_t		*poly;
	hobj_t		*vertex;
	hpair_t		*pair;
	int		num, i;
	int			planenum;
	vec3d_t			norms[MAX_PLANES_PER_BLOCKMAPBRUSH];
	fp_t			dists[MAX_PLANES_PER_BLOCKMAPBRUSH];
	char		tt[256];

	int		mapobjectnum = 0;
	linknodenum = 0;

	printf( "G_InitBlockmap: " );

	blockmap = G_NewBlockmap();
	map->blockmap = blockmap;
		
	InitClassSearchIterator( &iter, HManagerGetRootClass( map->brushhm ), "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &iter ) ); )
	{
		int		planenum;
		mapobject_t	*mapobj;
		vec3d_t		min, max;

		// count surfaces with at least one polygon
		// and get bound box
		planenum = 0;
		Vec3dInitBB( min, max, 999999.9 );

		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( ; ( surface = SearchGetNextClass( &surfiter ) );  )	
		{
			bool_t		has_poly; 

			has_poly = false;
			InitClassSearchIterator( &polyiter, surface, "fixpolygon" );
			for ( ; ( poly = SearchGetNextClass( &polyiter ) );  )	
			{
				int		 pointnum;

				has_poly = true;
				pair = FindHPair( poly, "pointnum" );
				if ( !pair )
					Error( "missing 'pointnum' in fixpolygon '%s'.\n", poly->name );
				HPairCastToInt_safe( &pointnum, pair );
				
				// get vertexrefs
				for ( i = 0; i < pointnum; i++ )
				{
					vec3d_t		vec;

					sprintf( tt, "%d", i );
					pair = FindHPair( poly, tt );
					if ( !pair )
						Error( "missing vertex clsref '%s' in fixpolygon '%s'.\n", tt, poly->name );
					
					vertex = HManagerSearchClassName( map->vertexhm, pair->value );
					if ( !vertex )
						Error( "fixpolygon '%s' can't find vertex '%s'.\n", poly->name, pair->value );
					
					// get index of vertex
					pair = FindHPair( vertex, "point" );
					if ( !pair )
						Error( "missing 'point' in vertex '%s'.\n", vertex->name );
					HPairCastToVec3d_safe( vec, pair );
					Vec3dAddToBB( min, max, vec );
				}
			}

			if ( has_poly )
			{
				cplane_t		*pl;
				
				if ( planenum == MAX_PLANES_PER_BLOCKMAPBRUSH )
					Error( "reached MAX_PLANES_PER_BLOCKMAPBRUSH\n" );

				pair = FindHPair( surface, "plane" );
				if ( !pair )
					Error( "missing 'plane' in surface '%s'.\n", surface->name );
				plane = HManagerSearchClassName( map->planehm, pair->value );
				if ( !plane )
					Error( "surface '%s' can't find plane '%s'.\n", surface->name, pair->value );
				
				pl = GetClassExtra( plane );
				Vec3dCopy( norms[planenum], pl->norm );
				dists[planenum] = pl->dist;
				planenum++;
			}
		}

		if ( planenum == 0 )
			continue;

		mapobj = G_NewBlockmapObject( planenum );
		mapobjectnum++;
		IVec3dRint( mapobj->min, min );
		IVec3dRint( mapobj->max, max );	       

		// expand bound box, to a minimum size of 32 each axis
		// the minimum trace step is 16
		for ( i = 0; i < 3; i++ )
		{
			mapobj->min[i] -= 16;
			mapobj->max[i] += 16;
		}

//		IVec3dPrint( mapobj->min );
//		IVec3dPrint( mapobj->max );

		for ( i = 0; i < planenum; i++ )
		{
			Vec3dCopy( mapobj->planes[i].norm, norms[i] );
			mapobj->planes[i].dist = dists[i];
		}
		G_BlockmapLinkObject( blockmap, mapobj );		
	}

	printf( " %d mapblocks with %d mapobjects and %d mapobjectrefs\n", blockmap->mapblocknum, mapobjectnum, linknodenum );

}
