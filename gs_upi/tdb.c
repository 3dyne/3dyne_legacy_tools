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
#include <string.h>

#include "tdb.h"
#include "cdb_service.h"
#include "lib_error.h"

int	matches[TDB_MAXENTRIES];
static tdbentry_t	entries[TDB_MAXENTRIES];
static int		entrynum = 0;

void	TDB_Load( char* arg_tdbname )
{
	tdbheader_t	tdbheader;
	FILE*		tdbhandle;
//	int		entrienum = 0;
	int		realentries = 0;

	if( !arg_tdbname )
	    Error( "arg_tdbname == NULL\n" );
	
	tdbhandle = fopen( arg_tdbname, "rb" );
	if( tdbhandle == NULL )
	{
		printf( "no texture database found at %s\nno data loaded.\n", arg_tdbname );
		return;
	}
	
	fread( &tdbheader, sizeof( tdbheader_t ), 1, tdbhandle );
	if( memcmp( tdbheader.id, "TDB ", 4 ))
	{
		Error( "file is not a tdb / is corrupted.\n" );
	}
	
	entrynum = tdbheader.entrynum;
	if( tdbheader.entrynum > TDB_MAXENTRIES )
	{
		printf( "tdb has to many entries. recompile!\n" );
		entrynum = TDB_MAXENTRIES;
	}

	printf( "reading database ...\n" );
	if( ( realentries = fread( entries, sizeof( tdbentry_t ), entrynum, tdbhandle )) != entrynum )
	{
		printf( "there is something wrong with the size / entrynum of your tdb.\n\tfixed. [entrynum %d, former: %d]\n", realentries, entrynum );
		entrynum = realentries;
	}
	fclose( tdbhandle );
}

void TDB_Write( char* arg_tdbname )
{
	tdbheader_t	tdbheader;
	FILE*	tdbhandle;
	
//	__chkptr( arg_tdbname );
	tdbhandle = fopen( arg_tdbname, "wb" );
	if( tdbhandle == NULL )
		Error( "couldn't create database.\n" );

	memcpy( tdbheader.id, "TDB ", 4 );
	tdbheader.entrynum = entrynum;

	fwrite( &tdbheader, sizeof( tdbheader_t ), 1, tdbhandle );
	fwrite( entries, sizeof( tdbentry_t ), entrynum, tdbhandle );
	fclose( tdbhandle );
}

void TDB_GetPointer( tdbentry_t** arg_entries )
{
//	__chkptr( entries );
	*arg_entries = entries;
}
int     TDB_GetNum( void )
{
	return entrynum;
}
int TDB_AddEntry( char* creator, char* arr, char* tga, char* comment )
{
	int	i;

//	__chkptr( creator );
//	__chkptr( arr );
//	__chkptr( tga );
//	__chkptr( comment );
	if( strlen( creator ) >= 32 )
		creator[31] = '\0';

	if( strlen( arr ) >= 32 )
		arr[31] = '\0';
	
	if( strlen( tga ) >= 256 )
		tga[255] = '\0';
	
	if( strlen( comment ) >= 256 )
		comment[255] = '\0';

	if( entrynum >= TDB_MAXENTRIES )
		Error( "not enough entries in database. recompile!\n" );

	for( i = 0; i < entrynum; i++ )
	{
		if( !strcmp( entries[i].arr, arr ) )
		{
			printf( "entry %s exists!\ncreator:\t%s\ntga:\t%s\ncomment:\t%s\n", arr, entries[i].creator, entries[i].tga, entries[i].comment );
			return  TDB_ENTRYEXISTS;
		}
	}
			

	entries[entrynum].creationtime = ( int ) time( NULL );
	strcpy( entries[entrynum].creator, creator );
	strcpy( entries[entrynum].arr, arr );
	strcpy( entries[entrynum].tga, tga );
	strcpy( entries[entrynum].comment, comment );
	entrynum++;
	return TDB_ENTRYOK;
}

int TDB_RemoveEntry( char* arr )
{
	int	i;

//	__chkptr( arr );
	for( i = 0; i < entrynum; i++ )
	{
		if( !strcmp( entries[i].arr, arr ))
		{
			printf( "deleting %d\n", i );
			memcpy( &entries[i], &entries[entrynum-1], sizeof( tdbentry_t ));
			entrynum--;
			return i;
		}
	}
	return -1;
}

int TDB_Query( char* arg_creator, char* arg_name )
{
	int	i, found;

	found = 0;
	
	//__chkptr( arg_creator );
	if( arg_name == NULL && arg_creator == NULL )
		return 0;

	if( arg_name == NULL)
	{
		for( i = 0; i < entrynum; i++ )
		{
			if( !strcmp( arg_creator, entries[i].creator ))
			{
				matches[found] = i;
				found++;
			}
		}
	} else if( arg_creator == NULL )
	{
		for( i = 0; i < entrynum; i++ )
		{
			if( !strcmp( arg_name, entries[i].arr ))
			{
				matches[found] = i;
				found++;
			}
		}
	}
	if( arg_creator != NULL && arg_name != NULL )
	{
		for( i = 0; i < entrynum; i++ )
		{
			if( !( strcmp( arg_name, entries[i].arr ) || strcmp( arg_creator, entries[i].creator) ))
			{
				matches[found] = i;
				found++;
			}
		}
	}

	return found;
}

void TDB_Ls()
{
	int	i;

	for( i = 0; i < entrynum; i++ )
	{
		printf( "%s\t%s\t%s\t%s\n", entries[i].arr, entries[i].tga, entries[i].creator, entries[i].comment );
	}
}
