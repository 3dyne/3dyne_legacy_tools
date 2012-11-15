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



// pvs.c

#include "light.h"

listnode_t * NewListNode( void )
{
	return NEW( listnode_t );
}

void FreeListNode( listnode_t *n )
{
	free( n );
}

#define MAX_MAPNODES		( 8192 )

int		r_mapnodenum;
mapnode_t	r_mapnodes[MAX_MAPNODES];
int		r_visleafnum;
int		r_leafbitpos[MAX_MAPNODES];

void PVS_CompileMapnodeClass( hmanager_t *mapnodehm, hmanager_t *planehm, hmanager_t *brushhm, face_t *facelist )
{
	int		i;
	hmanager_type_iterator_t	iters[3];
	hpair_search_iterator_t		iter;
	hobj_t		*mapnode;
	hobj_t		*brush, *leaf, *plane, *child;
	hpair_t		*pair;
	int		index;
	int		bitpos;
	int		see_buffer_size;

	face_t		*f;

	printf( "setup pvs ...\n" );

//	r_leafrefnum = 0;
//	r_brushrefnum = 0;	
	r_mapnodenum = 0;
	r_visleafnum = 0;

	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnodes" );
	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnode_front" );
	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnode_back" );

	printf( "r_mapnodenum: %d\n", r_mapnodenum );

	HManagerInitTypeSearchIterator( &iters[0], mapnodehm, "mapnodes" );
	HManagerInitTypeSearchIterator( &iters[1], mapnodehm, "mapnode_front" );
	HManagerInitTypeSearchIterator( &iters[2], mapnodehm, "mapnode_back" );

	for ( i = 0; i < 3; i++ )
	{
		for ( ; ( mapnode = HManagerGetNextClass( &iters[i] ) ); )
		{
			pair = FindHPair( mapnode, "index" );
			if ( !pair )
				Error( "missing 'index' in mapnode '%s'.\n", mapnode->name );
			HPairCastToInt_safe( &index, pair );

			r_mapnodes[index].self = mapnode;
			r_mapnodes[index].faces = NULL;

			pair = FindHPair( mapnode, "plane" );
			if ( pair )
			{
				// it's a node
				plane = HManagerSearchClassName( planehm, pair->value );
				if ( !plane )
					Error( "mapnode '%s' can't find plane '%s'.\n", mapnode->name, pair->value );
				r_mapnodes[index].pl = GetClassExtra( plane );

				child = FindClassType( mapnode, "mapnode_front" );
				if ( !child )
					Error( "missing 'mapnode_front' in mapnode '%s'.\n", mapnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "mapnode '%s' missing 'index' of child '%s'.\n", mapnode->name, child->name );
				HPairCastToInt( &r_mapnodes[index].child[0], pair );


				child = FindClassType( mapnode, "mapnode_back" );
				if ( !child )
					Error( "missing 'mapnode_back' in mapnode '%s'.\n", mapnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "mapnode '%s' missing 'index' of child '%s'.\n", mapnode->name, child->name );
				HPairCastToInt( &r_mapnodes[index].child[1], pair );				
			}
			else
			{
				// it's a leaf
				r_mapnodes[index].visleaf = -1;	// for debug draw
				r_mapnodes[index].pl = NULL;
				r_mapnodes[index].child[0] = -1;
				r_mapnodes[index].child[1] = -1;

				// get brushes
				InitHPairSearchIterator( &iter, mapnode, "brush" );
				for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
				{
					int		brushindex;
					
					brush = HManagerSearchClassName( brushhm, pair->value );
					if ( !brush )
						Error( "mapnode '%s' can't find brush '%s'.\n", mapnode->name, pair->value );
					pair = NewHPair2( "ref", "mapnode", mapnode->name );
					InsertHPair( brush, pair );
				}
				
				// get pvs
				pair = FindHPair( mapnode, "bitpos" );
				if ( pair )
				{
					HPairCastToInt( &bitpos, pair );
					r_leafbitpos[bitpos] = index;

					pair = FindHPair( mapnode, "complex_see" );
					if ( !pair )
					{
						pair = FindHPair( mapnode, "trivial_see" );
						if ( !pair )
							Error( "missing 'trivial_see'.\n" );
					
					}
					see_buffer_size = 4096;
					HPairCastToBstring_safe( r_mapnodes[index].can_see, &see_buffer_size, pair );

					r_visleafnum++;
					r_mapnodes[index].visinfo = true;

				// get center, min, max
					pair = FindHPair( mapnode, "center" );
					if ( !pair )
						Error( "missing 'center' in mapnode '%s'.\n", mapnode->name );
					HPairCastToVec3d_safe( r_mapnodes[index].center, pair );
					
				}
				else
				{
					r_mapnodes[index].visinfo = false;
				}
			}
		}
	}

	//
	// sort faces into mapnodes
	//

	for ( f = facelist; f ; f=f->next ) 
	{
		hpair_t		*pair;
		int		index;
		hobj_t		*mapnode;

		pair = FindHPair( f->brush, "mapnode" );
		if ( !pair )
			Error( "brush '%s' missing 'mapnode'\n", f->brush->name );
		
		mapnode = HManagerSearchClassName( mapnodehm, pair->value );
		if ( !mapnode )
			Error( "brush can't find mapnode\n" );
		pair = FindHPair( mapnode, "index" );
		if ( !pair ) 
			Error( "brush missing 'index'\n" );
		HPairCastToInt_safe( &index, pair );

		{
			listnode_t	*n;
			n = NewListNode();
			n->face = f;
			n->next = r_mapnodes[index].faces;
			r_mapnodes[index].faces = n;
		}		
	}

	//
	// collect faces in visleafs
	//

	for ( i = 0; i < r_mapnodenum; i++ )
	{
		mapnode_t		*mn;
		mn = &r_mapnodes[i];

		pair = FindHPair( mn->self, "plane" );
		if ( !pair )
		{
			// it's a leaf

			InitHPairSearchIterator( &iter, mn->self, "touchleaf" );
			for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
			{
				hobj_t		*leaf;
				leaf = HManagerSearchClassName( mapnodehm, pair->value );
				if ( !leaf )
					Error( "mapnode '%s' can't find mapnode '%s'.\n", mn->self->name, pair->value );
				
				pair = FindHPair( leaf, "index" );
				if ( !pair )
					Error( "missing 'index' in mapnode '%s'.\n", leaf->name );
				HPairCastToInt_safe( &index, pair );
				
				// copy facelist to mapnode
				{
					listnode_t	*n;

					for ( n = r_mapnodes[index].faces; n ; n=n->next )
					{
						listnode_t		*nn;
						nn = NewListNode();
						nn->face = n->face;
						nn->next = mn->faces;
						mn->faces = nn;
					}					
				}	
			}
		}
	}
}
	
mapnode_t * PVS_FindMapnode( vec3d_t point )
{
	int		node;
	mapnode_t	*n;
	fp_t		d;
	
	node = 0;
	
	for(;;)
	{
		n = &r_mapnodes[node];
		if ( !n->pl )
		{
			return n;
		}
		
		d = Vec3dDotProduct( n->pl->norm, point ) - n->pl->dist;
		if ( d >= 0 )
		{
			// go to front child
			node = n->child[0];
		}
		else
		{
			// go to back child
			node = n->child[1];
		}
	}	
}

