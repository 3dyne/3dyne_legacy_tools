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



// gather2.c

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

/*
  ==============================
  FindLeafForPoint

  ==============================
*/
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
  ==============================
  EraseFloodFlagRecursive

  ==============================
*/
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

	}
}


/*
  ====================
  BuildTouchLeafsRecursive

  ====================
*/
static int	gather_num;
static int	polygon_num;

hobj_t * BuildPolygonClassFromPortal( hobj_t *portal, bool_t flip )
{
	hobj_t		*polycls;
	hpair_t		*pair;
	int		i, num;
	char		tt[256];
	
	sprintf( tt, "#%u", HManagerGetFreeID() );
	polycls = NewClass( "polygon", tt );	

	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' of portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );

	pair = NewHPair2( "int", "num", pair->value );
	InsertHPair( polycls, pair );

	if ( flip )
	{
		for ( i = num-1; i >= 0; i-- )
		{
			sprintf( tt, "%d", i );
			pair = FindHPair( portal, tt );
			if ( !pair )
				Error( "missing point '%s' of portal '%s'.\n", tt, portal->name );

			sprintf( tt, "%d", (num-1)-i );
			pair = NewHPair2( pair->type, tt, pair->value );
			InsertHPair( polycls, pair );
		}
	}
	else
	{
		for ( i = 0; i < num; i++ )
		{
			sprintf( tt, "%d", i );
			pair = FindHPair( portal, tt );
			if ( !pair )
				Error( "missing point '%s' of portal '%s'.\n", tt, portal->name );
			
			sprintf( tt, "%d", i );
			pair = NewHPair2( pair->type, tt, pair->value );
			InsertHPair( polycls, pair );	
		}
	}

//	polygon_num++;

	return polycls;

}

hobj_t * BuildPolygonClassFromPolygon( polygon_t *p )
{
	hobj_t		*polycls;
	hpair_t		*pair;
	int		i, num;
	char		tt[256];
	
	sprintf( tt, "#%u", HManagerGetFreeID() );
	polycls = NewClass( "polygon", tt );	

	sprintf( tt, "%d", p->pointnum );
	pair = NewHPair2( "int", "num", tt );
	InsertHPair( polycls, pair );

	
	for ( i = 0; i < p->pointnum; i++ )
	{
		pair = NewHPair();
		sprintf( pair->type, "vec3d" );
		sprintf( pair->key, "%d", i );
		sprintf( pair->value, "%f %f %f", p->p[i][0], p->p[i][1], p->p[i][2] );
		InsertHPair( polycls, pair );	
	}

	polygon_num++;

	return polycls;

}


polygon_t * BuildPolygonFromPortal( hobj_t *portal, bool_t flip )
{
	polygon_t	*p;
	hpair_t		*pair;
	int		i, num;
	char		tt[256];
	

	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' of portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );

	p = NewPolygon( num );
	p->pointnum = num;

	if ( flip )
	{
		for ( i = num-1; i >= 0; i-- )
		{
			sprintf( tt, "%d", i );
			pair = FindHPair( portal, tt );
			if ( !pair )
				Error( "missing point '%s' of portal '%s'.\n", tt, portal->name );
			HPairCastToVec3d( p->p[(num-1)-i], pair );
		}
	}
	else
	{
		for ( i = 0; i < num; i++ )
		{
			sprintf( tt, "%d", i );
			pair = FindHPair( portal, tt );
			if ( !pair )
				Error( "missing point '%s' of portal '%s'.\n", tt, portal->name );
			HPairCastToVec3d( p->p[i], pair );
		}
	}

//	polygon_num++;

	return p;
}

#define MAX_SURFACES	( 128 )
void InsertPortalIntoBrush( hobj_t *brush, hobj_t *portal, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;	
	hobj_t		*surface;
	hobj_t		*plane;
	cplane_t	*ppl;
	cplane_t	*spl;

	hpair_t		*pair;

	int		i;
	int		surfacenum = 0;
	cplane_t	*planes[MAX_SURFACES];
	hobj_t		*insertsurf;
	polygon_t	*poly;

	// get portal plane
	pair = FindHPair( portal, "plane" );
	if ( !pair )
		Error( "missing 'plane' in portal '%s'.\n", portal->name );
	plane = HManagerSearchClassName( planehm, pair->value );
	if ( !plane )
		Error( "portal '%s' can't find plane '%s'\n", portal->name, pair->value );
	ppl = GetClassExtra( plane );
	
	// test with all surface planes
	insertsurf = NULL;
	InitClassSearchIterator( &iter, brush, "surface" );
	for ( ; ( surface = SearchGetNextClass( &iter ) ) ; )
	{
		pair = FindHPair( surface, "plane" );
		if ( !pair )
			Error( "missing 'plane' in bspbrush '%s'\n", brush->name );
		plane = HManagerSearchClassName( planehm, pair->value );
		if ( !plane )
			Error( "bspbrush '%s' can't find plane '%s'\n", brush->name, pair->value );

		if ( surfacenum == MAX_SURFACES )
			Error( "reached MAX_SURFACES\n" );
		planes[surfacenum++] = spl = GetClassExtra( plane );
	
		if ( insertsurf )
			continue;
	
		if ( ppl == spl )
		{
			poly = BuildPolygonFromPortal( portal, false );
//			InsertClass( surface, BuildPolygonClassFromPortal( portal, false ) );			
			insertsurf = surface;
//			break;
		}
		else if ( spl == ppl->flipplane )
		{
			poly = BuildPolygonFromPortal( portal, true );
//			InsertClass( surface, BuildPolygonClassFromPortal( portal, true ) );
			insertsurf = surface;
//			break;
		}
	}		       		

	if ( !insertsurf )
	{
		// no surface found in brush for the portal
		return;
	}

	// clip 'poly' by all surfaces of the 'brush'
	// if it is not clipped away, insert it into 'insertsurf'
	for ( i = 0; i < surfacenum; i++ )
	{
		ClipPolygonInPlace( &poly, planes[i]->norm, planes[i]->dist );
		if ( !poly )
		{
			// 'poly' clipped away, it's not the right brush
			return;
		}
	}
	
	InsertClass( insertsurf, BuildPolygonClassFromPolygon( poly ) );
	FreePolygon( poly );

	gather_num++;			
}


void InsertPortalIntoLeaf( hobj_t *node, hobj_t *portal, hmanager_t *brushhm, hmanager_t *planehm )
{
	hpair_search_iterator_t		iter;
	hpair_t				*pair;
	hobj_t			*brush;

	InitHPairSearchIterator( &iter, node, "brush" );
	for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
	{
		brush = HManagerSearchClassName( brushhm, pair->value );
		if ( !brush )
			Error( "node '%s' can't find brush '%s'.\n", node->name, pair->value );

		InsertPortalIntoBrush( brush, portal, planehm );
	}
}


void BuildTouchLeafsRecursive( hobj_t *node, hmanager_t *portalhm, hmanager_t *nodehm, hmanager_t *brushhm, hmanager_t *planehm )
{
	hpair_t		*pair;
	hobj_t		*child;
	hpair_search_iterator_t		iter;
	hobj_t		*portal;
	hobj_t		*otherleaf;
	hobj_t		*brush;

//	pair = FindHPair( node, "plane" );
	pair = FindHPair( node, "portalized_leaf" );
	if ( !pair )
	{
		// it's a node
		
		child = FindClassType( node, "bspnode_front" );
		if ( !child )
			Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
		BuildTouchLeafsRecursive( child, portalhm, nodehm, brushhm, planehm );

		child = FindClassType( node, "bspnode_back" );
		if ( !child )
			Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
		BuildTouchLeafsRecursive( child, portalhm, nodehm, brushhm, planehm  );
	}
	else
	{
		// it's a leaf
		// is it touched ?

		pair = FindHPair( node, "flood_state" );
		if ( !pair )
		{
			// not touched not flooded, ignore
			return;
		}
		
		// only flood_state 'touch' gather portals, ignore 'touch_ignore', 'flood', 'flood_ignore'
		if ( strcmp( pair->value, "touch" ) )
			return;
		
		// it's touched, go through all portals
		// and if the otherleaf are flooded, take the portal
		
		InitHPairSearchIterator( &iter, node, "portal" );
		for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
		{
			portal = HManagerSearchClassName( portalhm, pair->value );
			if ( !portal )
				Error( "leaf '%s' can't find portal '%s'.\n", node->name, pair->value );
			

			// get otherleaf

			pair = FindHPair( portal, "frontnode" );
			if ( !pair )
				Error( "missing 'frontnode' in portal '%s'.\n", portal->name );
			if ( !strcmp( pair->value, node->name ) )
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

			pair = FindHPair( otherleaf, "flood_state" );

			if ( !pair )
			{
				// other leaf was not flooded, ignore portal
				continue;
			}
					      		
			if ( !strcmp( pair->value, "flood" ) ||
			     !strcmp( pair->value, "flood_ignore" ) )
			{
				// ok, otherleaf was flooded, take the portal								
				InsertPortalIntoLeaf( node, portal, brushhm, planehm );
			}
			// no portals from touch, touch_ignore or flood_state_less otherleafs
		}		
	}
}

/*
  ====================
  ClassifyBrushSurfaces

  ====================
*/

int		surface_num = 0;
int		ignore_num = 0;
void ClassifyBrushSurfaces( hobj_t *brush )
{
	hobj_search_iterator_t	iter;	
	hobj_search_iterator_t	polyiter;	
	hobj_t		*surface;
	hpair_t		*pair;
	int		num;

	// a surface contains no portals
	// it may not be used for a better bsp-tree

	InitClassSearchIterator( &iter, brush, "surface" );
	for ( ; ( surface = SearchGetNextClass( &iter ) ) ; )
	{
		InitClassSearchIterator( &polyiter, surface, "polygon" );	
		for ( num = 0; SearchGetNextClass( &polyiter ) ; num++ )
		{ }

		if ( !num )
		{
			pair = NewHPair2( "int", "bsp_ignore", "1" );
			InsertHPair( surface, pair );
			ignore_num++;
		}
		surface_num++;
	}

}

void ClassifyLeafBrushes( hobj_t *node, hmanager_t *brushhm )
{
	hpair_search_iterator_t		iter;
	hpair_t				*pair;
	hobj_t			*brush;

	InitHPairSearchIterator( &iter, node, "brush" );
	for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
	{
		brush = HManagerSearchClassName( brushhm, pair->value );
		if ( !brush )
			Error( "node '%s' can't find brush '%s'.\n", node->name, pair->value );
		
		ClassifyBrushSurfaces( brush );
	}	
}


void ClassifyLeafsRecursive( hobj_t *node, hmanager_t *brushhm, bool_t touch_only )
{
	hpair_t		*pair;
	hobj_t		*child;
	hpair_search_iterator_t		iter;
	hobj_t		*portal;
	hobj_t		*otherleaf;
	hobj_t		*brush;

//	pair = FindHPair( node, "plane" );
	pair = FindHPair( node, "portalized_leaf" );
	if ( !pair )
	{
		// it's a node
		
		child = FindClassType( node, "bspnode_front" );
		if ( !child )
			Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
		ClassifyLeafsRecursive( child, brushhm, touch_only );

		child = FindClassType( node, "bspnode_back" );
		if ( !child )
			Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
		ClassifyLeafsRecursive( child, brushhm, touch_only  );
	}
	else
	{

		// should only touched leafs be classified ?
		if ( touch_only )
		{
			// it's a leaf
			// is it touched ?

			pair = FindHPair( node, "touch" );
			if ( !pair )
				return;	// no
		}

		ClassifyLeafBrushes( node, brushhm );

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

/*
  ====================
  FloodSectors

  ====================
*/
void FloodSectors( hmanager_t *athm, hmanager_t *portalhm, hmanager_t *nodehm, hmanager_t *brushhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;
	hobj_t			*sector;
	hobj_t			*leaf;
	hpair_t			*pair;
	vec3d_t			origin;
	int		num;

#if 1
	BuildTouchLeafsRecursive( HManagerGetRootClass( nodehm ), portalhm, nodehm, brushhm, planehm );
#else
	InitClassSearchIterator( &iter, HManagerGetRootClass( athm ), "sector" );
	for ( num = 0; ( sector = SearchGetNextClass( &iter ) ); num++ )
	{
		pair = FindHPair( sector, "origin" );
		if ( !pair )
			Error( "missing 'origin' of sector '%s'.\n", sector->name );
		HPairCastToVec3d_safe( origin, pair );

		leaf = FindLeafForPoint( nodehm, origin, planehm );
		printf( "the start leaf for sector '%s' is '%s'.\n", sector->name, leaf->name );
		
//		EraseFloodFlagRecursive( HManagerGetRootClass( nodehm ) );
		
		printf( "flood through leafs ...\n" );
		BuildTouchLeafsRecursive( leaf, portalhm, nodehm, brushhm, planehm );

	}

	if ( !num )
		Error( "no 'sector' found in archetype class.\n" );
#endif
}


void PrintHelp( void )
{
	puts( "usage:" );
	puts( " -p\t input portal class" );
	puts( " -n\t input node class" );
	puts( " -i\t input brush class" );
	puts( " -pl\t input plane class" );
	puts( " -o\t output brush class" );
}

int main( int argc, char *argv[] )
{
	char	*in_portal_name;
	char	*in_node_name;
	char	*in_brush_name;
	char	*in_plane_name;
	char	*out_brush_name;
	char	*in_at_name;

	hmanager_t		*portalhm;
	hmanager_t		*nodehm;
	hmanager_t		*brushhm;
	hmanager_t	*planehm;

	hmanager_t		*athm;

	tokenstream_t		*ts;
	FILE			*h;

	printf( "===== gather - collect portals of touched leafs =====\n" );
	SetCmdArgs( argc, argv );

	if ( CheckCmdSwitch2( "--help" ) )
	{
		PrintHelp();
		exit(-1);
	}

	in_portal_name = GetCmdOpt2( "-p" );
	in_node_name = GetCmdOpt2( "-n" );
	in_brush_name = GetCmdOpt2( "-i" );
	in_plane_name = GetCmdOpt2( "-pl" );
	out_brush_name = GetCmdOpt2( "-o" );
	in_at_name = GetCmdOpt2( "-at" );

	if ( !in_at_name )
	{
		Error( "no input arche type class\n" );		
	}
	
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
		in_node_name = "_leafflood_bspnode.hobj";
		printf( " default input bspnode class: %s\n", in_node_name );
	}
	else
	{
		printf( " input bspnode class: %s\n", in_node_name );
	}

	if ( !in_brush_name )
	{
		in_brush_name = "_bspout_bspbrush.hobj";
		printf( " default input bspbrush class: %s\n", in_brush_name );
	}
	else
	{
		printf( " input bspbrush class: %s\n", in_brush_name );
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

	if ( !out_brush_name )
	{
		out_brush_name = "_gather_bspbrush.hobj";
		printf( " default output bspbrush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output bspbrush class: %s\n", out_brush_name );
	}

	printf( "load portal class ...\n" );
	portalhm = NewHManager();
	ts = BeginTokenStream( in_portal_name );
	if ( !ts )
		Error( "can't open input portal class.\n" );
	HManagerSetRootClass( portalhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( portalhm );

	printf( "load node class ...\n" );
	nodehm = NewHManager();
	ts = BeginTokenStream( in_node_name );
	if ( !ts )
		Error( "can't open input node class.\n" );
	HManagerSetRootClass( nodehm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( nodehm );

	printf( "load brush class ...\n" );
	brushhm = NewHManager();
	ts = BeginTokenStream( in_brush_name );
	if ( !ts )
		Error( "can't open input brush class.\n" );
	HManagerSetRootClass( brushhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( brushhm );

	planehm = ReadPlaneClass( in_plane_name );

	printf( "load archetype class\n" );
	athm = NewHManagerLoadClass( in_at_name );
	if ( !athm )
		Error( "failed\n" ); 

	gather_num = 0;
	polygon_num = 0;
	printf( "gather portals ...\n" );
	FloodSectors( athm, portalhm, nodehm, brushhm, planehm );

	printf( " %d portals gathered.\n", gather_num );
	printf( " %d polygons build for brush surfaces.\n", polygon_num );
	printf( " %d surfaces of %d are marked as 'bsp_ignore'.\n", ignore_num, surface_num );

	DeepDumpClass( HManagerGetRootClass( brushhm ) );

	printf( "write output bspbrush class ...\n" );
	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( brushhm ), h );
	fclose( h );

	HManagerSaveID();

	exit(0);
}
