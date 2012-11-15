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



// r_fsglrastermt.c

#include "r_facesetup.h"
#include "r_fsstate.h"

void R_FS_RasterizeTMapsBfr_mt_tmap0_lmap0( void )
{
#ifdef GL_ARB_MULTITEXTURE_EXT
	int		elmt;
	int		i, j, k, num;

//	vec4d_t		color_white = { 1.0, 1.0, 1.0, 1.0 };

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 4, GL_FLOAT, 0, fs_vertex_array );	

	glClientActiveTextureARB(GL_TEXTURE0_ARB);
 	glTexCoordPointer( 2, GL_FLOAT, 0, fs_tmap0_texcoord_array );
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );

	glClientActiveTextureARB(GL_TEXTURE1_ARB);
	glTexCoordPointer( 2, GL_FLOAT, 0, fs_lmap0_texcoord_array );
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);

//	glLockArraysEXT( 0, f_vertex_array_num );

	glEnable( GL_TEXTURE_2D );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );
	glDisable(GL_BLEND);
//	glBlendEquationEXT( GL_FUNC_ADD_EXT );

//	glColor4f( 1.0, 1.0, 1.0, 1.0 );

//	elmt = 0;

	for( i = 0; i < MAX_LIGHTPAGES; i++ )		
	{
		int		*faceptr;
		int		pointnum;
		int		maps;

		int		last_tmap0ref = -1;

		num = fs_tmap0_sortby_lmap0_num[i];
		if ( !num )
			continue;
		// set lightmap texobj	
		glActiveTextureARB(GL_TEXTURE1_ARB);
		glBindTexture( GL_TEXTURE_2D, r_lightpages[i].texobj );
		glEnable(GL_TEXTURE_2D);	
//		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_BLEND );
		glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE );


		for ( j = 0; j < num; j++ )
		{
		
			// set texturemap texobj, if changed
			if ( last_tmap0ref != fs_tmap0_sortby_lmap0_facebfr[i][j].tmap0ref )
			{
				last_tmap0ref = fs_tmap0_sortby_lmap0_facebfr[i][j].tmap0ref;

				glActiveTextureARB(GL_TEXTURE0_ARB);
				glBindTexture( GL_TEXTURE_2D, r_textures[last_tmap0ref].texobj );
				glEnable(GL_TEXTURE_2D);
				glTexEnvi( GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE );				
			}

			faceptr = fs_tmap0_sortby_lmap0_facebfr[i][j].facebfr;
			pointnum = *faceptr++;
			maps = *faceptr++;
			
			elmt = *faceptr++;
			
			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < pointnum; k++, elmt++ )
			{
				glArrayElement( elmt );
//				glMultiTexCoord2fvARB(GL_TEXTURE0_ARB, fs_tmap0_texcoord_array[elmt] );
//				glMultiTexCoord2fvARB(GL_TEXTURE1_ARB, fs_lmap0_texcoord_array[elmt] );
//				glVertex4fv( fs_vertex_array[elmt] );
			}
			glEnd();
		}

		r_tri_count+=num-2;
	}

//	glUnlockArraysEXT();
#else
	Error( "GL_ARB_MULTITEXTURE_EXT disabled !\n" );
#endif
}
