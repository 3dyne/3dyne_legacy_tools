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



// lib_arche.c
#include <string.h>

#include "lib_arche.h"



/*
  ====================
  NewPair

  ====================
*/
pair_t* NewPair( void )
{
	pair_t		*pair;

	pair = (pair_t *) malloc( sizeof( pair_t ) );
	memset( pair, 0, sizeof( pair_t ) );

	return pair;

}



/*
  ====================
  FreePair

  ====================
*/
void FreePair( pair_t *pair )
{
	free( pair );
}



/*
  ====================
  NewArche

  ====================
*/
arche_t* NewArche( void )
{
	arche_t		*arche;

	arche = (arche_t *) malloc( sizeof( arche_t ) );
	memset( arche, 0, sizeof( arche_t ) );

	return arche;
}



/*
  ====================
  FreeArche

  ====================
*/
void FreeArche( arche_t *arche )
{
	free( arche );
}


/*
  ====================
  FindPair

  ====================
*/
pair_t* FindPair( arche_t* arche, const char* key )
{
	pair_t		*pair;

	for ( pair = arche->pairs; pair; pair=pair->next )
		if ( !strcmp( key, pair->key ) )
			return pair;

	return NULL;
}


/*
  ====================
  CastPairToVec3d

  ====================
*/
void CastPairToVec3d( vec3d_t v, pair_t *pair )
{
	char		*ptr;
	ptr = pair->value;

	v[0] = (float) strtod( ptr, &ptr );
	v[1] = (float) strtod( ptr, &ptr );
	v[2] = (float) strtod( ptr, &ptr );
}

/*
  ====================
  CastPairToFloat

  ====================
*/
void CastPairToFloat( float *f, pair_t *pair )
{
	*f = atof( pair->value );
}



/*
  ====================
  ReadArche

  ====================
*/
arche_t* ReadArche( tokenstream_t *ts )
{
	arche_t		*arche;
	pair_t		*pair;

	arche = NewArche();

	GetToken( ts );
	if ( ts->token[0] != '{' )
		Error( "ReadArche: expect '{'\n" );

	GetToken( ts );
	if ( ts->token[0] == '*' )
	{
		arche->id = atoi( &ts->token[1] );
	}
	else
	{
		arche->id = UNIQUE_INVALIDE;
		KeepToken( ts );
	}

	for (;;)
	{
		GetToken( ts );
		if ( ts->token[0] == '}' )
			break;

		pair = NewPair();

		if ( ts->token[0] != '{' )
			Error( "ReadArche: expect '{'\n" );

		// get type
		GetToken( ts );
		strcpy( pair->type, ts->token );

		// get key
		GetToken( ts );
		strcpy( pair->key, ts->token );

		// get value
		GetToken( ts );
		strcpy( pair->value, ts->token );

		// expect '}'
		GetToken( ts );
		if ( ts->token[0] != '}' )
			Error( "eadArche: expect '}'\n" );

		// link list
		pair->next = arche->pairs;
		arche->pairs = pair;
	}

	return arche;
}
