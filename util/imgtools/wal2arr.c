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



#include "cmdpars.h"
#include "shock.h"
#include <sys/types.h>
#include "arr.h"

typedef struct {
	char	name_junk[32];
	u_int32_t	size_x;
	u_int32_t	size_y;
	char	unknown_junk[60];
}	wal_header_t;

void PrintUsage()
{
	printf( "usage: wal2arr -w wal -a arr [ -i ident ]\n" );
}

void main( int argc, char* argv[] )
{
	arr_t*	arr;
       	wal_header_t	wal_head;

	FILE*	wal_handle;
	FILE*	arr_handle;
	unsigned char*	data;
	int	i, offset;
	
	char*	wal_name;
	char*	arr_name;
	char*	arr_ident;
	
	wal_name = GetCmdOpt( "-w", argc, argv );
	if( wal_name == NULL )
	{
		PrintUsage();
		__error( "no wal given.\n" );
	}
	
	arr_name = GetCmdOpt( "-a", argc, argv );
	if( arr_name == NULL )
	{
		PrintUsage();
		__error( "no arr given.\n" );
	}
 
	wal_handle = fopen( wal_name, "rb" );
	__chkptr( wal_handle );

	arr_handle = fopen( arr_name, "wb" );
	__chkptr( arr_handle );

	__message( "open wal.\n" );
	fread( &wal_head, sizeof( wal_header_t ), 1, wal_handle );
	__message( "wal:\nname_junk: %s\nsize x: %i\nsize y: %i\n", wal_head.name_junk, wal_head.size_x, wal_head.size_y );
	
	arr_ident = GetCmdOpt( "-i", argc, argv );
	if( arr_ident == NULL )
	{
		__warning( "no arr_ident given taking it from wal.\n" );
		arr_ident = ( char* ) malloc( 32 );
		memcpy( arr_ident, wal_head.name_junk, 32 ); 
	}
	
	arr = ARR_Create( wal_head.size_x, wal_head.size_y, 4, 1, arr_ident );

	offset = 0;
	for( i = 0; i < 4; i++ )
	{
		//printf( "offset: %d\n", offset );
		data = ( unsigned char* ) malloc( wal_head.size_x * wal_head.size_y );
		fread( data, wal_head.size_x * wal_head.size_y, 1, wal_handle );

		memcpy( arr->data+offset, data, wal_head.size_x * wal_head.size_y );
		offset+=wal_head.size_x * wal_head.size_y;
		wal_head.size_x/=2;
		wal_head.size_y/=2;
		//free( data );
	}

	ARR_Write( arr_handle, arr );
	ARR_Free( arr );
	free( data );


	fclose( wal_handle );
	fclose( arr_handle );
}
