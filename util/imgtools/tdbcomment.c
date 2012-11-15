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
#include <string.h>
#include "shock.h"
#include "cmdpars.h"
#include "cdb_service.h"

#include "tdb.h"

void PrintUsage()
{
	printf( "usage:\ntdbupdate -r rawname -c comment\n" );
}

int main( int argc, char* argv[] )
{
	char	*tdbname, *rawname, *comment;
	int	found;
	tdbentry_t*	entries = NULL;

	CDB_StartUp( 0 );

	tdbname = CDB_GetString( "tdb/tdb_name" );
	if( tdbname == NULL )
		__error( "cannot extract key \"tdb/tdb_name\" from cdb. please add.\n" );

	rawname = GetCmdOpt( "-r", argc, argv );
	if( rawname == NULL )
	{
		PrintUsage();
		__error( "missing argument -r\n" );
	}

	comment = GetCmdOpt( "-c", argc, argv );
	if( comment == NULL )
	{
		PrintUsage();
		__error( "missing argument -c\n" );
	}

	TDB_Load( tdbname );

	if( ( found = TDB_Query( NULL, rawname )) == 0 )
	{
		__message( "%s not found\n", rawname );
		exit( 0 );
	}
			
//	printf( "found: %d\n", found );
	if( strlen( comment ) >= 256 )
		comment[255] = '\0';

	TDB_GetPointer( &entries );

	strcpy( entries[matches[0]].comment, comment );
	TDB_Write( tdbname );

	exit( 0 );
	
}
	
