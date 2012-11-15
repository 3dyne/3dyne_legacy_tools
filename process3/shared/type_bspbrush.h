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



// bspbrush.h

#ifndef __bspbrush
#define __bspbrush

#include <stdio.h>
#include <math.h>
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_token.h"
#include "lib_error.h"
#include "type_plane.h"
#include "defs.h"

typedef struct surface_s {
	// external 
	int			plane;
	unsigned int		contents;
	unsigned int		state;

	// internal
	polygon_t		*p;
} surface_t;

typedef struct bspbrush_s {
	// external
	unsigned int		contents;
	int			surfacenum;
	int			original;

	// internal
	vec3d_t			min, max;
	struct bspbrush_s	*next;

	struct surface_s        surfaces[6];	// variable
} bspbrush_t;


extern plane_t	p_planes[];	// ! must be defined by bspbrush user !

bspbrush_t*	NewBrush( int surfacenum );
void		FreeBrush( bspbrush_t *bb );
void		FreeBrushList( bspbrush_t *list );
bspbrush_t*	CopyBrush( bspbrush_t *in );
int		BrushListLength( bspbrush_t *list );

void		CreateBrushPolygons( bspbrush_t *b );
void		CalcBrushBounds( bspbrush_t *bb );
void		CalcBrushListBounds( bspbrush_t *head, vec3d_t min, vec3d_t max );
fp_t		CalcBrushVolume( bspbrush_t *in );

bspbrush_t*	Read_Brush( tokenstream_t *ts );
bspbrush_t*	Read_BrushList( char *name );
void		Read_BrushArray( bspbrush_t **base, int *maxnum, char *name );

void		Write_Brush( FILE *h, bspbrush_t *b );
void		Write_BrushList( bspbrush_t *list, char *name, char *creator );
void		Write_BrushArray( bspbrush_t **base, int num, char *name, char *creator );

#endif
