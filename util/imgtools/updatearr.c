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
#include <unistd.h>
#include <sys/types.h>
#include <dirent.h>

#include "shock.h"
#include "arr.h"
#include "cmdpars.h"

void UpdateDir( char* type )
{
	DIR*		subdir;
	FILE*		arrhandle;

	struct dirent*	dirent;
	char		arrname[256];
	char		ident[256];
	char		name[256];

	char*		point;

	int	i = 0;


	arr_header_l	arrheader;

	if( chdir( type ) == -1 )
	{
		__message( "couldn't chdir to %s, skipping.\n", type );
		return;
	}
	subdir = opendir( "." );
	while(( dirent = readdir( subdir )) != NULL )
	{
		if( !(strcmp( dirent->d_name, "." ) && strcmp( dirent->d_name, ".." )))
		{
			continue;
		}
		strcpy( name, dirent->d_name );
		/*
		  strrev( name );
		  if( memcmp( name, "rra", 3 ))
		  {
		  continue;
		  }
		  strrev( name );
		*/
		//sprintf( arrname, "%s/%s", type, name );
		arrhandle = fopen( name, "r+" );
		__chkptr( arrhandle );
		fread( &arrheader, sizeof( arr_header_l ), 1, arrhandle );
		//fclose( arrhandle );
		if( memcmp( arrheader.id, ARR_ID, 4 ) )
		{
			__message( "\tfile %s is not an arr.\n", name );
			continue;
		}
		strcpy( arrname, name );
		point = strrchr( name, '.' );
		if( point == NULL )
		{
			__message( "\tcannot cut suffix: %s skipping.\n", name );
			continue;
		}
		*point = '\0';
		sprintf( ident, "%s/%s", type, name );
		//printf( "\tident: %s\n", ident );
		if( strlen( ident ) >= 32 )
		{
			ident[31] = '\0';
		}
		strcpy( arrheader.ident, ident );
		//arrhandle = fopen( arrname, "r+" );
		//__chkptr( arrhandle );
		fseek( arrhandle, 0, SEEK_SET );
		fwrite( &arrheader, sizeof( arr_header_l ), 1, arrhandle );
		fclose( arrhandle );
		i++;
	}
	//__message( "\t%d files\n", i );
	closedir( subdir );
	chdir( ".." );
	
}

void main( int argc, char* argv[] )
{
	DIR*		dir;
	
	
	struct dirent*	dirent;
	char		type[256];
	
	char*		path;
	char*		forcedir;

	path = GetCmdOpt( "-p", argc, argv );
	if( path == NULL )
	{
		__message( "no texture path given. use -p.\n" );
		exit( 0 );
	}
	__message( "updating arrs in %s ...\n", path );
	if( chdir( path ) == -1 )
	{
		__error( "couldn't chdir to %s\n", path );
	}
	if( ( forcedir = GetCmdOpt( "-f", argc, argv )) != NULL )
	{
		__message( "forcing dir %s\n", forcedir );
		UpdateDir( forcedir );
	} else
	{

		dir = opendir( "." );
		
		while(( dirent = readdir( dir )) != NULL )
		{
			if(! (strcmp( dirent->d_name, "." ) && strcmp( dirent->d_name, ".." )))
			{
				continue;
		}
			strcpy( type, dirent->d_name );
			printf( "updating dir %s\n", type );
			UpdateDir( type );
		}
		closedir( dir );
	}
}
