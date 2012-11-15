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



// r_fsfx.c

#include "r_facesetup.h"
#include "r_sprite.h"

/*
  ==============================
  FX_SetupProjectionVecs

  ==============================
*/
void FX_SetupProjectionVecs( vec3d_t norm, vec3d_t vright, vec3d_t vup )
{
	int		i;
	vec3d_t		an;
	fp_t		d;

	Vec3dInit( vup, 0, 0, 0 );

	for ( i = 0; i < 3; i++ )
		an[i] = fabs( norm[i] );

	if ( an[0] >= an[1] && an[0] >= an[2] )
		vup[1] = 1.0;
	else if ( an[1] >= an[0] && an[1] >= an[2] )
		vup[2] = 1.0;
	else if ( an[2] >= an[0] && an[2] >= an[1] )
		vup[1] = 1.0;

	d = Vec3dDotProduct( vup, norm );
	Vec3dMA( vup, -d, norm, vup );
	Vec3dUnify( vup );

	Vec3dCrossProduct( vright, norm, vup );		
}


static vec3d_t		dlpos = { -64, 128, -192 };
static vec3d_t		pvup, pvright;
static vec3d_t		porigin;

void R_FS_SetupFX( face_t *f )
{
	fixpolygon_t	*fix;
	static sprite_t	*spr = NULL;

	if ( !spr )
	{
		spr = R_NewSprite( ART_PATH "/fx/cross_red.tga", 1.0, 1.0 );
	}

	fix = f->fixpolygon;

	Vec3dProjectOnPlane( porigin, fix->pl->norm, fix->pl->dist, dlpos );

	FX_SetupProjectionVecs( fix->pl->norm, pvright, pvup );

	//
	{
		R_DrawSprite( spr, porigin );
	}
}
