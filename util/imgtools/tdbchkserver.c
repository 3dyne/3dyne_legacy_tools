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

void main()
{
	FILE*	handle;
	char	*texturepath, fname[128];
	struct stat	finfo;

	printf( "this programm will find out the timedifference between\n" );
	printf( "the local (tgas) and server (arrs) machine.\n" );
	CDB_StartUp( 1 );
	texturepath = CDB_GetString( "tdb/texture_path" );
	if( texturepath == NULL )
		__error( "cannot extract key \"tdb/texture_path\" from cdb. please add.\n" );
	sprintf( fname, "%s/chks.tmp", texturepath ); 
	printf( "testfile is %s\n", fname );
	handle = fopen( fname, "wb" );
	fprintf( handle, "this is a temporary file used by tdbchkserver.\n delete it!\n" );
	fclose( handle );
	//handle = fopen( fname, "rb" );
	if( stat( fname, &finfo ))
		__error( "cannot get modification time of %s\n", fname );
			 

	printf( "local time is %d\nserver time is %d\n", time( NULL ), finfo.st_mtime );
	
}
	
