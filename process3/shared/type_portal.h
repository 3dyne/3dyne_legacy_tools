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



// type_portal.h

#ifndef __type_portal
#define __type_portal

#include <stdio.h>
#include <math.h>
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_token.h"
#include "lib_error.h"
#include "type_simple.h"
#include "defs.h"

typedef struct portal_s {
	vec3d_t		norm;
	fp_t		dist;

	polygon_t	*p;

	int		nodes[2];       
} portal_t;

typedef struct int_array_s {
	int		intnum;
	int		ints[10];	// variable
} int_array_t;

int_array_t* NewIntArray( int num );
void FreeIntArray( int_array_t *np );

void Read_Portal( tokenstream_t *ts, portal_t *p );
void Read_PortalArray( portal_t *base, int *maxnum, char *name );

int_array_t* Read_IntArray( tokenstream_t *ts );
void Read_IAArray( int_array_t **base, int *maxnum, char *name );

#endif
