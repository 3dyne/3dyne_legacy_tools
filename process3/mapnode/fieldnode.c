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



// fieldnode.c

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

int	add_portal_num = 0;
int	add_leaf_num = 0;

void CalcFieldVectors( hmanager_t *nodehm, hobj_t *portals, hmanager_t *planehm )
{
	int		c1, c2;
	hobj_search_iterator_t	iter;
	hobj_t		*portal;
	hobj_t		*plane;
	hobj_t		*leaf;
	hpair_t		*pair;
	cplane_t	*pl;

	InitClassSearchIterator( &iter, portals, "portal" );
	for ( ; ( portal = SearchGetNextClass( &iter ) ) ; )
	{
		pair = FindHPair( portal, "front_contents" );
		if ( !pair )
			Error( "missing 'front_contents' in portal '%s'.\n", portal->name );
		HPairCastToInt_safe( &c1, pair );

		pair = FindHPair( portal, "back_contents" );
		if ( !pair )
			Error( "missing 'back_contents' in portal '%s'.\n", portal->name );
		HPairCastToInt_safe( &c2, pair );

		pair = FindHPair( portal, "plane" );
		if ( !pair )
			Error( "missing 'plane' in portal '%s'.\n", portal->name );
		plane = HManagerSearchClassName( planehm, pair->value );
		if ( !plane )
			Error( "portal '%s' can't find plane '%s'.\n", portal->name, pair->value );
		pl = GetClassExtra( plane );

		if ( c1 == 2 && c2 == 8 )
		{
			// front leaf gets the normal
			pair = FindHPair( portal, "frontnode" );
			if ( !pair )
				Error( "missing 'frontnode' in portal '%s'.\n", portal->name );
			leaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !leaf )
				Error( "portal '%s' can't find node '%s'.\n",  portal->name, pair->value );
			
			pair = FindHPair( leaf, "fieldvector" );
			if ( !pair )
			{
				// it's the first normal				
				pair = NewHPair2( "vec3d", "fieldvector", "x" );
				HPairCastFromVec3d( pl->norm, pair );
				InsertHPair( leaf, pair );
				add_leaf_num++;
			}
			else
			{
				// add normal
				vec3d_t		vec;
				HPairCastToVec3d( vec, pair );
				Vec3dAdd( vec, vec, pl->norm );
				HPairCastFromVec3d( vec, pair );
				
			}
			add_portal_num++;
		}
		else if ( c2 == 2 && c1 == 8 )
		{
			// back node gets the flip normal
			pair = FindHPair( portal, "backnode" );
			if ( !pair )
				Error( "missing 'backnode' in portal '%s'.\n", portal->name );
			leaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !leaf )
				Error( "portal '%s' can't find node '%s'.\n",  portal->name, pair->value );
			
			pair = FindHPair( leaf, "fieldvector" );
			if ( !pair )
			{
				// it's the first normal				
				pair = NewHPair2( "vec3d", "fieldvector", "x" );
				HPairCastFromVec3d( pl->flipplane->norm, pair );
				InsertHPair( leaf, pair );
				add_leaf_num++;
			}
			else
			{
				// add normal
				vec3d_t		vec;
				HPairCastToVec3d( vec, pair );
				Vec3dAdd( vec, vec, pl->flipplane->norm );
				HPairCastFromVec3d( vec, pair );
				
			}	
			add_portal_num++;
		}
	}
}

void BuildFieldNodesRecursive( hobj_t *fieldnode, hobj_t *bspnode )
{
	hpair_t		*pair;
	hobj_t		*fieldchild;
	hobj_t		*bspchild;

	pair = FindHPair( bspnode, "portalized_leaf" );
	if ( pair )
	{
		// it's a leaf

		// has it a field vector ?
		pair = FindHPair( bspnode, "fieldvector" );
		if ( !pair )
			return;

		pair = NewHPair2( pair->type, pair->key, pair->value );
		InsertHPair( fieldnode, pair );		
	}
	else
	{
		// it's a node

		// copy plane
		pair = FindHPair( bspnode, "plane" );
		if ( !pair )
			Error( "missing 'plane' in node '%s'.\n", bspnode->name );
		pair = NewHPair2( pair->type, pair->key, pair->value );
		InsertHPair( fieldnode, pair );

		// build children
		bspchild = FindClassType( bspnode, "bspnode_front" );
		if ( !bspchild )
			Error( "missing 'bspnode_front' in node '%s'.\n", bspnode->name );
		fieldchild = NewClass( "fieldnode_front", bspchild->name );
		BuildFieldNodesRecursive( fieldchild, bspchild );
		InsertClass( fieldnode, fieldchild );

		bspchild = FindClassType( bspnode, "bspnode_back" );
		if ( !bspchild )
			Error( "missing 'bspnode_back' in node '%s'.\n", bspnode->name );
		fieldchild = NewClass( "fieldnode_back", bspchild->name );
		BuildFieldNodesRecursive( fieldchild, bspchild );
		InsertClass( fieldnode, fieldchild );
	}
}

/*
  ====================
  ReadPlaneClass

  ====================
*/
hmanager_t * ReadPlaneClass( char *name )
{
	tokenstream_t	*ts;
	hobj_t		*planecls;
	hmanager_t	*hm;
	hobj_search_iterator_t	iter;
	hobj_t		*plane;
	hobj_t		*flipplane;
	cplane_t		*pl;
	int		num;
	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	//
	// create compiled planes
	//

	fprintf( stderr, "load plane class and compile ...\n" );

	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		pl = NewCPlane();
		pl->count = 0;

		// plane norm
		pair = FindHPair( plane, "norm" );
		if ( !pair )
			Error( "missing plane normal.\n" );
		HPairCastToVec3d_safe( pl->norm, pair );

		// plane dist
		pair = FindHPair( plane, "dist" );
		if ( !pair )
			Error( "missing plane distance.\n" );
		HPairCastToFloat_safe( &pl->dist, pair );
		
		// plane type
		pair = FindHPair( plane, "type" );
		if ( !pair )
			Error( "missing plane type.\n" );
		HPairCastToInt_safe( &pl->type, pair );

		pl->self = plane;
		SetClassExtra( plane, pl );
		
	}

	//
	// resolve clsref_flipplane
	//
	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		// plane flipplane clsref
		pair = FindHPair( plane, "flipplane" );
		if ( !pair )
			Error( "missinig clsref flipplane" );

		flipplane = HManagerSearchClassName( hm, pair->value );
		if ( !flipplane )
			Error( "can't resolve clsref flipplane.\n" );

		pl = GetClassExtra( plane );
		pl->flipplane = GetClassExtra( flipplane );
	}

	printf( " %d planes\n", num );

	return hm;	
}


int main( int argc, char *argv[] )
{
	char		*in_plane_name;
	char		*in_node_name;
	char		*in_portal_name;
	char		*out_node_name;

	hmanager_t	*nodehm;
	hmanager_t	*portalhm;
	hmanager_t	*planehm;

	hobj_t		*fieldnode;

	FILE		*h;

	printf( "===== fieldnode =====\n" );
	SetCmdArgs( argc, argv );

	in_plane_name = GetCmdOpt2( "-pl" );
	in_node_name = GetCmdOpt2( "-n" );
	in_portal_name = GetCmdOpt2( "-p" );
	out_node_name = GetCmdOpt2( "-o" );

	if ( !in_portal_name )
	{
		in_portal_name = "_portalout_portal.hobj";
		printf( " default input portal class: %s\n", in_portal_name );
	}
	else
	{
		printf( " input portal class: %s\n", in_portal_name );
	}
	
	if ( !in_node_name )
	{
		in_node_name = "_leafflood_bspnode.hobj";
		printf( " default input node class: %s\n", in_node_name );
	}
	else
	{
		printf( " input node class: %s\n", in_node_name );
	}

	if ( !out_node_name )
	{
		out_node_name = "_fieldtree.hobj";
		printf( " default output node class: %s\n", out_node_name );
	}
	else
	{
		printf( " output node class: %s\n", out_node_name );
	}

	if ( !in_plane_name )
	{
		in_plane_name = "_plane.hobj";
		printf( " default input plane class: %s\n", in_plane_name );
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}

	planehm = ReadPlaneClass( in_plane_name );

	portalhm = NewHManagerLoadClass( in_portal_name );
	if ( !portalhm )
		Error( "can't load portal class.\n" );
	nodehm = NewHManagerLoadClass( in_node_name );
	if ( !nodehm )
		Error( "can't load node class.\n" );

	printf( "calc field vectors ...\n" );
	CalcFieldVectors( nodehm, HManagerGetRootClass( portalhm ), planehm );
	printf( " %d portals used for fieldvectors\n", add_portal_num );
	printf( " %d fieldleafs\n", add_leaf_num );

	fieldnode = NewClass( "fieldnodes", "topnode" );
	BuildFieldNodesRecursive( fieldnode, HManagerGetRootClass( nodehm ) );

	h = fopen( out_node_name, "w" );
	if ( !h )
		Error( "can't open output class.\n" );
	WriteClass( fieldnode, h );
	fclose( h );
}
