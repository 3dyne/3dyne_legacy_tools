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



// lib_poly.h

#ifndef __lib_poly
#define __lib_poly

#include "lib_math.h"
#include "lib_hobj.h"

#ifdef __cplusplus
extern "C" 
{
#endif

#define MAX_POINTS_ON_POLYGON	( 256 )
#define BASE_POLYGON_SIZE	( 256.0*128.0 )

#define ON_EPSILON	( 0.01 ) // 0.01
#define SIDE_ON		( 0 )
#define SIDE_BACK	( 1 )
#define SIDE_FRONT	( 2 )

// for CheckPolygonWithPlane
#define POLY_ON		( 0 )
#define POLY_BACK	( 1 )
#define POLY_FRONT	( 2 )
#define POLY_SPLIT	( 3 )



typedef struct {	
	int		pointnum;
	vec3d_t		p[4];
} polygon_t;

void		DumpPolygonStat( void );

polygon_t*	NewPolygon( int points );
void		FreePolygon( polygon_t *in );
polygon_t*	CopyPolygon( polygon_t *in );

polygon_t *	CreatePolygonFromClass( hobj_t *polygon );
hobj_t *	CreateClassFromPolygon( polygon_t *p );

fp_t	PolygonArea( polygon_t *in );
void	PolygonCenter( polygon_t *in, vec3d_t center );
polygon_t*	PolygonFlip( polygon_t *in );

void	PlaneFromPolygon( polygon_t *in, vec3d_t norm, fp_t *dist );

polygon_t*	BasePolygonForPlane( vec3d_t norm, fp_t dist );

int	CheckPolygonWithPlane( polygon_t *in, vec3d_t norm, fp_t dist );
polygon_t*	ClipPolygon( polygon_t *in, vec3d_t norm , fp_t dist );
void	SplitPolygon( polygon_t *in, vec3d_t norm, fp_t dist, polygon_t **front, polygon_t **back );

void	ClipPolygonInPlace( polygon_t **inout, vec3d_t norm, fp_t dist ); 

void SetEpsilon( fp_t front, fp_t back );

#ifdef __cplusplus
}
#endif

#endif
