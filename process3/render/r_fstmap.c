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



// r_fstmap.c

/*
  =============================================================================
  FaceSetup :
  -----------

  o texture mapping setup
  o single pass texturing

  =============================================================================
*/

#include "render.h"
#include "r_facesetup.h"
#include "r_fsstate.h"

// facebfr refs sorted by textures
int		 fs_tmap0num[MAX_TEXTURES];
int		*fs_tmap0facebfr[MAX_TEXTURES][2048];

// facebfr refs sorted by static diffuse lightpages
int		 fs_lmap0num[MAX_LIGHTPAGES];
int		*fs_lmap0facebfr[MAX_LIGHTPAGES][1024];

// facebfr refs sorted by static specular lightpages
int		 fs_lmap1num[MAX_LIGHTPAGES];
int		*fs_lmap1facebfr[MAX_LIGHTPAGES][1024];

void R_FS_InitTMapsBfr( void )
{
	int		i;

	for ( i = 0; i < MAX_TEXTURES; i++ )
		fs_tmap0num[i] = 0;

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
		fs_lmap0num[i] = 0;	
		
	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
		fs_lmap1num[i] = 0;

}

void R_FS_FinishTMapsBfr( void )
{

}

fsTMapsFlags R_FS_GetTMapsFlags( face_t *f )
{
	fixpolygon_t	*fix;
	fsTMapsFlags	flags = FSTMapsFlags_none;

	fix = f->fixpolygon;

	if ( fix->texdef != -1 )
		flags |= FSTMapsFlags_tmap0;
	if ( r_lightdefs[fix->lightdef].lightmapnum >= 1 )
		flags |= FSTMapsFlags_lmap0;
	if ( r_lightdefs[fix->lightdef].lightmapnum >= 2 )
		flags |= FSTMapsFlags_lmap1;

	return flags;
}

void R_FS_FillTMapsBfr_tmap0( int *facebfr, int mapref )
{
	fs_tmap0facebfr[mapref][fs_tmap0num[mapref]++] = facebfr;	
}

void R_FS_FillTMapsBfr_lmap0( int *facebfr, int mapref )
{
	fs_lmap0facebfr[mapref][fs_lmap0num[mapref]++] = facebfr;	
}

void R_FS_FillTMapsBfr_lmap1( int *facebfr, int mapref )
{
	fs_lmap1facebfr[mapref][fs_lmap1num[mapref]++] = facebfr;	
}

