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

#include "pvs.h"

/*
  ====================
  allocation

  ====================
*/


int	portal_num = 0;
int	visleaf_num = 0;

portal_t * NewPortal( void )
{
	portal_num++;
	return NEW( portal_t );
}

void FreePortal( portal_t *p )
{
	portal_num--;
	free( p );
}

visleaf_t * NewVisleaf( void )
{
	visleaf_num++;
	return NEW( visleaf_t );
}

void FreeVisleaf( visleaf_t *v )
{
	visleaf_num--;
	free( v );
}




/*
  ====================
  CompileVisleafClasses

  ====================
*/
portal_t * CompilePortalClass( hobj_t *portal, hmanager_t *planehm )
{
	portal_t		*pt;
	int		i, num;
	hobj_t		*plane;
	hpair_t		*pair;
	char		tt[256];

	pt = NewPortal();
	pt->self = portal;
	SetClassExtra( portal, pt );

	pair = FindHPair( portal, "plane" );
	if ( !pair )
		Error( "missing 'plane' in portal '%s'.\n", portal->name );
	plane = HManagerSearchClassName( planehm, pair->value );
	if ( !plane )
		Error( "portal '%s' can't find plane '%s'.\n", portal->name, pair->value );
	pt->pl = GetClassExtra( plane );

	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' in portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );
	pt->p = NewPolygon( num );
	pt->p->pointnum = num;
	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( portal, tt );
		if ( !pair )
			Error( "missing point '%s' in portal '%s'.\n", tt, portal->name );
		HPairCastToVec3d_safe( pt->p->p[i], pair );
	}

	return pt;
}

visleaf_t * CompileVisleafClasses( hmanager_t *visleafhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	portaliter;
	hobj_t		*visleaf;
	hobj_t		*portal;
	hobj_t		*otherleaf;
	hpair_t		*pair;

	visleaf_t	*head;
	visleaf_t	*vl;

	portal_t	*pt;
	int		num;
	
	// build initial list
	head = NULL;
	InitClassSearchIterator( &iter, HManagerGetRootClass( visleafhm ), "visleaf" );
	for ( ; ( visleaf = SearchGetNextClass( &iter ) ); )
	{
		vl = NewVisleaf();
		vl->self = visleaf;
		SetClassExtra( visleaf, vl );

		vl->next = head;
		head = vl;

		InitClassSearchIterator( &portaliter, visleaf, "portal" );
		for ( ; ( portal = SearchGetNextClass( &portaliter ) ) ; )
		{
			pt = CompilePortalClass( portal, planehm );
			pt->next = vl->portals;
			vl->portals = pt;			
		}		
	}

	// resolve clsref 'otherleaf'
	// and set bitpos of leaf
	for ( vl = head, num = 0; vl ; vl=vl->next, num++ )
	{
		vl->bitpos = num;
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			pair = FindHPair( pt->self, "otherleaf" );
			if ( !pair )
			{
				pt->otherleaf = NULL;
				continue;
			}
			otherleaf = HManagerSearchClassName( visleafhm, pair->value );
			if ( !otherleaf )
				Error( "portal '%s' can't find otherleaf '%s'.\n", pt->self->name, pair->value );
			pt->otherleaf = GetClassExtra( otherleaf );			
		}
	}

	return head;
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

/*
  ==================================================
  debug draw

  ==================================================
*/

void DrawVisleaf( visleaf_t *leaf )
{
	vec3d_t		center;
	polygon_t	*pnorm;
	portal_t	*pt;
	pnorm = NewPolygon( 3 );
	pnorm->pointnum = 3;

	for ( pt = leaf->portals; pt ; pt=pt->next )
	{
		GLC_DrawPolygon( pt->p );
		PolygonCenter( pt->p, center );
		
		Vec3dCopy( pnorm->p[0], center );
		Vec3dCopy( pnorm->p[1], center );
		Vec3dMA( pnorm->p[2], 16.0, pt->pl->norm, center );
		GLC_DrawPolygon( pnorm );
	}

	FreePolygon( pnorm );
}

void DrawAll( visleaf_t *list )
{
	visleaf_t	*vl;
	portal_t	*pt;

	GLC_ConnectServer("");

	GLC_BeginList( "portals", 1 );
	for ( vl = list; vl ; vl=vl->next )
	{
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			
			GLC_DrawPolygon( pt->p );

		}
	}
	GLC_EndList();

	GLC_DisconnectServer();
}

hmanager_t	*global_visleafhm;
int main( int argc, char *argv[] )
{
	char		*in_visleaf_name;
	char		*in_plane_name;
	char		*out_visleaf_name;

	hmanager_t	*planehm;
	hmanager_t	*visleafhm;

	tokenstream_t	*ts;
	FILE		*h;

	visleaf_t		*visleaflist;

	printf( "===== pvs - build potential visibility set for visleafs =====\n" );
	SetCmdArgs( argc, argv );

	in_visleaf_name = GetCmdOpt2( "-i" );
	in_plane_name = GetCmdOpt2( "-pl" );
	out_visleaf_name = GetCmdOpt2( "-o" );

	if ( !in_visleaf_name )
	{
		in_visleaf_name = "_visleaf_visleaf.hobj";
		printf( " default input visleaf class: %s\n", in_visleaf_name );
	}
	else
	{
		printf( " input visleaf class: %s\n", in_visleaf_name );
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
		out_visleaf_name = "_pvsout_visleaf.hobj";
		printf( " default output visleaf class: %s\n", out_visleaf_name );
	}
	else
	{
		printf( " output visleaf class: %s\n", out_visleaf_name );
	}

	planehm = ReadPlaneClass( in_plane_name );

	printf( "load visleaf class ...\n" );
	ts = BeginTokenStream( in_visleaf_name );
	if ( !ts )
		Error( "can't open file.\n" );
	visleafhm = NewHManager();
	HManagerSetRootClass( visleafhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( visleafhm );
	
	printf( "compile visleaf class ...\n" );
	visleaflist = CompileVisleafClasses( visleafhm, planehm );
	printf( " %d visleafs\n", visleaf_num );
	printf( " %d portals\n", portal_num );

#ifdef GLC
	DrawAll( visleaflist );
#endif

	global_visleafhm = visleafhm;

	TrivialReject( visleaflist );
	if ( !CheckCmdSwitch2( "--trivial-only" ) )
		ComplexReject( visleaflist ); 

	printf( "write visleaf class ...\n" );       
	h = fopen( out_visleaf_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( visleafhm ), h );
	fclose( h );

}	
