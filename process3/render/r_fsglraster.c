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



// r_fsglraster.c

#include "r_facesetup.h"
#include "r_fsstate.h"


/*
  ==============================
  R_FS_RasterizeTMap0Bfr

  texture map

  ==============================
*/
void R_FS_RasterizeTMapsBfr_tmap0( void )
{
	int		i, j, num;

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 4, GL_FLOAT, 0, fs_vertex_array );	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, fs_tmap0_texcoord_array );

	glEnable( GL_TEXTURE_2D );
      	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );
	glDisable( GL_BLEND );

	for ( i = 0; i < MAX_TEXTURES; i++ )
	{
		num = fs_tmap0num[i];
		if ( !num )
			continue;

		glBindTexture( GL_TEXTURE_2D, r_textures[i].texobj );
		
		for( j = 0; j < num; j++ )
		{
			int		*faceptr;
			int		pointnum;
			int		k, elmt;

			faceptr =  fs_tmap0facebfr[i][j];
			if ( !faceptr )
				break;

			pointnum = *faceptr++;
			
			// maps flag
			faceptr++;
			
			elmt = *faceptr++;

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < pointnum; k++, elmt++ )
			{		       
				glTexCoord2fv( fs_tmap0_texcoord_array[elmt] );
				glVertex4fv( fs_vertex_array[elmt] );
			}
			glEnd();
		}
	}
}


/*
  ==============================
  R_FS_RasterizeLMap0Bfr

  static diffuse light map

  ==============================
*/
void R_FS_RasterizeTMapsBfr_lmap0( void )
{
	int		i, j, num;

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 4, GL_FLOAT, 0, fs_vertex_array );	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, fs_lmap0_texcoord_array );

	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		num = fs_lmap0num[i];
		if ( !num )
			continue;
		
		glBindTexture( GL_TEXTURE_2D, r_lightpages[i].texobj );
		
		for( j = 0; j < num; j++ )
		{
			int		*faceptr;
			int		pointnum;
			int		k, elmt;

			faceptr =  fs_lmap0facebfr[i][j];
			if ( !faceptr )
				break;

			pointnum = *faceptr++;
			
			// maps flag
			faceptr++;
			
			elmt = *faceptr++;

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < pointnum; k++, elmt++ )
			{		       
				glTexCoord2fv( fs_lmap0_texcoord_array[elmt] );
				glVertex4fv( fs_vertex_array[elmt] );
			}
			glEnd();
		}
	}	
}

/*
  ==============================
  R_FS_RasterizeLMap1Bfr

  static specular light map

  ==============================
*/
void R_FS_RasterizeTMapsBfr_lmap1( void )
{
	int		i, j, num;

	glEnableClientState( GL_VERTEX_ARRAY );
	glVertexPointer( 4, GL_FLOAT, 0, fs_vertex_array );	
	glEnableClientState( GL_TEXTURE_COORD_ARRAY );
	glTexCoordPointer( 2, GL_FLOAT, 0, fs_lmap1_texcoord_array );

	glEnable( GL_TEXTURE_2D );
	glEnable(GL_BLEND);
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBlendFunc( GL_ONE, GL_ONE );


	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		num = fs_lmap1num[i];
		if ( !num )
			continue;
		
		glBindTexture( GL_TEXTURE_2D, r_lightpages[i].texobj );
		
		for( j = 0; j < num; j++ )
		{
			int		*faceptr;
			int		pointnum;
			int		k, elmt;

			faceptr =  fs_lmap1facebfr[i][j];
			if ( !faceptr )
				break;

			pointnum = *faceptr++;
			
			// maps flag
			faceptr++;
			
			elmt = *faceptr++;

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < pointnum; k++, elmt++ )
			{		       
				glTexCoord2fv( fs_lmap1_texcoord_array[elmt] );
				glVertex4fv( fs_vertex_array[elmt] );
			}
			glEnd();
		}
	}
}

