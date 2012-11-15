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
#include "cdb_service.h"

#include "tdb.h"

int main( int argc, char* argv[] )
{
	char	*creator, arr[256], tga[256], *comment, *rawname, *artpath, *tdbname;
	
	CDB_StartUp( 0 );
	creator = CDB_GetString( "tdb/creator" );
	if( creator == NULL )
		__error( "cannot extract key \"tdb/creator\" from cdb. please add.\n" );

	artpath = CDB_GetString( "tdb/art_path" );
	if( artpath == NULL )
		__error( "cannot extract key \"tdb/art_path\" from cdb. please add.\n" );

	rawname = GetCmdOpt( "-r", argc, argv );
	if( rawname == NULL )
	{
		__error( "missung argument -r rawname\n" );
	}
	if( strlen( rawname ) >= 32 )
		__error( "rawname has more than 31 characters\n" );

	if( !CheckCmdSwitch( "-a", argc, argv ))
		sprintf( tga, "%s/%s.tga", artpath, rawname );
	else
		sprintf( tga, "%s/%s.anim", artpath, rawname );

	strcpy( arr, rawname );
	
	comment = GetCmdOpt( "-c", argc, argv );
	if( comment == NULL )
	{
		__message( "no comment given\n" );
		comment = ( char* )malloc( 1 );
		comment[0] = '\0';
	}

	tdbname = CDB_GetString( "tdb/tdb_name" );
	if( tdbname == NULL )
		__error( "cannot extract key \"tdb/tdb_name\" from cdb. please add.\n" );
	
	TDB_Load( tdbname );
	if( TDB_AddEntry( creator, arr, tga, comment ) == TDB_ENTRYEXISTS )
		__message( "entry exists\n" );
	TDB_Write( tdbname );
}
