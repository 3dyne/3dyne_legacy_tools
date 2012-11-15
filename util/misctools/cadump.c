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



#include <stdio.h>

#include "shock.h"
#include "cmdpars.h"
#include "ca_service.h"

typedef struct {
        char            name[56];
        u_int32_t       loaded;
        size_t          size;
        void*           data;
}       ca_entry_t;


void main( int argc, char* argv[] )
{
	char*	dumpname;
	FILE*	dumphandle;
	int	dumps;
	int	i;
	int	entryoffs;
	char	id[4];
	ca_entry_t	entry;

	unsigned char 	*buf;
	char		*lmpname;
	FILE		*lmphandle;

	dumpname = GetCmdOpt( "-d", argc, argv );
	if( dumpname == NULL )
		__error( "missing arg. no -d\n" );

	lmpname = GetCmdOpt( "-e", argc, argv );

	dumphandle = fopen( dumpname, "rb" );
	__chkptr( dumphandle );
	fread( id, 4, 1, dumphandle );
	if( !memcmp( id, "CA  ", 4 ))
		__warning( "wrong id in dump.\n" );

	fread( &dumps, sizeof( int ), 1, dumphandle );

	for( i = 0; i < dumps; i++ )
	{
		
		fread( &entry, sizeof( ca_entry_t ), 1, dumphandle );
		entryoffs = ftell( dumphandle );
		printf( "chunk name:\t%s\n", entry.name );
		printf( "chunk size:\t%d\n", entry.size );
		printf( "chunk pointer:\t%p\n", entry.data );
		printf( "data position in dump:\t%d\n\n", entryoffs );
		if( lmpname )
			if( !strcmp( lmpname, entry.name ) )
			{
				printf( "extracting lump to lump\n" );
				buf = ( unsigned char* )malloc( entry.size );
				__chkptr( buf );
				fread( buf, entry.size, 1, dumphandle );
				lmphandle = fopen( "lump", "wb" );
				fwrite( buf, entry.size, 1, lmphandle );
				free( buf );
				fclose( lmphandle );
			}
		else 
		{
			fseek( dumphandle, entry.size, SEEK_CUR );
		}
	}
	fclose( dumphandle );
}
			
