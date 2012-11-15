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



// type_simple.h

#ifndef __type_simple
#define __type_simple

#include <stdio.h>
#include <math.h>
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_token.h"

void Write_fp( FILE *h, fp_t b );
void Write_Vec2d( FILE *h, vec2d_t v );
void Write_Vec3d( FILE *h, vec3d_t v );
void Write_Polygon( FILE *, polygon_t *p );

void Read_Vec2d( tokenstream_t *ts, vec2d_t v );
void Read_Vec3d( tokenstream_t *ts, vec3d_t v );
polygon_t *Read_Polygon( tokenstream_t *ts );

#endif
