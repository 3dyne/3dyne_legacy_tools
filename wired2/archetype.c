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



// archetype.h

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#include "archetype.h"

arche_t* AT_NewArche( void )
{
	arche_t		*a;

	a = ( arche_t * ) malloc( sizeof( arche_t ) );
	memset( a, 0, sizeof( arche_t ) );

	return a;
}

void AT_FreeArche( arche_t *a )
{
	free( a );
}

kvpair_t* AT_NewPair( const char *type, const char *key, const char *value )
{
	kvpair_t	*pair;

	pair = ( kvpair_t * ) malloc( sizeof( kvpair_t ) );
	memset( pair, 0, sizeof( kvpair_t ) );

	if ( type )
		strcpy( pair->type, type );
	if ( key )
		strcpy( pair->key, key );
	if ( value )
		strcpy( pair->value, value );

	return pair;		
}

void AT_FreePair( kvpair_t *pair )
{
	free( pair );
}


kvpair_t* AT_NewPairFromPair( kvpair_t * pair )
{
	kvpair_t	*p;
	
	p = AT_NewPair( pair->type, pair->key, pair->value );
	return p;
}

kvpair_t* AT_NewPairsFromList( kvpair_t *list )
{
	kvpair_t	*p;
	kvpair_t	*p2, *list2;

	list2 = NULL;
	for( p = list; p ; p=p->next )
	{
		p2 = AT_NewPairFromPair( p );
		p2->next = list2;
		list2 = p2;
	}
	return list2;
}

void AT_FreePairList( kvpair_t *list )
{
	kvpair_t	*p, *pnext;

	for ( p = list; p ; p=pnext )
	{
		pnext = p->next;
		AT_FreePair( p );
	}
}

arche_t* AT_NewArcheFromArche( arche_t* arche )
{
	arche_t		*anew;

	anew = AT_NewArche();
	anew->pairs = AT_NewPairsFromList( arche->pairs );

	return anew;
}

#if 0
void AT_Write( FILE *h, arche_t *a )
{
	int		i;

	fprintf( h, "{\n" );
	for ( i = 0; i < AT_MAX_PAIRS; i++ )
	{
		if ( a->key[i][0] )
			fprintf( h, "\t\"%s\" \"%s\"\n", a->key[i], a->value[i] );
	}
	fprintf( h, "}\n" );
}

void AT_Read( FILE *h, arche_t *a )
{
	int		i;

	ParseToken( h );
	if ( token[0] != '{' )
		goto at_read_error;

	// key & value loop
	for( i = 0; i < AT_MAX_PAIRS; i++ )
	{
		ParseToken( h );
		if ( token[0] == '}' )
			break;
	
		strcpy( a->key[i], token );
//		printf( "key: %s, ", token );
		ParseToken( h );
		strcpy( a->value[i], token );
//		printf( "value: %s\n", token );
		
	}
	
	return;

at_read_error:
	printf( "AT_Read: parse error.\n" );
	exit(-1);
}
#endif


kvpair_t* AT_GetPair( arche_t *a, const char *key )
{
	kvpair_t	*p;

	for( p = a->pairs; p ; p=p->next )
	{
		if ( !strcmp( key, p->key ) )
			return p;
	}

	return NULL;
}

void AT_RemovePair( arche_t *a, const char *key )
{
	kvpair_t	*p, *next, *head;
	
	head = NULL;
	for( p = a->pairs; p ; p=next )
	{
		next = p->next;

		if ( !strcmp( key, p->key ) )
		{
			AT_FreePair( p );			
			continue;
		}

		p->next = head;
		head = p;
	}

	a->pairs = head;
}

void AT_SetPair( arche_t *a, const char *type, const char *key, const char *value )
{
	kvpair_t	*p;

	p = AT_GetPair( a, key );

	if ( p )
	{
		strcpy( p->type, type );
		strcpy( p->value, value );
	}
	else
	{
		p = AT_NewPair( type, key, value );
		AT_AddPair( a, p ); 
	}
}

void AT_AddPair( arche_t *a, kvpair_t *pair )
{
	pair->next = a->pairs;
	a->pairs = pair;
}


void AT_Dump( arche_t *a )
{
	kvpair_t	*p;

	printf( "AT_Dump:\n" );
	for ( p = a->pairs; p ; p=p->next )
		printf( " (%s) %s: %s\n", p->type, p->key, p->value );
}


//
// casts ...
//

void AT_CastValueToVec3d( vec3d_t v, char *value )
{
	float		val;
	char		*ptr;

	ptr = value;
	val = (float) strtod( ptr, &ptr );
	v[0] = val;
	val = (float) strtod( ptr, &ptr );
	v[1] = val;
	val = (float) strtod( ptr, &ptr );
	v[2] = val;	
}

void AT_CastVec3dToValue( char *value, vec3d_t v )
{
	sprintf( value, "%.3f %.3f %.3f", v[0], v[1], v[2] );
}

/*
  ==================================================
  hobj <-> arche stuff

  ==================================================
*/

/*
  ==============================
  AT_InitArcheFromClass

  ==============================
*/
void AT_InitArcheFromClass( arche_t *arche, hobj_t *obj )
{
	hpair_search_iterator_t		iter;
	hpair_t			*pair;

	InitHPairSearchIterator( &iter, obj, "*" );
	for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
	{
		AT_SetPair( arche, pair->type, pair->key, pair->value );
	}

	AT_SetPair( arche, "STRING", "type", obj->type );
	AT_SetPair( arche, "STRING", "name", obj->name );
}

/*
  ==============================
  AT_BuildClassFromArche

  ==============================
*/
hobj_t * AT_BuildClassFromArche( arche_t *arche )
{
	kvpair_t	*name;
	kvpair_t	*type;
	kvpair_t		*p;

	hobj_t		*obj;

	name = AT_GetPair( arche, "name" );
	type = AT_GetPair( arche, "type" );

	if ( name && type )
	{
		obj = NewClass( type->value, name->value );
	}
	else if ( name && !type )
	{
		obj = NewClass( "none", name->value );
	}
	else if ( !name && type )
	{
		obj = NewClass( type->value, "none" );
	}
	else if ( !name && !type )
	{
		obj = NewClass( "none", "none" );
	}

	for ( p = arche->pairs; p ; p=p->next )
	{
		if ( p == name || p == type )
			continue;
		
		InsertHPair( obj, NewHPair2( p->type, p->key, p->value ) );
	}
	
	return obj;
}

#if 0

int main()
{
	arche_t		*a;
	kvpair_t	*p;

	a = AT_NewArche();
	p = AT_NewPair( AT_ID_FLOAT, "key1", "1234.5678" );
	AT_AddPair( a, p );
	p = AT_NewPair( AT_ID_VEC3D, "key2", "123 456 789" );
	AT_AddPair( a, p );
	p = AT_NewPair( AT_ID_STRING, "type", "dummy" );
	AT_AddPair( a, p );

	AT_Dump( a );

	p = AT_GetPair( a, "key2" );
	strcpy( p->value, "666.666" );

	AT_Dump( a );
}

//
//
//

#endif // __WIRED
