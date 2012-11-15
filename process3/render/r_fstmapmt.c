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



// r_fstmapmt.c

/*
  =============================================================================
  FaceSetup :
  -----------

  o texture mapping setup 
  o multi pass texturing
  =============================================================================
*/

#include "render.h"
#include "r_facesetup.h"
#include "r_fsstate.h"


int			 fs_tmap0_sortby_lmap0_num[MAX_LIGHTPAGES];
facebfr_tmap0_sort_t	 fs_tmap0_sortby_lmap0_facebfr[MAX_LIGHTPAGES][1024];
//int		*fs_tmap0_sortby_lmap0_facebfr[MAX_LIGHTPAGES][1024];
//int		 fs_tmap0_sortby_lmap0_tmap0ref[MAX_LIGHTPAGES][1024];

static int qsort_compare( const void *fb1, const void *fb2 )
{
	if ( (*(facebfr_tmap0_sort_t**)fb1)->tmap0ref > (*(facebfr_tmap0_sort_t**)fb2)->tmap0ref )
		return 1;
	else if ( (*(facebfr_tmap0_sort_t**)fb1)->tmap0ref < (*(facebfr_tmap0_sort_t**)fb2)->tmap0ref )
		return -1;
	return 0;
}

void R_FS_InitTMapsBfr_mt( void )
{
	int		i;

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
		fs_tmap0_sortby_lmap0_num[i] = 0;
}

void R_FS_FinishTMapsBfr_mt( void )
{
	int		i;

	// sort tmap0refs
	
	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		int		num;

		num = fs_tmap0_sortby_lmap0_num[i];

		if ( !num )
			continue;
		qsort( &fs_tmap0_sortby_lmap0_facebfr[i][0], num, sizeof( facebfr_tmap0_sort_t ), qsort_compare );		
	}
}

void R_FSFillTMapsBfr_mt_tmap0_lmap0( int *facebfr, int tmap0ref, int lmap0ref )
{
	fs_tmap0_sortby_lmap0_facebfr[lmap0ref][fs_tmap0_sortby_lmap0_num[lmap0ref]].facebfr = facebfr;
	fs_tmap0_sortby_lmap0_facebfr[lmap0ref][fs_tmap0_sortby_lmap0_num[lmap0ref]].tmap0ref = tmap0ref;
	fs_tmap0_sortby_lmap0_num[lmap0ref]++;
}
