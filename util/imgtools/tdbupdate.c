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
#include <time.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include "shock.h"
#include "cmdpars.h"
#include "cdb_service.h"

#include "tdb.h"

void PrintUsage()
{
	printf( "usage:\n tdbupdate [-na (don't rebuild animations)]\n" );
}

int main( int argc, char* argv[] )
{
	int	i, i2;
	int	found = 2;
//	int	noanim;
	int	rebuildanim = 0;
	tdbentry_t*	entries = NULL;
	time_t	arrtime, tgatime;
	struct stat	finfo;
	char	arrname[256];
	char	*creator, *texturepath, *tdbname, *point, *artpath;
	FILE*	animhandle;
	char	tga[256];
	char*	arrs[256];
	char	tmp[256];
	
	CDB_StartUp( 0 );
	tdbname = CDB_GetString( "tdb/tdb_name" );
	if( tdbname == NULL )
		__error( "cannot extract key \"tdb/tdb_name\" from cdb. please add.\n" );
	
	creator = CDB_GetString( "tdb/creator" );
	if( creator == NULL )
		__error( "cannot extract key \"tdb/creator\" from cdb. please add.\n" );

	texturepath = CDB_GetString( "tdb/texture_path" );
	if( texturepath == NULL )
		__error( "cannot extract key \"tdb/texture_path\" from cdb. please add.\n" );

	artpath = CDB_GetString( "tdb/art_path" );
	if( artpath == NULL )
		__error( "cannot extract key \"tdb/art_path\" from cdb. please add.\n" );
	
	//if( (noanim = CheckCmdSwitch( "-na", argc, argv )) == 1)
	//	__message( "no animations is on.\n" );
	
/*
	if( CheckCmdSwitch( "-h", argc, argv ))
	{
		PrintUsage();
		exit( 0 );
	}
	*/    

	TDB_Load( tdbname );
	__message( "searching changes ...\n" );
	TDB_GetPointer( &entries );

	found = TDB_Query( creator, NULL );
	//printf( "found: %d\n", found );

	for( i = 0; i < found; i++ )
	{
		sprintf( arrname, "%s/%s.arr", texturepath, entries[matches[i]].arr );
		//printf( "%s\n", arrname );
		if( stat( arrname, &finfo ))
			arrtime = 0;
		else
			arrtime = finfo.st_mtime;
		
		point = strrchr( entries[matches[i]].tga, '.' );
		if( point == NULL )
			__error( "no suffix found: %s\n",  entries[matches[i]].tga );

		if( !strcmp( point+1, "tga" ))
		{
			if( stat( entries[matches[i]].tga, &finfo ))
			{
				printf( "cannot get modification time of %s\n", entries[matches[i]].tga );
				continue;
			}
			tgatime = finfo.st_mtime;
			
			if( tgatime > arrtime )
			{
				//printf( "arr %s needs update\n", entries[matches[i]].arr );
				if( !fork() )
				{
					printf( "=== %s is updated to %s ===========\n", entries[matches[i]].tga, arrname );
					if( execlp( "tga2arr", "", "-t", entries[matches[i]].tga, "-a", arrname, "-i", entries[matches[i]].arr, "--rgb565", NULL ) == -1 )
						__warning( "exec failed.\n" );
				} else
					wait( NULL );
				
			}
			
		} else if( !strcmp( point+1, "anim" ))
		{
			// **********************************************************
			// * warning: don't cross this line
			// * do not try to understand the following code
			// * be glad that it works. if it doesn't, write a better one
			// **********************************************************
			//printf( "updating animation ...\n" );
			
			animhandle = fopen( entries[matches[i]].tga, "r" );
			if( animhandle == NULL )
				__error( "animation not found: %s\n", entries[matches[i]].tga );
			arrs[0] = ( char* )malloc( 32 );  // building argument list for
			arrs[1] = ( char* )malloc( 32 );  // the execlp call
			strcpy( arrs[1], "-o" );
			arrs[2] = ( char* )malloc( 128 );
			strcpy( arrs[2], arrname );
			arrs[3] = ( char* )malloc( 32 );
			strcpy( arrs[3], "-i" );
			arrs[4] = ( char* )malloc( 32 );
			strcpy( arrs[4], entries[matches[i]].arr );
			for( i2 = 0;; i2++ )
			{
				fscanf( animhandle, "%s", tmp );
				if( feof( animhandle ))
					break;
				sprintf( tga, "%s/%s.tga", artpath, tmp );
				arrs[i2+5] = ( char* )malloc( 128 );
				arrs[i2+6] = NULL;
				sprintf( arrs[i2+5], "%s/%s.arr", artpath, tmp );
//				printf( "%s %s\n", tgas[i2], arrs[i2] );
				if( stat( arrs[i2+5], &finfo ))
					arrtime = 0;
				else
					arrtime = finfo.st_mtime;
				
				if( stat( tga, &finfo ))
					__error( "tga not found: %s ( named in %s )\n", tga, entries[matches[i]].tga );
				tgatime = finfo.st_mtime;
				if( tgatime > arrtime )
				{
					rebuildanim = 1;
					if( !fork() )
					{
						if( execlp( "tga2arr", "", "-t", tga, "-a", arrs[i2+5], "--rgb565", NULL ) == -1 )
							__warning( "exec failed.\n" );
					} else
						wait( NULL );
				}
			}
			if( rebuildanim )
			{
				rebuildanim = 0;
				__message( "making animation ...\n" );
				if( !fork() )
				{
					if( execvp( "arr2anim", arrs ) == -1 )
						__warning( "exec failed.\n" );
				} else
					wait( NULL );
			}
		} else 
			__error( "this should not happen\n" );
	}
}
