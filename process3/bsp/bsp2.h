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



// bsp2.h

#ifndef __bsp
#define __bsp

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

#include "defs.h"
//#include "type_simple.h"
//#include "type_plane.h"
//#include "type_bspbrush.h"
//#include "type_bspnode.h"

#include "../csg/cbspbrush.h"

typedef enum 
{
	NodeType_node = 0,
	NodeType_emptyleaf,
	NodeType_solidleaf,
	NodeType_contentsleaf
} nodeType;

typedef struct cbspnode_s {
	
	nodeType		type;	       

	// node+leaf
	struct cbspbrush_s	*volume;

	// node stuff

	cplane_t		*plane;
	int			contents;	// contents of brush, the plane was from
	vec3d_t		min, max;

	struct cbspnode_s	*child[2];

	// leaf stuff
	
	struct cbspbrush_s	*solid;	// list of brush, that build this node

} cbspnode_t;

#define		MAX_PLANES	( 4096 )
#define		MAX_NODES	( 32000 )
#define		MAX_BRUSHES	( 16000 )

#define BRUSH_BACK_ON   ( 0 )
#define BRUSH_FRONT_ON  ( 1 )
#define BRUSH_BACK      ( 2 )
#define BRUSH_FRONT     ( 3 )
#define BRUSH_SPLIT     ( 4 )


#endif
