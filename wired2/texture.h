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



// texture.h

#ifndef __texture_included
#define __texture_included

#include "arr.h"
#include "tga.h"
#include "pal.h"

#include "lib_hobj.h"

#ifdef __cplusplus
extern "C"
{
#endif

extern pal_t		*t_pal;

typedef struct {
	char	ident[32];
	float	rotate;
	float	shift[2];
	float	scale[2];
} texturedef_t;

typedef enum
{
	texIdentType_gltexres,
	texIdentType_multilayer,

	texIdentType_num
} texIdentType;

typedef struct texident_s
{
	char		ident[64];		// primary key
	hobj_t		*obj;
	texIdentType	type;

	int		width;
	int		height;
	void		*image;
} texident_t;

texident_t * TexIdent_GetByIdent( const char *ident );

void	T_InitTexCache( void );
void	T_FreeTexCache( void );
void	T_DumpStat( void );

arr_t* T_GetTextureByName( const char *ident );
tga_t* T_GetTGA888ByName( const char *ident );

#ifdef __cplusplus
}
#endif

#endif // __texture_included
