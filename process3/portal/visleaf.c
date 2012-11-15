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



// visleaf.c

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
  ====================
  BuildVisleafsRecursive

  ====================
*/
void CopyPortalPolygonToPortal( hobj_t *vportal, hobj_t *portal, bool_t flip )
{
	hpair_t		*pair;
	int		i, num;
	char		tt[256];
	
	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' of portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );

	pair = NewHPair2( "int", "pointnum", pair->value );
	InsertHPair( vportal, pair );

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
			InsertHPair( vportal, pair );
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
			InsertHPair( vportal, pair );	
		}
	}
}

void CalcPortalCenter( hobj_t *portal, vec3d_t center, vec3d_t min, vec3d_t max )
{
	vec3d_t		p;
	int		i, num;
	hpair_t		*pair;
	char		tt[256];

	Vec3dInit( center, 0, 0, 0 );

	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' of portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );

	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( portal, tt );
		if ( !pair )
			Error( "missing point '%s' of portal '%s'.\n", tt, portal->name );
		
		HPairCastToVec3d( p, pair );
		Vec3dAddToBB( min, max, p );
		Vec3dAdd( center, center, p );
	}	
	Vec3dScale( center, 1.0/num, center );
}


hobj_t * BuildVisleaf( hobj_t *leaf, hmanager_t *portalhm, hmanager_t *nodehm, hmanager_t *planehm )
{
	hpair_search_iterator_t		iter;
	bool_t			flip;
	cplane_t		*pl;
	hobj_t			*portal;
	hobj_t			*otherleaf;
	hobj_t			*vportal;
	hobj_t			*plane;
	hobj_t			*visleaf;
	hpair_t			*pair;
	char			tt[256];

	vec3d_t			center, pc, min, max;
	int			num;

	Vec3dInit( center, 0, 0, 0 );
	Vec3dInitBB( min, max, 999999.9 );

	sprintf( tt, "#%u", HManagerGetFreeID() );
	visleaf = NewClass( "visleaf", tt );
	
	InitHPairSearchIterator( &iter, leaf, "portal" );
	for ( num = 0; ( pair = SearchGetNextHPair( &iter ) ); num++ )
	{
		portal = HManagerSearchClassName( portalhm, pair->value );
		if ( !portal )
			Error( "leaf '%s' can't find portal '%s'.\n", leaf->name, pair->value );

		CalcPortalCenter( portal, pc, min, max );
		Vec3dAdd( center, center, pc );
		
		// get otherleaf
		
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
			flip = true;
		}
		else
		{
			otherleaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !otherleaf )
				Error( "portal '%s' can't find leaf '%s'.\n", portal->name, pair->value );

			flip = false;
		}

		pair = FindHPair( portal, "plane" );
		if ( !pair )
			Error( "missing 'plane' of portal '%s'.\n", portal->name );
		plane = HManagerSearchClassName( planehm, pair->value );
		if ( !plane )
			Error( "portal '%s' can't find plane '%s'.\n", portal->name, pair->value );
		pl = GetClassExtra( plane );
		if ( flip )
			pl = pl->flipplane;
	      

		sprintf( tt, "#%u", HManagerGetFreeID() );
		vportal = NewClass( "portal", tt );
		
		pair = NewHPair2( "ref", "plane", pl->self->name );
		InsertHPair( vportal, pair );
		
		CopyPortalPolygonToPortal( vportal, portal, flip );
		
		pair = FindHPair( portal, "state" );
		if ( !pair )
			Error( "missing 'state' of portal '%s'.\n", portal->name );
		if ( !strcmp( pair->value, "open" ) )
		{
			// open portal goes to othernode

			pair = NewHPair2( "ref", "otherleaf", otherleaf->name );
			InsertHPair( vportal, pair );
		}
		else
		{
			// closed portals 
			pair = NewHPair2( "ref", "touchleaf", otherleaf->name );
			InsertHPair( visleaf, pair );
			
			// hack: also insert touchleaf into the closed portal
			pair = NewHPair2( pair->type, pair->key, pair->value );
			InsertHPair( vportal, pair );
		}
		
		InsertClass( visleaf, vportal );
	}

	// leaf center
	Vec3dScale( center, 1.0/num, center );
	pair = NewHPair2( "vec3d", "center", "x" );
	HPairCastFromVec3d( center, pair );
	InsertHPair( visleaf, pair );

	// leaf bounds min
	pair = NewHPair2( "vec3d", "min", "x" );
	HPairCastFromVec3d( min, pair );
	InsertHPair( visleaf, pair );

	// leaf bounds max
	pair = NewHPair2( "vec3d", "max", "X" );
	HPairCastFromVec3d( max, pair );
	InsertHPair( visleaf, pair );
	
	return visleaf;
}


void BuildVisleafsRecursive( hobj_t *node, hmanager_t *portalhm, hmanager_t *nodehm, hmanager_t *planehm, hobj_t *visleafcls )
{
	hpair_t		*pair;
	hobj_t		*child;
	hobj_t		*visleaf;


//	pair = FindHPair( node, "plane" );
	pair = FindHPair( node, "portalized_leaf" );
	if ( !pair )
	{
		// it's a node
		
		child = FindClassType( node, "bspnode_front" );
		if ( !child )
			Error( "missing 'bspnode_front' in node '%s'.\n", node->name );
		BuildVisleafsRecursive( child, portalhm, nodehm, planehm, visleafcls );

		child = FindClassType( node, "bspnode_back" );
		if ( !child )
			Error( "missing 'bspnode_back' in node '%s'.\n", node->name );
		BuildVisleafsRecursive( child, portalhm, nodehm, planehm, visleafcls  );
	}
	else
	{
		// it's a leaf
		// has it a flood_state
		pair = FindHPair( node, "flood_state" );
		if ( !pair )
			return;	// no

		// is it flooded
		if ( strcmp( pair->value, "flood" ) )
			return; // no
		

		// it's flood, get all open portals of this leaf
		visleaf = BuildVisleaf( node, portalhm, nodehm, planehm );
		InsertClass( visleafcls, visleaf );

		pair = NewHPair2( "ref", "visleaf", visleaf->name );
		InsertHPair( node, pair );

	}

}

int FixOtherleafs( hobj_t *visleafcls, hmanager_t *nodehm )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	portaliter;
	hobj_t		*visleaf;
	hobj_t		*portal;
	hobj_t		*otherleaf;
	hpair_t		*pair;

	int		visleaf_num = 0;

	InitClassSearchIterator( &iter, visleafcls, "visleaf" );
	for ( ; ( visleaf = SearchGetNextClass( &iter ) ); )
	{
		InitClassSearchIterator( &portaliter, visleaf, "portal" ); 
		for ( ; ( portal = SearchGetNextClass( &portaliter ) ) ; )
		{
			pair = FindHPair( portal, "otherleaf" );
			if ( !pair )
				continue;
			
			otherleaf = HManagerSearchClassName( nodehm, pair->value );
			if ( !otherleaf )
				Error( "portal '%s' can't find otherleaf '%s'.\n", portal->name, pair->value );
			
			pair = FindHPair( otherleaf, "visleaf" );
			if ( !pair )
				Error( "missing 'visleaf' in otherleaf '%s'.\n", otherleaf->name );
			
			RemoveAndDestroyAllHPairsOfKey( portal, "otherleaf" );
			pair = NewHPair2( "ref", "otherleaf", pair->value );
			InsertHPair( portal, pair );
		}
		visleaf_num++;
	}

	return visleaf_num;
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
	char		*in_node_name;
	char		*in_portal_name;
	char		*in_plane_name;
	char		*out_node_name;
	char		*out_visleaf_name;

	hmanager_t		*nodehm;
	hmanager_t		*portalhm;
	hmanager_t		*planehm;

	hobj_t			*visleafcls;

	tokenstream_t		*ts;
	FILE			*h;

	printf( "===== visleaf - build a initial visleaf class for pvs generation =====\n" );
	SetCmdArgs( argc, argv );

	in_portal_name = GetCmdOpt2( "-p" );
	in_node_name = GetCmdOpt2( "-n" );
	in_plane_name = GetCmdOpt2( "-pl" );
	out_node_name = GetCmdOpt2( "-o" );
	out_visleaf_name = GetCmdOpt2( "-v" );

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

	if ( !out_node_name )
	{
		out_node_name = "_visleaf_bspnode.hobj";
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

	if ( !out_visleaf_name )
	{
		out_visleaf_name = "_visleaf_visleaf.hobj";
		printf( " default output visleaf class: %s\n", out_visleaf_name );
	}
	else
	{
		printf( " output visleaf class: %s\n", out_visleaf_name );
	}

	printf( "load portal class ...\n" );
	ts = BeginTokenStream( in_portal_name );
	if ( !ts )
		Error( "can't open portal class.\n" );
	portalhm = NewHManager();
	HManagerSetRootClass( portalhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( portalhm );

	printf( "load bspnode class ...\n" );
	ts = BeginTokenStream( in_node_name );
	if ( !ts )
		Error( "can't open bspnode class.\n" );
	nodehm = NewHManager();
	HManagerSetRootClass( nodehm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( nodehm );

	planehm = ReadPlaneClass( in_plane_name );	

	printf( "build visleafs ...\n" );
	visleafcls = NewClass( "visleafs", "visleafs0" );
	BuildVisleafsRecursive( HManagerGetRootClass( nodehm ), portalhm, nodehm, planehm, visleafcls );
	printf( "fix otherleaf clsref ...\n" );
	printf( " %d visleafs total\n", FixOtherleafs( visleafcls, nodehm ) );

	h = fopen( out_visleaf_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( visleafcls, h );
	fclose( h );

	h = fopen( out_node_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( nodehm ), h );
	fclose( h );

	HManagerSaveID();
}
