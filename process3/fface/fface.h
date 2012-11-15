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



// fface.h

#ifndef __fface
#define __fface

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "lib_math.h"
#include "lib_poly.h"
#include "lib_token.h"
#include "lib_error.h"
#include "cmdpars.h"


typedef struct face_s
{
	polygon_t	*p;
	vec3d_t		min, max;
	vec3d_t			norm;
	fp_t			dist;

	polygon_t	*fix;	
	polygon_t		*fix2;	// polygon2 if split is needed
	int		type;
	// = -1      : polygon needs no fix
	// = 1024    : draw with center
	// 0 .. 1023 : draw with this startvertex

	int		startfacevertex;
	int		facevertexnum;

	int		startfacevertex2;
	int		facevertexnum2;
} face_t;

#define MAX_FACES		( 65000 )
#define MAX_VERTICES		( 16000 )
#define MAX_FACEVERTICES	( 65000 )

#endif __fface
