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



// defs.h

#ifndef __defs
#define __defs

#define TEXTURE_IDENT_SIZE	( 32 )

#define		BIG_BOX		( 128.0*256.0 - 1024.0 )
/*
  ====================
  ANYplane_t

*/

// unsigned int type 
#define		PLANE_X		( 0 )
#define		PLANE_Y		( 1 )
#define		PLANE_Z		( 2 )
#define		PLANE_ANYX		( 3 )
#define		PLANE_ANYY		( 4 )
#define		PLANE_ANYZ		( 5 )
#define		PLANE_AXIS_MASK		( 7 )

#define		PLANE_POS		( 8 )

#define		PLANE_NORM_EPSILON	( 0.00001 )
#define		PLANE_DIST_EPSILON	( 0.01 )




/*
  ====================
  ANYtexdef_t

*/

// unsigned int flags
#define         PROJECT_X       ( 0 )   // ! like PLANE_X ... ! 
#define         PROJECT_Y       ( 1 )   // dto _Y
#define         PROJECT_Z       ( 2 )	// dto _Z
#define         PROJECT_VEC     ( 4 )
#define         PROJECT_SHIFT   ( 8 ) 



/*
  ====================
  ANYsurface_t

*/

// unsigned int contents

// external 
#define SURFACE_CONTENTS_OPEN           ( 1 )   // open portal
#define SURFACE_CONTENTS_CLOSE          ( 2 )   // close portal
#define SURFACE_CONTENTS_TEXTURE        ( 4 )   // textured portal
#define SURFACE_CONTENTS_WINDOW         ( 8 )   // both sides are textured
#define SURFACE_CONTENTS_SF_CLOSE       ( 16 )  // sector flood closed portal

// internal
#define SURFACE_CONTENTS_INSIDE         ( 0x10000 ) // portal between leafs with equal contents
#define SURFACE_CONTENTS_VIS_FLOOD      ( 0x20000 ) // portal was reached by vis flood
#define SURFACE_CONTENTS_VISIBLE        ( 0x20000 ) // fix me !  
// portal goes to a node and not a leaf                                         
// this happen if a sub tree could not be                                       
// portalized ... 
#define SURFACE_CONTENTS_BROKEN         ( 0x40000 ) 


// unsigned int state
// internal 
#define SURFACE_STATE_ONNODE		( 1 )
#define SURFACE_STATE_BYSPLIT		( 2 )
#define SURFACE_STATE_IGNORE		( 4 )
#define SURFACE_STATE_DONT_SPLIT	( 8 )	// bsp shouldn't split such surfaces
/*
  ====================
  ANYbrush_t

*/

#define	MAX_SURFACES_PER_BRUSH	( 128 )

// unsigned int contents

// external
#define BRUSH_CONTENTS_SOLID    ( 16 )
#define BRUSH_CONTENTS_LIQUID   ( 8 )
#define BRUSH_CONTENTS_DECO     ( 4 )
//#define BRUSH_CONTENTS_HINT     ( 4 )

#define BRUSH_CONTENTS_HULL		( 2 )

// internal
#define BRUSH_CONTENTS_EMPTY    ( 0 )   // for empty leafs ( got no brush at all ) 


/*
  ====================
  ANYnode_t

*/

// int plane

#define         NODE_PLANE_LEAF_EMPTY           ( -1 )                          
#define         NODE_PLANE_LEAF_BRUSH           ( -2 )                          
#define         NODE_PLANE_LEAF_OUTSIDE         ( -3 )                          

#endif
