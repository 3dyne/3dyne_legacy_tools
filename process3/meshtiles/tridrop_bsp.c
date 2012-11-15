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



// tridrop_bsp.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_mem.h"
#include "lib_hobj.h"
#include "lib_container.h"
#include "cmdpars.h"

typedef struct node_s
{
	vec3d_t		norm;
	fp_t		dist;
	
	int		child[2];
} node_t;

#define MAX_NUM_NODES		( 1024*16 )

int	g_num_node;
int	g_num_leaf;
node_t g_nodes[MAX_NUM_NODES];

fp_t	g_best_area;
node_t	*g_best_leaf;

void DropPolygonRecursive( node_t *n, polygon_t *p )
{
	if ( n->child[0] == -1 && n->child[1] == -1 )
	{
		fp_t	area;
		// leaf
		
		area = PolygonArea( p );
//		printf( " split: %f\n", area );
		if ( area > g_best_area )
		{
			g_best_area = area;
			g_best_leaf = n;
		}
	}
	else
	{
		polygon_t	*front;
		polygon_t	*back;
		// node
	       
		SplitPolygon( p, n->norm, n->dist, &front, &back );

		if ( front )
		{
			DropPolygonRecursive( &g_nodes[n->child[0]], front );	
			FreePolygon( front );
		}
		if ( back )
		{
			DropPolygonRecursive( &g_nodes[n->child[1]], back );
			FreePolygon( back );
		}
	}
}

void FindLeafForPolygon( polygon_t *p )
{
	g_best_area = -1.0;
	g_best_leaf = NULL;

	DropPolygonRecursive( &g_nodes[0], p );

	if ( !g_best_leaf )
		Error( "no leaf found\n" );

	printf( " %.2f%% of polygon area in one leaf\n", 100.0*g_best_area/PolygonArea(p) );
}

void BuildTreeFromClass( char *name )
{
	tokenstream_t		*ts;
	hobj_search_iterator_t	iter;
	hobj_t			*root;
	hobj_t			*node;
	hpair_t			*pair;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "can't open node class\n" );
	root = ReadClass( ts );
	EndTokenStream( ts );

	g_num_node = 0;

	InitClassSearchIterator( &iter, root, "bspnode" );
	for ( ; ( node = SearchGetNextClass( &iter ) ) ; )
	{
		int		index;

		EasyFindInt( &index, node, "index" );
		
		if ( index >= MAX_NUM_NODES )
			Error( "reached MAX_NUM_NODES\n" );

		pair = FindHPair( node, "type" );
		if ( !pair )
			Error( "missing key 'type' in bspnode\n" );

		if ( !strcmp( pair->value, "node" ) )
		{
			// node
			EasyFindVec3d( g_nodes[index].norm, node, "norm" );
			EasyFindFloat( &g_nodes[index].dist, node, "dist" );

//			printf( "dist: %f, norm: ", g_nodes[index].dist ); Vec3dPrint( g_nodes[index].norm );
			
			EasyFindInt( &g_nodes[index].child[0], node, "child_front" );
			EasyFindInt( &g_nodes[index].child[1], node, "child_back" );

			g_num_node++;
		}
		else
		{
			// leaf
			g_nodes[index].child[0] = -1;
			g_nodes[index].child[1] = -1;

			g_num_leaf++;
		}
	}
}

u_list_t * BuildPolygonListFromClass( char *name )
{
	tokenstream_t		*ts;
	hobj_t			*root;
	hobj_t			*polygon;
	hobj_search_iterator_t	iter;
	u_list_t		*list;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "can't open polygon class\n" );
	root = ReadClass( ts );
	EndTokenStream( ts );

	list = U_NewList();

	InitClassSearchIterator( &iter, root, "polygon" );
	for ( ; ( polygon = SearchGetNextClass( &iter ) ) ; )
	{
		polygon_t	*p;

		p = CreatePolygonFromClass( polygon );
		U_ListInsertAtHead( list, p );
	}
	
	return list;
}

int main( int argc, char *argv[] )
{
	char		*in_node_name;
	char		*in_polygon_name;

	u_list_t		*polygon_list;

	puts( "===== tridrop_bsp =====" );
	
	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -in filename: input node class" );
		puts( " -ip filename: input polygon class" );
		exit(-1);
	}

	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-in" );
	in_polygon_name = GetCmdOpt2( "-ip" );

	if ( !in_node_name )
		Error( "no input node class\n" );
	if ( !in_polygon_name )
		Error( "no input polygon class\n" );
	
	
	BuildTreeFromClass( in_node_name );
	printf( " %d nodes, %d leafs\n", g_num_node, g_num_leaf );

	polygon_list = BuildPolygonListFromClass( in_polygon_name );
	printf( " %d polygons\n", U_ListLength( polygon_list ) );
	
	{
		u_list_iter_t	iter;
		polygon_t	*p;

		U_ListIterInit( &iter, polygon_list );
		for ( ; ( p = U_ListIterNext( &iter ) ) ; )
		{
			FindLeafForPolygon( p );
		}
	}

	exit(0);
}
