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



// node_debug.c


void WritePortals( hobj_t *portal, FILE *h )
{
	hpair_search_iterator_t		iter;

	hpair_t			*pair;

	int			i, num;
	char		tt[256];

	fp_t		scale;
	vec3d_t		center;

		
	pair = FindHPair( portal, "pointnum" );
	if ( !pair )
		Error( "missing 'pointnum' in portal '%s'.\n", portal->name );
	HPairCastToInt_safe( &num, pair );
	
	Vec3dInit( center, 0, 0, 0 );
	for ( i = 0; i < num; i++ )
	{
		vec3d_t		v;
		
		sprintf( tt, "%d", i );
		pair = FindHPair( portal, tt );						
		if ( !pair )
			Error( "missing point '%s' in portal '%s'.\n", tt, portal->name );
		HPairCastToVec3d_safe( v, pair );
		Vec3dAdd( center, center, v );
	}
	scale = 1.0 / num;
	Vec3dScale( center, scale, center );
	fprintf( h, "( %f %f %f ) ", center[0], center[1], center[2] );	
}

void WriteNodePortals( hobj_t *node, hmanager_t *portalhm, FILE *h )
{
	hpair_search_iterator_t		iter;

	hpair_t			*pair;
	hobj_t			*portal;	

	int			i, num;
	char		tt[256];

	
	InitHPairSearchIterator( &iter, node, "portal" );
	
	for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
	{
		portal = HManagerSearchClassName( portalhm, pair->value );
		if ( !portal )
			Error( "leaf '%s' can't find portal '%s'.\n", node->name, pair->value );
		
		pair = FindHPair( portal, "pointnum" );
		if ( !pair )
			Error( "missing 'pointnum' in portal '%s'.\n", portal->name );
		HPairCastToInt_safe( &num, pair );

		for ( i = 0; i < num; i++ )
		{
			sprintf( tt, "%d", i );
			pair = FindHPair( portal, tt );						
			if ( !pair )
				Error( "missing point '%s' in portal '%s'.\n", tt, portal->name );
			fprintf( h, "( %s ) ", pair->value );
		}
	}
}
