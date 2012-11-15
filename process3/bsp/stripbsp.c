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



// stripbsp.c

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

int		remove_num = 0;
int		keep_num = 0;

int StripBspnodeRecursive( hobj_t *node )
{
	hpair_t		*plane;

	plane = FindHPair( node, "plane" );
	
	if ( plane )
	{
		hobj_t		*front, *back;
		int		c1, c2;
		// it's a node
		
		front = FindClassType( node, "bspnode_front" );
		if ( !front )
			Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
		c1 = StripBspnodeRecursive( front );

		back = FindClassType( node, "bspnode_back" );
		if ( !back )
			Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
		c2 = StripBspnodeRecursive( back );

		if ( c1 == 0 && c2 == 0 )
		{
			hpair_t		*pair;
			hpair_search_iterator_t		iter;

			// children are both empty, remove them
			// and makes self empty too
			RemoveClass2( front );
			RemoveClass2( back );
			remove_num+=2;

			RemoveHPair( node, plane );
			
			pair = NewHPair2( "int", "contents", "0" );
			InsertHPair( node, pair );
			

			// get brushes of children
			InitHPairSearchIterator( &iter, front, "brush" );
			for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( node, pair );
			}

			InitHPairSearchIterator( &iter, back, "brush" );
			for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( node, pair );
			}

			return 0;
		}

		return 16;
	}
	else
	{
		hpair_t		*pair;
		int		contents;

		// it's a leaf		
		
		pair = FindHPair( node, "contents" );
		if ( !pair )
			Error( "missing 'contents' in node '%s'\n", node->name );
		HPairCastToInt_safe( &contents, pair );

		if ( contents == 0 )
			return 0;

		if ( contents < 16 )
		{
			// empty leaf or weak contents
			// remove

			sprintf( pair->value, "0" );

			return 0;
		}
		
		keep_num++;
		return contents;
	}
	
}

int main( int argc, char *argv[] )
{
	char		*in_node_name;
	char		*out_node_name;
	
	hmanager_t		*nodehm;
	FILE		*h;

	printf( "===== stripbsp - merge leafs =====\n" );
	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-i" );
	out_node_name = GetCmdOpt2( "-o" );

	if ( !in_node_name )
	{
		in_node_name = "_bspout_bspnode.hobj";
		printf( " default input node class: %s\n", in_node_name );
	}
	else
	{
		printf( " input node class: %s\n", in_node_name );
	}

	if ( !out_node_name )
	{
		out_node_name = "_stripbsp_bspnode.hobj";
		printf( " default output node class: %s\n", out_node_name );
	}
	else
	{
		printf( " output node class: %s\n", out_node_name );
	}

	nodehm = NewHManagerLoadClass( in_node_name );
	if ( !nodehm )
		Error( "can't open input node class.\n" );

	StripBspnodeRecursive( HManagerGetRootClass( nodehm ) );

	printf( " %d leafs removed\n", remove_num );
	printf( " %d leafs kept\n", keep_num );

	h = fopen( out_node_name, "w" );
	if ( !h )
		Error( "can't open output node class.\n" );
	WriteClass( HManagerGetRootClass( nodehm ), h );
	fclose( h );

	exit(0);
}
