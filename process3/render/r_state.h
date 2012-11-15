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



// r_state.h

#ifndef __r_state
#define __r_state

//#include "render.h"
#include "r_defs.h"
#include "lib_math.h"

//
// render3.c
//


extern int		r_vertexnum;	// render3.c
extern vertex_t		r_vertices[];	// render3.c
extern vertexref_t	r_vertexrefs[];		// render3.c
extern fixpolygon_t	r_fixpolygons[];	// render3.c
extern fixpolygonref_t	r_fixpolygonrefs[];	// render3.c

// lightpage
extern int		r_lightpagenum;

// visleaf stuff
extern int	r_pointnum ;
extern int	r_portalnum;
extern int	r_visleafnum;
extern vec3d_t	r_points[];
extern portal_t r_portals[];
extern visleaf_t r_visleafs[];

// mapnode
extern int		r_mapnodenum;
extern mapnode_t	r_mapnodes[];

extern int		r_visleafnum;
extern int		r_leafbitpos[];


extern int		r_frame_count;	// render3.c
extern int		r_tri_count;

extern vec3d_t		r_origin;
extern matrix3_t	r_matrix;

//
// r_initgl.c
//

extern gl_extensions_t		gl_ext;

//
// r_csurface.c
//
extern int		r_csurfacedefnum;
extern csurfacedef_t	r_csurfacedefs[];

#endif
