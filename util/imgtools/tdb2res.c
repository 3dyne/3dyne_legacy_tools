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



// tdb2res.c

#include <stdio.h>                                                              
#include "lib_error.h"                                                          
#include "lib_hobj.h"    
#include "tdb.h"                                                                
#include "cdb_service.h"                                                        
  
int main()
{
	int	i;
	char	*tdbname;
	int	entrynum;
	tdbentry_t	*entries;

	hobj_t	*root;
	hobj_t	*tmp;
	hpair_t	*pair;

	FILE	*h;
	char	tt[256];
	char	*ptr;

	CDB_StartUp( 0 );
	tdbname = CDB_GetString( "tdb/tdb_name" );
	if ( !tdbname )
		Error( "cannot find key tdb/tdb_name.\n" );

	TDB_Load( tdbname );

	entrynum = TDB_GetNum();
	TDB_GetPointer( &entries );

	printf( " %d textures\n", entrynum );

	root = NewClass( "resources", "gltex_res" );

	for ( i = 0; i < entrynum; i++ )
	{
		tmp = NewClass( "resource", "gltex" );
		InsertClass( root, tmp );

		sprintf( tt, "textures/%s.arr", entries[i].arr );
		pair = NewHPair2( "string", "path", tt );
		InsertHPair( tmp, pair );

		sprintf( tt, "gltex.%s", entries[i].arr );
		ptr = tt;
		for( ; *ptr ; ptr++ )
		{
			if ( *ptr == '/' )
				*ptr = '.';
		}
		pair = NewHPair2( "string", "name", tt );
		InsertHPair( tmp, pair );
	}

	h = fopen( "gltex_res.class", "w" );
	if ( !h )
		Error( "can't write class\n" );

	WriteClass( root, h );
	fclose( h );
}
