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



// leafflood.c

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

char		*touch_text;
char		*touch_ignore_text;
char		*flood_text;
char		*flood_ignore_text;

int		touch_contents;
int		touch_ignore_contents;
int		flood_contents;
int		flood_ignore_contents;

hobj_t * FindLeafForPoint( hmanager_t *nodehm, vec3d_t point, hmanager_t *planecls )
{
	hobj_t	*node;
	hobj_t	*plane;
	cplane_t	*pl;
	hpair_t		*pair;
	fp_t		d;
	hobj_t	*child;

	node = HManagerGetRootClass( nodehm );

	for(;;)
	{
		pair = FindHPair( node, "portalized_leaf" );
		if ( pair )
		{
			return node;
		}

		pair = FindHPair( node, "plane" );
		if ( !pair )
		{
			// leaf
			Error( "missing 'plane' in none portalized_leaf '%s'.\n", node->name );
//			return node;
		}
		else
		{
			// node
			plane = HManagerSearchClassName( planecls, pair->value );
			if ( !plane )
				Error( "node '%s' can't find plane '%s'.\n", node->name, pair->value );
			pl = GetClassExtra( plane );

			d = Vec3dDotProduct( pl->norm, point ) - pl->dist;

			if ( d >= 0 )
			{
				// go to front child
				child = FindClassType( node, "bspnode_front" );
				if ( !child )
					Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
				
				node = child;
			}
			else
			{
				// go to back child
				child = FindClassType( node, "bspnode_back" );
				if ( !child )
					Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
				
				node = child;
			}			
		}
	}
}


/*
  ====================
  FloodThroughLeafsRecursive

  ====================
*/
static int	flood_num = 0;
static int	touch_num = 0;
static int	flood_ignore_num = 0;
static int	touch_ignore_num = 0;

static int	leaf_num;

void EraseFloodFlagRecursive( hobj_t *node )
{
	hpair_search_iterator_t		iter;
	hpair_t				*pair;
	hobj_t				*child;
	int			num;

	// always remove
	RemoveAndDestroyAllHPairsOfKey( node, "flood_state" );
//	RemoveAndDestroyAllHPairsOfKey( node, touch_text );


	pair = FindHPair( node, "plane" );

	if ( pair )
	{
		// a plane, it's a node

		child = FindClassType( node, "bspnode_front" );
		if ( !child )
			Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
		EraseFloodFlagRecursive( child );

		child = FindClassType( node, "bspnode_back" );
		if ( !child )
			Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
		EraseFloodFlagRecursive( child );		
	}
	else
	{
		leaf_num++;		
	}
}

#include "node_debug.c"

FILE	*trace_file;

bool_t FloodThroughLeafsRecursive( hobj_t *leaf, hmanager_t *portalhm, hmanager_t *nodehm )
{
	hpair_search_iterator_t		iter;
	hpair_t			*pair;
	hobj_t			*portal;
	hobj_t			*otherleaf;
	int		contents;

	pair = FindHPair( leaf, "flood_state" );
	if ( pair )
	{
		// allready flooded
		return true;
	}

	// mark as done

	
	pair = FindHPair( leaf, "contents" );
	if ( !pair )
		Error( "missing 'contents' in node '%s'.\n", leaf->name );
	HPairCastToInt_safe( &contents, pair );
	if ( (contents & flood_contents) || ( flood_contents == 0 && contents == 0 ) )
	{
		flood_num++;
		pair = NewHPair2( "string", "flood_state", flood_text );
		InsertHPair( leaf, pair );
	}
	else if ( contents & flood_ignore_contents )
	{
		flood_ignore_num++;
		pair = NewHPair2( "string", "flood_state", flood_ignore_text );
		InsertHPair( leaf, pair );		
	}
	else
	{
		Error( "how can I flood into this leaf, with %d as contents ?\n", contents );
	}

	portal = NULL;

	// test touch_outside
	pair =  FindHPair( leaf, "touch_outside" );
	if ( pair )
	{
		printf( "touch_outside !\n" ); 
		goto write_leaf;
	}

	InitHPairSearchIterator( &iter, leaf, "portal" );

	for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
	{
		portal = HManagerSearchClassName( portalhm, pair->value );
		if ( !portal )
			Error( "leaf '%s' can't find portal '%s'.\n", leaf->name, pair->value );

		pair = FindHPair( portal, "frontnode" );
		if ( !pair )
			Error( "missing 'frontnode' in portal '%s'.\n", portal->name );
		if ( !strcmp( pair->value, leaf->name ) )
		{
			// I'm the frontnode of the portal, let's go to the backnode
			pair = FindHPair( portal, "backnode" );
			if ( !pair )
				Error( "missing 'backnode' in portal '%s'.\n", portal->name );

			otherleaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !otherleaf )
				Error( "portal '%s' can't find leaf '%s'.\n", portal->name, pair->value );
		}
		else
		{
			otherleaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !otherleaf )
				Error( "portal '%s' can't find leaf '%s'.\n", portal->name, pair->value );
		}


		pair = FindHPair( portal, "state" );
		if ( !pair )
			Error( "can't get 'state' from portal '%s'.\n", portal->name );
		
		if ( !strcmp( pair->value, "closed" ) )
		{
			// portal is closed, don't flood through
			// but mark it as touched
			
			pair = FindHPair( otherleaf, "flood_state" );
			if ( !pair )
			{
				pair = FindHPair( otherleaf, "contents" );
				if ( !pair )
					Error( "missing 'contents' in node '%s'.\n", otherleaf->name );
				HPairCastToInt_safe( &contents, pair );

				if ( contents & touch_contents )
				{
					pair = NewHPair2( "string", "flood_state", touch_text );
					InsertHPair( otherleaf, pair );
					touch_num++;
				}
				else if ( contents & touch_ignore_contents )
				{					
					pair = NewHPair2( "string", "flood_state", touch_ignore_text );
					InsertHPair( otherleaf, pair );
					touch_ignore_num++;
				}
			}
		}
		else
		{
			if ( !FloodThroughLeafsRecursive( otherleaf, portalhm, nodehm ) )
				goto write_leaf;
		}
	}

	return true;

write_leaf:

	printf( "trace: %s\n", leaf->name );
	if ( portal )
		WritePortals( portal, trace_file );
	else
		WriteNodePortals( leaf, portalhm, trace_file );
	return false;
}

/*
  ====================
  FloodSectors

  ====================
*/
void FloodSectors( hmanager_t *athm, hmanager_t *planehm, hmanager_t *portalhm, hmanager_t *nodehm )
{
	hobj_search_iterator_t	iter;
	hobj_t			*sector;
	hobj_t			*leaf;
	hpair_t			*pair;
	vec3d_t			origin;
	int		num;

	InitClassSearchIterator( &iter, HManagerGetRootClass( athm ), "sector" );
	for ( num = 0; ( sector = SearchGetNextClass( &iter ) ); num++ )
	{
		pair = FindHPair( sector, "origin" );
		if ( !pair )
			Error( "missing 'origin' of sector '%s'.\n", sector->name );
		HPairCastToVec3d_safe( origin, pair );

		leaf = FindLeafForPoint( nodehm, origin, planehm );
		printf( "the start leaf for sector '%s' is '%s'.\n", sector->name, leaf->name );
		
		leaf_num = 0;
		EraseFloodFlagRecursive( HManagerGetRootClass( nodehm ) );
		
		flood_num = 0;
		flood_ignore_num = 0;
		touch_num = 0;
		touch_ignore_num = 0;

		printf( "flood through leafs ...\n" );
		FloodThroughLeafsRecursive( leaf, portalhm, nodehm );
		printf( " %d floodable leafs.\n", leaf_num );
		printf( " %d leafs are flood.\n", flood_num );
		printf( " %d leafs are flood_ignore.\n", flood_ignore_num );
		printf( " %d leafs are touch.\n", touch_num );
		printf( " %d leafs are touch_ignore.\n", touch_ignore_num );

	}

	if ( !num )
		Error( "no 'sector' found in archetype class.\n" );
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
	char		*in_portal_name;
	char		*in_node_name;	
	char		*out_node_name;
	char		*in_plane_name;
	char		*in_at_name;

	tokenstream_t	*ts;
	FILE		*h;

	hobj_t		*portalcls;
	hobj_t		*nodecls;

	hmanager_t	*portalhm;
	hmanager_t	*nodehm;
	hmanager_t	*planehm;
	hmanager_t	*athm;

	printf( "===== leafflood - flood through open portals and mark reached leafs =====\n" );
	SetCmdArgs( argc, argv );

	in_portal_name = GetCmdOpt2( "-p" );
//	in_brush_name = GetCmdOpt2( "-b" );
	in_node_name = GetCmdOpt2( "-n" );
	out_node_name = GetCmdOpt2( "-o" );
	in_plane_name = GetCmdOpt2( "-pl" );
	in_at_name = GetCmdOpt2( "-at" );
	
	if ( !in_portal_name )
	{
		in_portal_name = "_mkpcout_portal.hobj";
		printf( " default input portal class: %s\n", in_portal_name );
	}
	else
	{
		printf( " input portal class: %s\n", in_portal_name );	
	}

	if ( !in_node_name )
	{
		in_node_name = "_portalout_bspnode.hobj";
		printf( " default input bspnode class: %s\n", in_node_name );
	}
	else
	{
		printf( " input bspnode class: %s\n", in_node_name );
	}

	if ( !out_node_name )
	{
		out_node_name = "_leafflood_bspnode.hobj";
		printf( " default output bspnode class: %s\n", out_node_name );
	}
	else
	{
		printf( " output bspnode class: %s\n", out_node_name );
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

	if ( !in_at_name )
	{
		in_at_name = "ats.hobj";
		printf( " default archetype class: %s\n", in_at_name );
	}
	else
	{
		printf( " input archetype class: %s\n", in_at_name );
	}

	touch_text = GetCmdOpt2( "-touch_text" );
	if ( !touch_text )
	{
		touch_text = "touch";
		printf( " default key for touched leafs: %s\n", touch_text );
	}
	else
	{
		printf( "Switch: key for touched leafs: %s\n", touch_text );
	}
	
	flood_text = GetCmdOpt2( "-flood_text" );
	if ( !flood_text )
	{
		flood_text = "flood";
		printf( " default key for flooded leafs: %s\n", flood_text );
	}
	else
	{
		printf( "Switch: key for flooded leafs: %s\n", flood_text );
	}
	
	touch_ignore_text = GetCmdOpt2( "-touch_ignore_text" );
	if ( !touch_ignore_text )
	{
		touch_ignore_text = "touch_ignore";
		printf( " default key for touch_ignore leafs: %s\n", touch_ignore_text );
	}
	else
	{
		printf( "Switch: key for touch_ignore leafs: %s\n", touch_ignore_text );
	}	

	flood_ignore_text = GetCmdOpt2( "-flood_ignore_text" );
	if ( !flood_ignore_text )
	{
		flood_ignore_text = "flood_ignore";
		printf( " default key for flood_ignore leafs: %s\n", flood_ignore_text );
	}
	else
	{
		printf( "Switch: key for flood_ignore leafs: %s\n", flood_ignore_text );
	}	
	
	if ( !GetCmdOpt2( "-fc" ) )
	{
		flood_contents = 0;
	}
	else
	{
		flood_contents = atoi( GetCmdOpt2( "-fc" ) );		
	}

	if ( !GetCmdOpt2( "-fci" ) )
	{
		flood_ignore_contents = 1<<16;
	}
	else
	{
		flood_ignore_contents = atoi( GetCmdOpt2( "-fci" ) );		
	}

	
	if ( !GetCmdOpt2( "-tc" ) )
	{
		touch_contents = 4+8+16;
	}
	else
	{
		touch_contents = atoi( GetCmdOpt2( "-tc" ) );		
	}

	if ( !GetCmdOpt2( "-tci" ) )
	{
		touch_ignore_contents = 1<<16;
	}
	else
	{
		touch_ignore_contents = atoi( GetCmdOpt2( "-tci" ) );		
	}

	printf( "flood contents: %d\n", flood_contents );
	printf( "flood ignore contents: %d\n", flood_ignore_contents );
	printf( "touch contents: %d\n", touch_contents );
	printf( "touch ignore contnets: %d\n", touch_ignore_contents );
	

	printf( "load portal class ...\n" );
	ts = BeginTokenStream( in_portal_name );
	if ( !ts )
		Error( "can't open portal class.\n" );
	portalcls = ReadClass( ts );
	EndTokenStream( ts );
	portalhm = NewHManager();
	HManagerSetRootClass( portalhm, portalcls );
	HManagerRebuildHash( portalhm );
//	DumpHManager( portalhm, false ); 

	printf( "load bspnode class ...\n" );
	ts = BeginTokenStream( in_node_name );
	if ( !ts )
		Error( "can't open bspnode class.\n" );
	nodecls = ReadClass( ts );
	EndTokenStream( ts );
	nodehm = NewHManager();
	HManagerSetRootClass( nodehm, nodecls );
	HManagerRebuildHash( nodehm );

	planehm = ReadPlaneClass( in_plane_name );
	
	printf( "load archetype class ...\n" );
	ts = BeginTokenStream( in_at_name );
	if ( !ts )
		Error( "can't open class.\n" );
	athm = NewHManager();
	HManagerSetRootClass( athm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( athm );

	trace_file = fopen( "trace", "w" );
	if ( !trace_file )
		Error( "can't open trace file.\n" );

	printf( "flood all sectors ...\n" );
	FloodSectors( athm, planehm, portalhm, nodehm );

	fprintf( trace_file, "end" );
	fclose( trace_file );
 
 
#if 0
	leaf = FindLeafForPoint( nodehm, pos, planecls );
	printf( "the leaf for the point is '%s'.\n", leaf->name );

	leaf_num = 0;
	EraseFloodFlagRecursive( HManagerGetRootClass( nodehm ) );

	flood_num = 0;
	touch_num = 0;
	printf( "flood through leafs ...\n" );
	FloodThroughLeafsRecursive( leaf, portalhm, nodehm );
	printf( " %d floodable leafs.\n", leaf_num );
	printf( " %d leafs are flood.\n", flood_num );
	printf( " %d leafs are flood_ignore.\n", flood_ignore_num );
	printf( " %d leafs are touch.\n", touch_num );
	printf( " %d leafs are touch_ignore.\n", touch_ignore_num );
	
#endif


	printf( "write bspnode class ...\n" );
	h = fopen( out_node_name, "w" );
	if ( !h )
		Error( "can't open output bspnode class.\n" );

	WriteClass( HManagerGetRootClass( nodehm ), h );
	fclose( h );
}
