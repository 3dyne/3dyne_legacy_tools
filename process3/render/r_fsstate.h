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



// r_fsstate.h

#ifndef __r_fsstate
#define __r_fsstate

#include "r_facesetup.h"

//
// r_facesetup.h
//


extern int		 fs_vertexnum;	
extern vertex_t		*fs_vertices[];	

extern int		*fs_facebfrptr;	
extern int		 fs_facebfr[];	


//
// r_fsglva.c
//

extern int		fs_transformed_vref[];
extern int		fs_vertex_array_num;
extern vec4d_t		fs_vertex_array[];

//
// r_fstexcrd.c
//

extern vec2d_t		fs_tmap0_texcoord_array[];
extern vec2d_t		fs_lmap0_texcoord_array[];
extern vec2d_t		fs_lmap1_texcoord_array[];


//
// r_fstmap.c
//

extern int		 fs_tmap0num[];
extern int		*fs_tmap0facebfr[MAX_TEXTURES][2048];
extern int		 fs_lmap0num[];
extern int		*fs_lmap0facebfr[MAX_LIGHTPAGES][1024];
extern int		 fs_lmap1num[];
extern int		*fs_lmap1facebfr[MAX_LIGHTPAGES][1024];

//
// r_fstmapmt.c
//

extern int				 fs_tmap0_sortby_lmap0_num[MAX_LIGHTPAGES];
extern facebfr_tmap0_sort_t		 fs_tmap0_sortby_lmap0_facebfr[MAX_LIGHTPAGES][1024];
//extern int				 fs_tmap0_sortby_lmap0_tmap0ref[MAX_LIGHTPAGES][1024];


#endif
