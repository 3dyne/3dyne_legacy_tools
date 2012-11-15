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



// r_fsglva.c

/*
  =============================================================================
  setup opengl vertex arrays

  =============================================================================
*/

#include "render.h"
#include "r_facesetup.h"
#include "r_fsstate.h"

vec4d_t		fs_transformed_vertices[MAX_FS_VERTICES];

int		fs_vertex_array_num;
int		fs_transformed_vref[256*256];
vec4d_t		fs_vertex_array[256*256];

/*
  ====================
  R_FS_SetupVA

  ====================
*/

void R_FS_SetupVA( void )
{
	int		i;

 	for ( i = 0; i < fs_vertexnum; i++ )
	{
		vec3d_t		v;

		Vec3dScale( v, 1.0/16.0, fs_vertices[i]->v );
		CalcVertex( fs_transformed_vertices[i], v );
	}

	for ( i = 0; i < fs_vertex_array_num; i++ )
	{
		Vec4dCopy( fs_vertex_array[i], fs_transformed_vertices[fs_transformed_vref[i]] );
	}
	
}
