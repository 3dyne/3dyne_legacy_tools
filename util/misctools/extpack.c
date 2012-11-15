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



// extpack.c
// tool to extract one 'lump' from an id PACKfile

#include <stdio.h> 
#include <sys/types.h>
#include <string.h>
#include <stdlib.h>
#include "shock.h"
#include "cmdpars.h"

// stuff from qfiles, Qutils
typedef struct
{
        char    name[56];
        int             filepos, filelen;
} packfile_t;

typedef struct
{
        char    id[4];
        int             dirofs;
        int             dirlen;
} packheader_t;

static packfile_t	files[10000];

int main( int argc, char* argv[] )
{
	char*	pack_name;
	char*	file_name;
	char*	out_name;
	char	list_name[256];
	char*	dir_path;

	unsigned char*	buf = NULL;

	u_int32_t	i;
	char*		temp_ptr;
	u_int32_t	pack_filenum;
	int		pack_megs = 0;
	int32_t	file_ind = -1;

	FILE*	pack_handle;
	FILE*	out_handle;
	FILE*	list_handle;
	packheader_t	header;

	pack_name = GetCmdOpt( "-p", argc, argv );
	if( pack_name == NULL )
	{
		__named_message( "no pack given.\n" );
	}	

	file_name = GetCmdOpt( "-f", argc, argv );
	if( (file_name == NULL) && ( !CheckCmdSwitch( "-a", argc, argv )) )
	{
		__error( "no file given.\n" );
	}

	out_name = GetCmdOpt( "-o", argc, argv );
	CHKPTR( out_name );
	

	pack_handle = fopen( pack_name, "rb" );
	
	CHKPTR( pack_name );
	
	fread( &header, sizeof( packheader_t ), 1, pack_handle );

	printf( "id: %c%c%c%c dirofs: %i dirlen: %i filenum: %i\n", header.id[0], header.id[1], header.id[2], header.id[3], header.dirofs, header.dirlen, header.dirlen / sizeof( packfile_t ) );
	if( memcmp( "PACK", header.id, 4 ) )
	{
		__error( "this doesn't seem to be a packfile.\n" );
	}
	
	MESSAGE( "trying to read directory ...\n" );
	pack_filenum = header.dirlen / sizeof( packfile_t );
	fseek( pack_handle, header.dirofs, SEEK_SET );
	
	for( i = 0; i < pack_filenum; i++ )
	{
		fread( &files[i], sizeof( packfile_t ), 1, pack_handle );
		pack_megs += files[i].filelen;
	}
	MESSAGE( "the end\n" );
	
	if( CheckCmdSwitch( "-a", argc, argv ) )
	{
		__message( "do you really want to extract a whole PACK with %d files( %d megs )?\n", pack_filenum, pack_megs/1000000 );
		if( getchar() == 'n' )
		{
			__message( "exiting due to 'n'.\n" );
			exit( 0 );
		}
		dir_path = GetCmdOpt( "-a", argc, argv );
		__chkptr( dir_path );
		out_name = ( char* ) malloc( 256 );
		for( i = 0; i < pack_filenum; i++ )
		{
			
			
			buf = ( unsigned char* ) realloc( buf, files[i].filelen );
			__chkptr( buf );
			sprintf( out_name, "./%s/%s", dir_path, files[i].name );
			out_handle = fopen( out_name, "wb" );
			temp_ptr = out_name;
			if( out_handle == NULL )
			{
				for(;;)
				{
					temp_ptr = strchr( temp_ptr, '/' );
					if( temp_ptr != NULL )
					{
						*temp_ptr = '\0';
						mkdir( out_name, 511 );
						printf(" mkdir : %s\n", out_name );
						*temp_ptr = '/';
						temp_ptr++;
					} else
					{
						out_handle = fopen( out_name, "wb" );
						break;
					}
				}
			}
			printf( "%s\n", out_name );
			
			//printf(" fopen errno: %d\n", errno );
			__chkptr( out_handle );
			fseek( pack_handle, files[i].filepos, SEEK_SET );
						
			fread( buf, files[i].filelen, 1, pack_handle );
			fwrite( buf, files[i].filelen, 1, out_handle );
			fclose( out_handle );
		}
	}
	else
	{
		MESSAGE( "trying to extract file %s ...\n", file_name );
		MESSAGE( "\tsearching ..." );
		for( i = 0; i < pack_filenum; i++ )
		{
			if( !strcmp( files[i].name, file_name ) )
			{
				file_ind = i;
				MESSAGE( "found\n" );
			break;
			}
		}
		if( file_ind == -1 )
		{
			MESSAGE( "\n" );
			ERROR( "file not found.\n" );
		}
		MESSAGE( "\tfile index: %i size: %i\n", file_ind, files[file_ind].filelen );
		
		
		MESSAGE( "writing out file... \n" );
		fseek( pack_handle, files[file_ind].filepos, SEEK_SET );
		out_handle = fopen( out_name, "wb" );
		CHKPTR( out_handle );
		
		buf = ( unsigned char* ) malloc( files[file_ind].filelen );
		CHKPTR( buf );
		
		fread( buf, files[file_ind].filelen, 1, pack_handle );
		fwrite( buf, files[file_ind].filelen, 1, out_handle );
		fclose( out_handle );
	}
	if( CheckCmdSwitch( "-l", argc, argv ) )
	{
		MESSAGE( "writing file list ...\n" );
		sprintf( list_name, "%s.lst", pack_name );		
		list_handle = fopen( list_name, "wb" );
		fprintf( list_handle, "pack list(c) from file '%s'.\n", pack_name );
		for( i = 0; i < pack_filenum; i++ )
		{
			fprintf( list_handle, "%s\t%i kb\n", files[i].name, files[i].filelen/1024 );
		}
		fprintf( list_handle, "end.\ninfo: %i files.\n", pack_filenum );
		fclose( list_handle );
	}
	
	fclose( pack_handle );
	exit( 0 );
}
