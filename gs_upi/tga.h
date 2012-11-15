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



// tga.h

#ifndef __TGA_H_INCLUDED
#define __TGA_H_INCLUDED

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdio.h>

#include "pal.h"

// fix me:
#define NO_TGA_OBJ

// defines ...

#define		TGA_TYPE_TRUECOLOR	( 2 )
#define		TGA_TYPE_INDEXED	( 1 )

// typedefs ...

typedef struct {
	unsigned int	pixels;
	unsigned int	bytes;
	unsigned char	*red;
	unsigned char	*green;
	unsigned char	*blue;
	unsigned char	*alpha;
} tga_rgb_buf_t;

typedef struct {
	unsigned int	pixels;
	unsigned int	bytes;
	unsigned char	*data;
	pal_t		*pal;
} tga_indexed_buf_t;

typedef struct {
	unsigned char	ident_len;
	unsigned char	cmap_type;
	unsigned char	image_type;
	unsigned short	cmap_origin;
	unsigned short	cmap_len;
	unsigned char	centry_size;
	unsigned short	image_xorg;
	unsigned short	image_yorg;
	unsigned short	image_width;
	unsigned short	image_height;
	unsigned char	pixel_size;
	unsigned char	image_discr;

	// the image itself

	tga_rgb_buf_t		image;
	tga_indexed_buf_t	image_indexed;

} tga_t;


//typedef unsigned char* tga_arr_buf_t;

#ifdef NO_TGA_OBJ
//#include "tga.c"
#endif

//#ifndef NO_TGA_OBJ

// global functions ...

#ifndef __TGA_C
tga_t *TGA_Create( unsigned short width, unsigned short height, unsigned char image_type );
tga_t *TGA_Read( FILE *tga_handle );
void TGA_Write( FILE *tga_handle, tga_t *tga );
void TGA_Dump( tga_t *tga );
void TGA_Free( tga_t *tga );
#endif

#ifdef __cplusplus
}
#endif

#endif
