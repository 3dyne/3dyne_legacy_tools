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



// r_fstexcrd.c

#include "render.h"
#include "r_facesetup.h"
#include "r_fsstate.h"

vec2d_t		fs_tmap0_texcoord_array[256*256];
vec2d_t		fs_lmap0_texcoord_array[256*256];
vec2d_t		fs_lmap1_texcoord_array[256*256];

static vec2d_t		texcrd_pvec;	// projection of vertex
static projectionType	texcrd_ptype;
static int		texcrd_vindex;

/*
  ==============================
  FS_GenTexcoord_SetVertex

  ==============================
*/
void FS_GenTexcoord_Vertex( int vindex, projectionType type )
{
	texcrd_vindex = vindex;
	texcrd_ptype = type;
      
	ProjectVec3d( texcrd_pvec, fs_vertices[fs_transformed_vref[vindex]]->v, type );
}

/*
  ==============================
  FS_GenTexcoord_tmap0

  ==============================
*/

void FS_GenTexcoord_tmap0( texdef_t *td )
{
	vec2d_t		texel;

	if ( texcrd_ptype & ProjectionType_Vecs )
	{
		vec2d_t		tmp;

		tmp[0] = texcrd_pvec[0];
		tmp[1] = texcrd_pvec[1];

		texel[0] = tmp[0]*td->vecs[0][0] + tmp[1]*td->vecs[0][1];
		texel[1] = tmp[0]*td->vecs[1][0] + tmp[1]*td->vecs[1][1];	
	}
	else
	{
		texel[0] = texcrd_pvec[0];
		texel[1] = texcrd_pvec[1];		
	}

	if ( texcrd_ptype & ProjectionType_Shift )
	{
		texel[0] += td->shift[0];
		texel[1] -= td->shift[1];
	}

	texel[0] *= r_textures[td->texture].inv_width;
	texel[1] *= -r_textures[td->texture].inv_height;

	memcpy( fs_tmap0_texcoord_array[texcrd_vindex], texel, 8 );
//	_Vec2dCopy( fs_tmap0_texcoord_array[texcrd_vindex], texel );
}



/*
  ==============================
  FS_GenTexcoord_lmap0

  ==============================
*/

void FS_GenTexcoord_lmap0( lightdef_t *ld )
{
	vec2d_t		texel;

	texel[0] = texcrd_pvec[0];
	texel[1] = texcrd_pvec[1];

	texel[0] -= ld->shift[0];
	texel[0] *= ld->scale;
	texel[0] += ld->lightmaps[0].xofs2;
	
	texel[1] -= ld->shift[1];
	texel[1] *= ld->scale;
	texel[1] += ld->lightmaps[0].yofs2;

	memcpy( fs_lmap0_texcoord_array[texcrd_vindex], texel, 8 );
//	_Vec2dCopy( fs_lmap0_texcoord_array[texcrd_vindex], texel );
}



/*
  ==============================
  FS_GenTexcoord_lmap1

  ==============================
*/
void FS_GenTexcoord_lmap1( lightdef_t *ld )
{
	vec2d_t		texel;

	texel[0] = texcrd_pvec[0];
	texel[1] = texcrd_pvec[1];

	texel[0] -= ld->shift[0];
	texel[0] *= ld->scale;
	texel[0] += ld->lightmaps[1].xofs2;
	
	texel[1] -= ld->shift[1];
	texel[1] *= ld->scale;
	texel[1] += ld->lightmaps[1].yofs2;

	memcpy( fs_lmap1_texcoord_array[texcrd_vindex], texel, 8 );	
//	_Vec2dCopy( fs_lmap1_texcoord_array[texcrd_vindex], texel );	
}
