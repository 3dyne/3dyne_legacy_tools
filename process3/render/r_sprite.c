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



// r_sprite.c

#include "r_sprite.h"

sprite_t * R_NewSprite( char *path, fp_t width, fp_t height )
{
	int			i;
	sprite_t	*sp;
	fp_t	pts[4][2]={ {1.0, -1.0}, {-1.0,-1.0}, { -1.0, 1.0 }, { 1.0, 1.0 } };

	sp = NEW( sprite_t );
	for ( i = 0; i < 4; i++ )
	{
		sp->pts[i][0] = pts[i][0] * width;
		sp->pts[i][1] = pts[i][1] * height;
	}

	sp->texobj = Misc_GenTexture_TGA_8888( path );

	return sp;
}

void R_DrawSprite( sprite_t *sp, vec3d_t pos )
{
	vec3d_t		tmp;
	vec4d_t		v;

	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glBindTexture( GL_TEXTURE_2D, sp->texobj );
	glEnable( GL_TEXTURE_2D );
	glEnable ( GL_BLEND );                                          
	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );	

	Vec3dScale( tmp, 1.0/16.0, pos );
	CalcVertex( v, tmp );

	glColor4f( 1, 1, 1, 1 );

	glBegin( GL_TRIANGLE_FAN );
	glTexCoord2f( 0,0 );
	glVertex3f( (v[0]+sp->pts[0][0])/v[3], (v[1]+sp->pts[0][1])/v[3], 1.0/v[3] );
	glTexCoord2f( 1,0 );
	glVertex3f( (v[0]+sp->pts[1][0])/v[3], (v[1]+sp->pts[1][1])/v[3], 1.0/v[3] );
	glTexCoord2f( 1,1 );
	glVertex3f( (v[0]+sp->pts[2][0])/v[3], (v[1]+sp->pts[2][1])/v[3], 1.0/v[3] );
	glTexCoord2f( 0,1 );
	glVertex3f( (v[0]+sp->pts[3][0])/v[3], (v[1]+sp->pts[3][1])/v[3], 1.0/v[3] );
	glEnd();	
	
}
