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



// w2p2.h

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

#include "defs.h"
#include "../csg/cbspbrush.h"
//#include "type_simple.h"
//#include "type_plane.h"
//#include "type_wtexture.h"
//#include "type_bspbrush.h"

typedef struct aplane_s
{
	vec3d_t		norm;
	fp_t		dist;
	struct aplane_s	*next;
} aplane_t;

typedef struct wplane_s
{
	unsigned int		clsname;
	unsigned int		clsref_flipplane;
	vec3d_t		norm;
	fp_t		dist;
	int		type;

	struct aplane_s	*planes;	// for calc averrage
} wplane_t;

typedef struct wtexture_s
{
	unsigned int	clsname;
	char		ident[TEXTURE_IDENT_SIZE];
} wtexture_t;


typedef struct wtexdef_s {
	unsigned int		clsname;
	unsigned int		clsref_texture;
	unsigned int		flags;
	vec2d_t			vec[2];
	vec2d_t			shift;

	vec2d_t			scale;
	fp_t			rotate;
} wtexdef_t;



typedef struct wface_s {
	unsigned int		contents;
	int			plane;
	int			texdef;
} wsurface_t;

typedef struct wbrush_s {
	unique_t		id;
	unsigned int		contents;
	int			firstsurface;
	int			surfacenum;
} wbrush_t;

#define		MAX_PLANES	( 8192*4 )
#define		MAX_WTEXTURES	( 1024 )
#define		MAX_WTEXDEFS	( 4096 )
#define		MAX_WSURFACES	( 16000 )
#define		MAX_WBRUSHES	( 4000 )

