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



// vertex_bsp.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_error.h"
#include "lib_math.h"
#include "lib_mem.h"
#include "lib_hobj.h"
#include "lib_container.h"
#include "cmdpars.h"

typedef struct vertex_s
{
	vec3d_t		vec;
} vertex_t;

typedef struct node_s
{
	vec3d_t		norm;
	fp_t		dist;
	struct node_s	*child[2];

	// leaf
	u_list_t	*vertex_list;

	// for index
	int		index;
} node_t;

u_list_t	g_node_list;

void BSP_TestVertex( vertex_t *vert, vec3d_t norm, fp_t dist, int *num_front, int *num_back, fp_t *sum_dist )
{
	fp_t		d;

	d = Vec3dDotProduct( vert->vec, norm ) - dist;

	if ( d >= 0.0 )
	{
		(*num_front)++;
	}
	else
	{
		(*num_back)++;
	}

	(*sum_dist) += d;
}

bool_t BSP_SelectBestSplitPlane( u_list_t *vertex_list, vec3d_t best_norm, fp_t *best_dist )
{
	vec3d_t		plane_norms[3] = { { 1, 0, 0 }, 
					   { 0, 1, 0 },
					   { 0, 0, 1 } };
	u_list_iter_t	iter1, iter2;
	int			i;

	vec3d_t		norm;
	fp_t		dist;

	int		num_front, num_back, num_on;
	fp_t		sum_dist;

	vertex_t	*vert1, *vert2;

	vertex_t		*best_vert;
	int			best_value, value;

	best_value = 2000000000;
	best_vert = NULL;

	if ( U_ListLength( vertex_list ) <= 256 )
		return false;

	U_ListIterInit( &iter1, vertex_list );
	for ( ; ( vert1 = U_ListIterNext( &iter1 ) ) ; )
	{
		for ( i = 0; i < 3; i++ )
		{
			Vec3dCopy( norm, plane_norms[i] );
			dist = Vec3dDotProduct( vert1->vec, norm );

			//
			// sort vertices with split plane
			//

			num_front = 0;
			num_back = 0;
			num_on = 0;
			sum_dist = 0.0;

			U_ListIterInit( &iter2, vertex_list );
			for ( ; ( vert2 = U_ListIterNext( &iter2 ) ) ; )
			{
				BSP_TestVertex( vert2, norm, dist, &num_front, &num_back, &sum_dist );				
			}

			value = abs(num_front-num_back) + (int)fabs((sum_dist));

//			printf( " num_front %d, num_back %d, sum_dist %f, value %d\n", num_front, num_back, sum_dist, value );

			if ( value < best_value )
			{
				best_value = value;
				best_vert = vert1;
				Vec3dCopy( best_norm, norm );
				*best_dist = dist;
			}

		}
	}

	if ( best_vert )
		return true;
	else
		return false;
}

void BSP_SplitVertexList( u_list_t *vertex_list, vec3d_t norm, fp_t dist, u_list_t **front_list, u_list_t **back_list )
{
	u_list_iter_t		iter;
	fp_t			d;
	vertex_t		*vert;

	*front_list = U_NewList();
	*back_list = U_NewList();

	U_ListIterInit( &iter, vertex_list );
	for ( ; ( vert = U_ListIterNext( &iter ) ) ; )
	{
		U_ListIterRemoveGoPrev( &iter );
		
		d = Vec3dDotProduct( vert->vec, norm ) - dist;

		if ( d >= 0.0 )
		{
			// front and on goes to front_list
			U_ListInsertAtHead( *front_list, vert );
		}
		else
		{
			// back goes to back_list
			U_ListInsertAtHead( *back_list, vert );
		}
	}
}

node_t * BSP_MakeTreeRecursive( u_list_t *vertex_list )
{
	vec3d_t		best_norm;
	fp_t		best_dist;

	node_t		*node;
	
	u_list_t		*front_list;
	u_list_t		*back_list;

	node = NEWTYPE( node_t );
	U_ListInsertAtTail( &g_node_list, node );

	if ( BSP_SelectBestSplitPlane( vertex_list, best_norm, &best_dist ) )
	{
		// found a split plane: it's a node

		Vec3dCopy( node->norm, best_norm );
		node->dist = best_dist;

		BSP_SplitVertexList( vertex_list, best_norm, best_dist, &front_list, &back_list );
		// fixme: free input list

		if ( U_ListLength( front_list ) )
		{
			node->child[0] = BSP_MakeTreeRecursive( front_list );
		}
		else
		{
			// fixme: free empty list
		}
		if ( U_ListLength( back_list ) )
		{
			node->child[1] = BSP_MakeTreeRecursive( back_list );
		}
		else
		{
			// fixme: free empty list
		}
	}
	else
	{
		// no split plane: it's a leaf
		node->vertex_list = vertex_list;
//		num_in_leafs += U_ListLength( vertex_list );
//		printf( "leaf: %d vertices ( %d )\n", U_ListLength( vertex_list ), num_in_leafs );
	}

	return node;
}

u_list_t * BuildVertexListFromClass( char *name )
{
	tokenstream_t		*ts;
	hobj_t			*root;
	u_list_t		*list;
	hobj_search_iterator_t	iter;
	hobj_t			*vertex;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "can't open vertex class\n" );
	root = ReadClass( ts );
	EndTokenStream( ts );

	list = U_NewList();

	InitClassSearchIterator( &iter, root, "vertex" );
	for ( ; ( vertex = SearchGetNextClass( &iter ) ) ; )
	{
		vertex_t	*tmp;
		
		tmp = NEWTYPE( vertex_t );
		U_ListInsertAtHead( list, tmp );
		
		EasyFindVec3d( tmp->vec, vertex, "vec" );
	}

	return list;
}

void BuildClassFromNodeList( u_list_t *node_list, char *name )
{
	int		i;
	
	u_list_iter_t	iter;
	node_t		*n;

	hobj_t		*node;
	hobj_t		*root;

	FILE		*h;

	// indexing nodes	
	U_ListIterInit( &iter, node_list );
	for ( i = 0; ( n = U_ListIterNext( &iter ) ) ; i++ )
	{
		n->index = i;
	}

	root = NewClass( "bspnodes", "bspnodes0" );

	// build class
	U_ListIterInit( &iter, node_list );
	for ( ; ( n = U_ListIterNext( &iter ) ) ; )
	{
		node = EasyNewClass( "bspnode" );
		InsertClass( root, node );

		EasyNewInt( node, "index", n->index );
		
		if ( n->vertex_list )
		{
			// leaf
			EasyNewString( node, "type", "leaf" );
		}
		else
		{
			EasyNewString( node, "type", "node" );
			EasyNewVec3d( node, "norm", n->norm );
			EasyNewFloat( node, "dist", n->dist );
			EasyNewInt( node, "child_front", n->child[0]->index );
			EasyNewInt( node, "child_back", n->child[1]->index );
		}						
	}

	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open node class\n" );
	WriteClass( root, h );
	fclose( h );
}

int main( int argc, char *argv[] )
{
	char	*in_vertex_name;
	char	*out_node_name;

	u_list_t	*vertex_list;

	node_t		*top_node;

	puts( "===== vertex_bsp =====" );
	
	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -iv filname: input vertex class" );
		puts( " -on filname: output node class" );
		exit(-1);
	}

	SetCmdArgs( argc, argv );

	in_vertex_name = GetCmdOpt2( "-iv" );
	out_node_name = GetCmdOpt2( "-on" );

	if ( !in_vertex_name )
		Error( "no input vertex class\n" );

	if ( !out_node_name )
		Error( "no output node class\n" );
	
	vertex_list = BuildVertexListFromClass( in_vertex_name );
	printf( " %d input vertices\n", U_ListLength( vertex_list ) );

	U_InitList( &g_node_list );
	top_node = BSP_MakeTreeRecursive( vertex_list );
	BuildClassFromNodeList( &g_node_list, out_node_name );

	HManagerSaveID();
	exit(0);
}
