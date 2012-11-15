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



// pal.h

#ifndef __PAL_H_INCLUDED
#define __PAL_H_INCLUDED
#ifdef __cplusplus
extern "C"
{
#endif
typedef struct {
	unsigned char	red;
	unsigned char   green;
	unsigned char	blue;
} rgb_t;

typedef struct {
	unsigned int	rgb_num;
	rgb_t		*rgb_set;
} pal_t;

pal_t *PAL_Create( unsigned int rgb_num );
pal_t *PAL_Read( FILE *handle );
void PAL_Free( pal_t *pal );
void PAL_Dump( pal_t *pal );

unsigned int PAL_ReduceColor( pal_t *pal, rgb_t *rgb );
#ifdef __cplusplus
}
#endif
#endif
