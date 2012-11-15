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



// worldpatch.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include <lib_math.h>
#include <lib_container.h>
#include <lib_poly.h>

#include "light.h"

void PatchListSetupWorldPatches( patch_t *p_head, cplane_t *pl, fp_t patchsize )
{
	patch_t		*p;	
	world_scrap_t	*scrap;

	for ( p = p_head; p ; p=p->next )
	{
		for ( scrap = p->scrap_head; scrap ; scrap=scrap->next )
		{
			Balance_AddWorldPatchScrap( scrap->area, pl, scrap->x, scrap->y, patchsize, p->intens_diffuse, p->intens_specular );	
		}
	}
}

void SetupWorldPatches( face_t *list )
{
	face_t		*f;

	printf( "setup world patches ...\n" );
	
	for ( f = list; f ; f=f->next )
	{
		PatchListSetupWorldPatches( f->patches, f->pl, f->patchsize );
	}
}

void FaceListSetupWorldPatches( u_list_t *face_list )
{
	u_list_iter_t		iter;
	face_t			*f;

	U_ListIterInit( &iter, face_list );
	for ( ; ( f = U_ListIterNext( &iter ) ) ; )
	{
		PatchListSetupWorldPatches( f->patches, f->pl, f->patchsize );
	}
}
