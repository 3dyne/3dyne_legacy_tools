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



#include <string.h>
#include "cdb_service.h"
#include "lib_error.h"
#include "lib_arche.h"
#include "lib_token.h"

static arche_t		*spawn;
static char		cdb_name[256];

void CDB_StartUp( int flag )
{
	char*	cdb_path;
	tokenstream_t	*ts;


	cdb_path = getenv( "HOME" );
	
	sprintf( cdb_name, "%s/%s", cdb_path, CDB_FILE );

	if( flag )
	{
		printf( "\tcdb: %s\n", cdb_name );
	}

	ts = BeginTokenStream( cdb_name );
	
	spawn = ReadArche( ts );

	EndTokenStream( ts );

	return;
}

void CDB_Save()
{
	FILE		*h;
	pair_t		*p;
	

	h = fopen( cdb_name, "wb" );
	fprintf( h, "{" );

	for( p = spawn->pairs; p; p = p->next )
	{
		fprintf( h, "\t{ \"%s\" \"%s\" \"%s\" }\n", p->type, p->key, p->value );
	}
	fprintf( h, "}\n" );
	fclose( h );
}

void CDB_ChangeStringEntry( const char* arg_key, char* arg_string )
{
	int	i;
	pair_t	*p;

	p = FindPair( spawn, arg_key );
	if( p )
	{
		strncpy( p->value, arg_string, AT_VALUE_SIZE-2 );
		return;
	}

	printf( "adding new key.\n" );

	p = NewPair();
	
	strncpy( p->type, "STRING", AT_ID_SIZE-2 );
	strncpy( p->key, arg_key, AT_KEY_SIZE-2 );
	strncpy( p->value, arg_string, AT_VALUE_SIZE-2 );
	
	p->next = spawn->pairs;
	spawn->pairs = p;
}

char* CDB_GetString( const char* arg_key )
{
	pair_t	*p;
	char	*data;
       	
	data = ( char* ) malloc( CDB_STRING_SIZE );
	memset( data, 0, CDB_STRING_SIZE );

	p = FindPair( spawn, arg_key );

	if( !p )
	{
		free( data );
		return NULL;
	}

	strncpy( data, p->value, CDB_STRING_SIZE-2 );

	return data;
}

void CDB_ChangeIntEntry( const char* arg_key, int arg_value )
{
	pair_t	*p;
	char	tmp[256];

	p = FindPair( spawn, arg_key );
	
	sprintf( tmp, "%d", arg_value );

	if( p )
	{
		strncpy( p->value, tmp, AT_VALUE_SIZE-2 );
		return;
	}

	printf( "adding new key.\n" );

	p = NewPair();
	strncpy( p->type, "INT", AT_ID_SIZE-2 );
	strncpy( p->key, arg_key, AT_KEY_SIZE-2 );
	strncpy( p->value, tmp, AT_VALUE_SIZE-2 );
	
	p->next = spawn->pairs;
	spawn->pairs = p;

}


int CDB_GetIntValue( const char* arg_key )
{
	int	i;
	pair_t	*p;

	p = FindPair( spawn, arg_key );

	if( !p )
	{
		return 0;
	}

	i = atoi( p->value );

	return i;
}

void CDB_RemoveEntry( const char* arg_key )
{
	pair_t		*p, *pnext, *p2, *head;
	

	p = FindPair( spawn, arg_key );
	
	if( !p )
		return;

	head = NULL;

	for( p2 = spawn->pairs; p2; p2 = pnext )
	{
		pnext = p2->next;
		if( p2 == p )
			continue;

		p2->next = head;
		head = p2;
	}
	FreePair( p );

	spawn->pairs = head;
}

void CDB_RemoveStringEntry( const char* arg_key )
{
	CDB_RemoveEntry( arg_key );
}

void CDB_RemoveIntEntry( const char* arg_key )
{
	CDB_RemoveEntry( arg_key );
}
