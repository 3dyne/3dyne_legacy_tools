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

#ifndef __archetype
#define __archetype

#include "vec.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define AT_ID_SIZE	( 8 )
#define AT_KEY_SIZE	( 32 )
#define AT_VALUE_SIZE	( 32 )

#define AT_MAX_PAIRS	( 16 )

#define AT_ID_STRING	( "STRING" )
#define AT_ID_FLOAT	( "FLOAT" )
#define AT_ID_VEC3D	( "VEC3D" )

typedef struct kvpair_s {
	struct kvpair_s		*next;
	char		type[AT_ID_SIZE]; // "TYPE"
	char		key[AT_KEY_SIZE];
	char		value[AT_VALUE_SIZE];
} kvpair_t;
		
typedef struct archetype_s {
	int		status;
	int		visible;
	
	int		select;

	struct archetype_s	*next;
	struct kvpair_s		*pairs;

//	kvpair_t	pairs[AT_MAX_PAIRS];
} arche_t;

// 1. level
arche_t*	AT_NewArche( void );
void		AT_FreeArche( arche_t *arche );
kvpair_t*	AT_NewPair( const char *type, const char* key, const char *value );
void		AT_FreePair( kvpair_t *pair );
void		AT_Dump( arche_t *arche );

// 2. level
kvpair_t*	AT_NewPairFromPair( kvpair_t *pair );
kvpair_t*	AT_NewPairsFromList( kvpair_t *pair );
void		AT_FreePairList( kvpair_t *pair );
arche_t*	AT_NewArcheFromArche( arche_t* arche );

//void		AT_Write( FILE *h, arche_t *a );
//void		AT_Read( FILE *h, arche_t *a );

kvpair_t*	AT_GetPair( arche_t *a, const char *key );
void		AT_SetPair( arche_t *a, const char *type, const char *key, const char *value );
void		AT_AddPair( arche_t *a, kvpair_t *pair );

void		AT_RemovePair( arche_t *a, const char *key );

//
// archetype casts
//
 
void		AT_CastValueToVec3d( vec3d_t v, char *value );
void		AT_CastVec3dToValue( char *value, vec3d_t v );

//
// hobj to arche stuff
//
void		AT_InitArcheFromClass( arche_t *arche, hobj_t *obj );
hobj_t *	AT_BuildClassFromArche( arche_t *arche );

#ifdef __cplusplus
}
#endif

#endif
