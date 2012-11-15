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

#include "arr.h"
#include "pal.h"

#include "shock.h"
#include "cmdpars.h"
#include "cdb_service.h"


void main( int argc, char* argv[] )
{
	int	firstarg;
	int	i;
	int	rgb565 = 0;

	char*	outpath;
	char*	palname;
	char*	tganame;
	char	arrname[256];
	char	tmpstring[256];
	char*	point;

	pid_t	newpid, oldpid;

	outpath = GetCmdOpt( "-o", argc, argv );

	if( outpath == NULL )
	{
		firstarg = 1;
		outpath = ( char* ) malloc( 2 );
		sprintf( outpath, "%s", ".\0" );
	}
	else
		firstarg = 3;

	if( CheckCmdSwitch( "--rgb565", argc, argv ) )
	{
		printf( "rgb565 mode\n" );
		rgb565 = 1;
		firstarg++;
	} else
		rgb565 = 0;


	CDB_StartUp( 0 );
	palname = CDB_GetString( "misc/default_pal" );

	if( palname == NULL )
	{
		__error( "no misc/default_pal found in cdb. please add.\n" );
	}

	oldpid = getpid();

	for( i = firstarg; i < argc; i++ )
	{
		tganame = argv[i];
		sprintf( arrname, "%s/%s", outpath, tganame );
		memcpy( tmpstring, arrname, 256 );

		point = strrchr( tmpstring, '.' );
		*point = '\0';
		sprintf( arrname, "%s.arr", tmpstring );

		if( !fork() )
		{
			printf( "=== %s to %s ===========\n", tganame, arrname );
			if( execlp( "tga2arr", "", "-p", palname, "-t", tganame, "-a", arrname, rgb565 ? "--rgb565" : "--p8", NULL ) == -1 )
			{
				__warning( "exec failed.\n" );
			}
		} else
		{
			//printf( "old process.\n" );
			wait( NULL );
		}
		
	}
}
