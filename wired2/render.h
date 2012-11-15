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



// render.h

#ifndef __render_included
#define __render_included

#ifdef __cplusplus
extern "C"
{
#endif

//typedef	float	vec5d_t[5];

#include "vec.h"
#include "brush.h"

void	R_CalcMatrix( float, float, float );
void	R_SetOrigin( vec3d_t );
void	R_SetZoom( float );
void	R_SetView( float width, float height );
void	R_CalcFrustum( void );

void	R_Vec3dRot( vec3d_t, vec3d_t );
void	R_Vec3dInverseRot( vec3d_t, vec3d_t );
void	R_Vec3dPer( vec3d_t, vec3d_t );	

int	R_FrustumClipLine( vec3d_t from, vec3d_t to, int planenum );

void	R_DumpStat( void );
void	R_SetFrameBuffer( void *ptr, int depth );
void	R_InitFrame( void );

void	R_RenderTextureFrame( brush_t *brushes );
void	R_RenderDebugCSGFaces( const char *file );
void	R_RenderTextureFace( face_t *face );
void	R_RasterizeFace( void );

void	R_NO_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		       int xright, float zright, float uright, float vright );

void	R_LightFace( face_t *f );
void	R_RasterizeLight( void );

void	R_NO_LightSpan( int y, int xleft, float zleft, float uleft, float vleft,
		       int xright, float zright, float uright, float vright );


// new
void	R_SetTexture( const char *ident );
void	R_RenderPolygon( polygon_t *poly );

#ifdef __cplusplus
}
#endif

#endif // __render_included
