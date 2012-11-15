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



// 3dnow_vec.h

#ifndef __3dnow_vec
#define __3dnow_vec

//typedef float	vec3d_t[3];

void vec3d_add_3dnow( vec3d_t out, vec3d_t in1, vec3d_t in2 );
void vec3d_sub_3dnow( vec3d_t out, vec3d_t in1, vec3d_t in2 );
void vec3d_scale_3dnow( vec3d_t out, float scale, vec3d_t in );

void vec3d_len_3dnow( float *len, vec3d_t in );
void vec3d_unify_3dnow( vec3d_t out, vec3d_t in );

void vec3d_dotproduct_3dnow( float *dot, vec3d_t in1, vec3d_t in2 );

#endif
