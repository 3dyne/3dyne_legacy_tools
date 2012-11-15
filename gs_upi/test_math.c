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



// test_math.c

#include <stdio.h>

#include "lib_math.h"
#include "lib_poly2d.h"

int main()
{
	vec2d_t		p1 = { 0, 0 };
	vec2d_t		p2 = { 0, 10 };
	vec2d_t		p3 = { 10,10 };
	vec2d_t		p4 = { 10, 0 };

	vec2d_t		p5 = { 10, 0 };
	vec2d_t		p6 = { 10, 10 };

	vec2d_t		norm;
	fp_t		dist;

	vec3d_t		norm3;
	vec3d_t		v1;
	vec3d_t		v2;
	      
	polygon2d_t	*poly1;
	polygon2d_t	*front, *back;

	poly1 = NewPolygon2d( 4 );
	poly1->pointnum = 4;
	Vec2dCopy( poly1->p[0], p1 ); 
	Vec2dCopy( poly1->p[1], p2 ); 
	Vec2dCopy( poly1->p[2], p3 ); 
	Vec2dCopy( poly1->p[3], p4 ); 

	Vec2dInitStraight( norm, &dist, p5, p6 );
	SplitPolygon2d( poly1, norm, dist, &front, &back );

	printf( "front %p, back %p\n", front, back );

	Vec2dPrint( norm );
	printf( "%f\n", dist );

	Vec3dInit( norm3, 0.707, 0.0, 0.707 );
	dist = 0.0;
	Vec3dInit( v1, 6, 0, 2 );
	Vec3dProjectOnPlane( v2, norm3, dist, v1 );
	Vec3dPrint( v2 );
	
}
