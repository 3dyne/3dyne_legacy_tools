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
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
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
	printf( "usage: [ -o outpath ] files ...\n" );
}

void main( int argc, char* argv[] )
{
	int	i, i2, offset;
	int	firstarg;
	
	char*	outpath;
	char	arrname[256];
	char	walname[256];
	char	tmpname[256];
	char*	tmpptr;	
	char	arrident[32];
	unsigned char*	data;

	arr_t*		arr;
	wal_header_t	walheader;

	FILE*	arrhandle;
	FILE*	walhandle;

	outpath = GetCmdOpt( "-o", argc, argv );
	if( outpath == NULL )
	{
		firstarg = 1;
	} else
	{
		firstarg = 3;
	}

	for( i = firstarg; i < argc; i++ )
	{
		strcpy( walname, argv[i] );
		if( strrchr( walname, '.' ) == 0 )
		{
			continue;
		}
		tmpptr = strrchr( argv[i], '.' );
		*tmpptr = '\0';
		sprintf( arrname, "%s.arr", argv[i] );
		if( outpath != NULL )
		{
			strcpy( tmpname, arrname );
			sprintf( arrname, "%s/%s", outpath, tmpname );
		}
		
		printf( "converting %s\tto %s\n", walname, arrname );
		walhandle = fopen( walname, "rb" );
		__chkptr( walhandle );

		fread( &walheader, sizeof( wal_header_t ), 1, walhandle );
		strcpy( arrident, walheader.name_junk );
		arr = ARR_Create( walheader.size_x, walheader.size_y, 4, 1, arrident );
		arrhandle = fopen( arrname, "wb" );
		//strcpy( tmpname, arrname );
		tmpptr = arrname;
		if( arrhandle == NULL )
		{
			for(;;)
			{
				tmpptr = strchr( tmpptr, '/' );
				if( tmpptr != NULL )
				{
					*tmpptr = '\0';
					mkdir( arrname, 511 );
					printf(" mkdir : %s\n", arrname );
					*tmpptr = '/';
					tmpptr++;
				} else
				{
					arrhandle = fopen( arrname, "wb" );
					break;
				}
			}
		}
		
		__chkptr( arrhandle );
		data = ( unsigned char* ) malloc( walheader.size_x * walheader.size_y );
		__chkptr( data );
		offset = 0;
		for( i2 = 0; i2 < 4; i2++ )
		{
			//printf( "offset: %d\n", offset );
		
			fread( data, walheader.size_x * walheader.size_y, 1, walhandle );
			
			memcpy( arr->data+offset, data, walheader.size_x * walheader.size_y );
			offset+=walheader.size_x * walheader.size_y;
			walheader.size_x/=2;
			walheader.size_y/=2;
		}
		ARR_Write( arrhandle, arr );
		ARR_Free( arr );
		free( ( void* )data );
		fclose( walhandle );
		fclose( arrhandle );
	}
}
