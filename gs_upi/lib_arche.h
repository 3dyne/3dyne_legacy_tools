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



// lib_arche.h

#ifndef __lib_arche
#define __lib_arche

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_math.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_unique.h"

#define AT_ID_SIZE		( 8 )
#define AT_KEY_SIZE		( 32 )
#define AT_VALUE_SIZE		( 128 )

#define AT_MAX_PAIRS		( 16 )

typedef struct pair_s {
	struct pair_s	*next;
	char		type[AT_ID_SIZE];
	char		key[AT_KEY_SIZE];
	char		value[AT_VALUE_SIZE];
} pair_t;

typedef struct arche_s {
	unique_t		id;
	struct arche_s		*next;
	struct pair_s		*pairs;	// variable
} arche_t;

pair_t*	NewPair( void );
void FreePair( pair_t * );

arche_t* NewArche( void );
void FreeArche( arche_t * );

pair_t*	FindPair( arche_t *arche, const char *key );

arche_t* ReadArche( tokenstream_t *ts );

void CastPairToVec3d( vec3d_t v, pair_t *pair );
void CastPairToFloat( fp_t *f, pair_t *pair );


#ifdef __cplusplus
}
#endif

#endif
