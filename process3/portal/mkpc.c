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



// mkpc.c

#include "mkpc.h"

int GetNodeContents( hobj_t *node )
{
	hpair_search_iterator_t		iter;
	int				num;

	InitHPairSearchIterator( &iter, node, "brush" );
	for ( num = 0; ( SearchGetNextHPair( &iter ) ); num++ )
	{ }

	return num;
}

void MakePortalContents( char *out_portal_name, hmanager_t *portalhm, bool_t vis_solid_only )
{
	hobj_search_iterator_t	iter;
	hobj_t		*portal;
	hobj_t		*frontnode;
	hobj_t		*backnode;
	hpair_t		*pair;
	int		num;
	int		c1, c2;
	int		countclose, countopen;

	FILE		*h;
	char		tt[256];

	countopen = countclose = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( portalhm ), "portal" );
	
	for ( num = 0; ( portal = SearchGetNextClass( &iter ) ); num++ )
	{
		pair = FindHPair( portal, "front_contents" );
		if ( !pair )
			Error( "missing 'front_contents' of portal '%s'.\n", portal->name );
		HPairCastToInt_safe( &c1, pair );

		pair = FindHPair( portal, "back_contents" );
		if ( !pair )
			Error( "missing 'back_contnets' of portal '%s'.\n", portal->name );
		HPairCastToInt_safe( &c2, pair );

		if ( vis_solid_only )
		{
			if ( c1 == c2 || ( c1 < 16 && c2 < 16 ) )
			{
				countopen++;
				pair = NewHPair2( "string", "state", "open" );
				InsertHPair( portal, pair );	
			}
			else if ( (c1 == 16 && c2 < 16) || (c2 == 16 && c1 < 16 ) )
			{
				countclose++;
				pair = NewHPair2( "string", "state", "closed" );
				InsertHPair( portal, pair );	
			}
			else
				Error( "? c1 %d, c2 %d\n", c1, c2 );
		}
		else
		{
			if ( c1 == c2 )
			{
				countopen++;
				pair = NewHPair2( "string", "state", "open" );
				InsertHPair( portal, pair );
			}
			else
			{
				countclose++;
				pair = NewHPair2( "string", "state", "closed" );
				InsertHPair( portal, pair );
			}
		}
	}
	
	printf( " %d portals.\n", num );
	printf( " %d are open.\n", countopen );
	printf( " %d are closed.\n", countclose );

	printf( "write portal class ...\n" );
	h = fopen( out_portal_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( portalhm ), h );
	fclose( h );
}

int main( int argc, char *argv[] )
{
	char		*in_portal_name;
	char		*out_portal_name;
	
	tokenstream_t	*ts;

	hobj_t		*portalcls;

	hmanager_t	*portalhm;

	printf( "===== mkpc - make portal contents =====\n" );

	SetCmdArgs( argc, argv );

	in_portal_name = GetCmdOpt2( "-p" );
//	in_brush_name = GetCmdOpt2( "-b" );
	out_portal_name = GetCmdOpt2( "-o" );
	

	if ( !in_portal_name )
	{
		in_portal_name = "_portalout_portal.hobj";
		printf( " default input portal class: %s\n", in_portal_name );
	}
	else
	{
		printf( " input portal class: %s\n", in_portal_name );	
	}

	if ( !out_portal_name )
	{
		out_portal_name = "_mkpcout_portal.hobj";
		printf( " default output portal class: %s\n", out_portal_name );
	}
	else
	{
		printf( " output portal class: %s\n", out_portal_name );
	}

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

	printf( "make portal contents ...\n" );

	if ( CheckCmdSwitch2( "--vis-solid-only" ) )
	{
		printf( "Switch: --vis-solid-only\n" );
		MakePortalContents( out_portal_name, portalhm, true );
	}
	else
	{
		MakePortalContents( out_portal_name, portalhm, false );
	}
       
}
