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



// mapnode.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#include "defs.h"

#include "../csg/cbspbrush.c"

void InsertBrushesRecursive( hobj_t *mapnode, hobj_t *bspnode )
{
	hpair_t		*pair;
	hpair_search_iterator_t	iter;
	hobj_t		*bspchild;

	InitHPairSearchIterator( &iter, bspnode, "brush" );
	for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
	{
		pair = NewHPair2( pair->type, pair->key, pair->value );
		InsertHPair( mapnode, pair );
	}

	pair = FindHPair( bspnode, "plane" );
	if ( pair )
	{
		// it's a node
	     
		bspchild = FindClassType( bspnode, "bspnode_front" );
		if ( !bspchild )
			Error( "missing 'bspnode_front' in node '%s'.\n", bspnode->name );
		InsertBrushesRecursive( mapnode, bspchild );

		bspchild = FindClassType( bspnode, "bspnode_back" );
		if ( !bspchild )
			Error( "missing 'bspnode_back' in node '%s'.\n", bspnode->name );
		InsertBrushesRecursive( mapnode, bspchild );
	}
	else
	{
		// it's a leaf		
	}	
}

void BuildMapNodesRecursive( hobj_t *mapnode, hobj_t *bspnode, hmanager_t *visleafhm )
{
	hpair_t		*pair;
	hpair_search_iterator_t	iter;
	hobj_t		*visleaf;
	hobj_t		*mapchild;
	hobj_t		*bspchild;
	char		tt[256];
	
	pair = FindHPair( bspnode, "portalized_leaf" );
	if ( pair )
	{
		// is it a visleaf ?
		pair = FindHPair( bspnode, "visleaf" );
		if ( pair )
		{
			// it's a vis leaf
			visleaf = HManagerSearchClassName( visleafhm, pair->value );
			if ( !visleaf )
				Error( "node '%s' can't find visleaf '%s'.\n", bspnode->name, pair->value );
			
			// copy bitpos
			pair = FindHPair( visleaf, "bitpos" );
			if ( !pair )
				Error( "missing bitpos.\n" );
			pair = NewHPair2( pair->type, pair->key, pair->value );
			InsertHPair( mapnode, pair );
			
			// copy trivial_see
			pair = FindHPair( visleaf, "trivial_see" );
			if ( pair )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( mapnode, pair );
			}

			// copy complex_see
			pair = FindHPair( visleaf, "complex_see" );
			if ( pair )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( mapnode, pair );
			}

			// copy touchleafs
			InitHPairSearchIterator( &iter, visleaf, "touchleaf" );
			for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( mapnode, pair );
			}

			// get brushes in the sub-tree recursive
			InsertBrushesRecursive( mapnode, bspnode );
		}
		else
		{
			// it's a solid leaf
			// copy brushes
			InitHPairSearchIterator( &iter, bspnode, "brush" );
			for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( mapnode, pair );
			}
		}
	       

	}
	else
	{
		// it's a node
		
		// copy plane
		pair = FindHPair( bspnode, "plane" );
		if ( !pair )
			Error( "node '%s' can't find plane '%s'.\n", bspnode->name, pair->value );
		pair = NewHPair2( pair->type, pair->key, pair->value );
		InsertHPair( mapnode, pair );

		// copy children
		bspchild = FindClassType( bspnode, "bspnode_front" );
		if ( !bspchild )
			Error( "missing 'bspnode_front' in node '%s'.\n", bspnode->name );
//		sprintf( tt, "#%u", HManagerGetFreeID() );
		mapchild = NewClass( "mapnode_front", bspchild->name );
		BuildMapNodesRecursive( mapchild, bspchild, visleafhm );
		InsertClass( mapnode, mapchild );

		bspchild = FindClassType( bspnode, "bspnode_back" );
		if ( !bspchild )
			Error( "missing 'bspnode_back' in node '%s'.\n", bspnode->name );
//		sprintf( tt, "#%u", HManagerGetFreeID() );
		mapchild = NewClass( "mapnode_back", bspchild->name );
		BuildMapNodesRecursive( mapchild, bspchild, visleafhm );
		InsertClass( mapnode, mapchild );
	}
}

int main( int argc, char *argv[] )
{
	char	*in_node_name;
	char	*in_visleaf_name;
	char	*out_mapnode_name;

	hmanager_t	*nodehm;
	hmanager_t	*visleafhm;

	hobj_t		*mapnode;

	tokenstream_t		*ts;
	FILE			*h;

	printf( "===== mapnode - build a customized mapnode class =====\n" );
	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-n" );
	in_visleaf_name = GetCmdOpt2( "-v" );
	out_mapnode_name = GetCmdOpt2( "-o" );

	if ( !in_node_name )
	{
		Error( "no input node class.\n" );
	}
	else
	{
		printf( " input node class: %s\n", in_node_name );
	}

	if ( !in_visleaf_name )
	{
		in_visleaf_name = "_pvsout_visleaf.hobj";
		printf( " default input visleaf class: %s\n", in_visleaf_name );
	}
	else
	{
		printf( " input visleaf class: %s\n", in_visleaf_name );
	}

	if ( !out_mapnode_name )
	{
		out_mapnode_name = "_mapnode.hobj";
		printf( " default output mapnode class: %s\n", out_mapnode_name );
	}
	else
	{
		printf( " output mapnode class: %s\n", out_mapnode_name );
	}

	printf( "load node class ...\n" );
	nodehm = NewHManager();
	ts = BeginTokenStream( in_node_name );
	if ( !ts )
		Error( "can't open file.\n" );
	HManagerSetRootClass( nodehm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( nodehm );

	printf( "load visleaf class ...\n" );
	visleafhm = NewHManager();
	ts = BeginTokenStream( in_visleaf_name );
	if ( !ts )
		Error( "can't open file.\n" );
	HManagerSetRootClass( visleafhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( visleafhm );
	

	mapnode = NewClass( "mapnodes", "topnode" );
	BuildMapNodesRecursive( mapnode, HManagerGetRootClass( nodehm ), visleafhm );


	printf( "write mapnode class ...\n" );
	h = fopen( out_mapnode_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( mapnode, h );
	fclose( h );

//	HManagerSaveID();
}


       
