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



// visclip.h

#ifndef __visclip
#define __visclip

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#include "gl_client.h"

// visclip.c
typedef struct plane_s
{
	vec3d_t		norm;
	fp_t		dist;
} plane_t;

typedef struct visclip_s
{
	int		planenum;
	plane_t		planes[4];	// variable
} visclip_t;

visclip_t * SetupNewVisclip_2( polygon_t *from, polygon_t *through, bool_t flipclip );
visclip_t * SetupNewVisclip( polygon_t *from, polygon_t *through, bool_t flip );
visclip_t * SetupNewVisclip_old( polygon_t *from, polygon_t *through, bool_t flip_from );
void FreeVisclip( visclip_t *vis );
polygon_t * PolygonVisclip( visclip_t *vc, polygon_t *to );
void PolygonVisclipInPlace( visclip_t *vc, polygon_t **inout );
#endif
