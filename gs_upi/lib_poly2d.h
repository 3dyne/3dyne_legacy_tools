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



// lib_poly2d.h

#ifndef __lib_poly2d
#define __lib_poly2d

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_poly.h"	// for #defines

#include "lib_math.h"
#include "lib_error.h"

#ifdef __cplusplus
extern "C"
{
#endif

#define	MAX_POINTS_ON_POLYGON2D ( 32 )	

typedef struct {
	int		pointnum;
	vec2d_t		p[4];		//variable
} polygon2d_t;
	
polygon2d_t*	NewPolygon2d( int pointnum );
void		FreePolygon2d( polygon2d_t *in );
polygon2d_t*	CopyPolygon2d( polygon2d_t *in );

fp_t		Polygon2dArea( polygon2d_t *in );
void		Polygon2dCenter( polygon2d_t *in, vec2d_t center );

void		SplitPolygon2d( polygon2d_t *in, vec2d_t norm, fp_t dist, polygon2d_t **front, polygon2d_t **back );

#ifdef __cplusplus
}
#endif

#endif
