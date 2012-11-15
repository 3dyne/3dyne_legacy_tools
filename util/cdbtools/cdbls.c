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
#include "cdb_service.h"
#include "cmdpars.h"


void main( int argc, char* argv[] )
{
	
	string_entry_l*		string_entries = NULL;
	int_entry_l*		int_entries = NULL;
	int		string_num;
	int		int_num;
	int		i;

	CDB_StartUp( 0 );
	CDB_GetPointers( &string_entries, &int_entries, &string_num, &int_num );
	printf( "string num:\t%d\nint num:\t%d\n", string_num, int_num );
	
//	printf( "argc: %d\n", argc );
	if( (argc == 1) || CheckCmdSwitch( "-s", argc, argv ))
	{
		printf( "string:\n" ); 
		if( ! string_entries )
		{
			printf( "cannot get strings from cdb\n" );
			exit( -1 );
		}
		for( i = 0; i < string_num; i++ )
		{
			printf( "%s:\t%s\n", string_entries[i].key, string_entries[i].data );
		}
	}
	if( (argc == 1) || CheckCmdSwitch( "-i", argc, argv ))
	{
		printf( "int:\n" );
		for( i = 0; i < int_num; i++ )
		{
			printf( "%s:\t%i\n", int_entries[i].key, int_entries[i].value );
		}
	}
}
	
