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



// arr.h

#ifndef __ARR_H_INCLUDED
#define __ARR_H_INCLUDED

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

// #define __arr_ib_ext

#if 1
#ifdef __arr_ib_ext
#undef __arr_ib_ext
#endif
#endif

#ifdef __arr_ib_ext
#include "ib_service.h"
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define ARR_ID ("ARRM")

#define ARR_F_P8	( 1 ) // obsolete
#define ARR_F_RGB565	( 2 ) 	

#define ARR_HEADERSIZE   ( 48 )  // hope so	

typedef struct {
	char	id[4];
	char		ident[32];
	unsigned short	mipmap_num;
	unsigned short	size_x;
	unsigned short	size_y;
	unsigned short	steps;
	unsigned short	flags;
}	arr_header_l;


typedef struct {
	unsigned int	size_x;
	unsigned int	size_y;
	unsigned int	mipmap_num;
	unsigned int	bytes;
	unsigned int	steps;
	char		ident[32];
	unsigned int	flags;
	unsigned char	*data;
} arr_t;

arr_t *ARR_Create( unsigned short size_x, unsigned short size_y, unsigned short mipmap_num, unsigned short steps,  const char* ident, unsigned int flags );
arr_t *ARR_Read( FILE* in_handle );

#ifdef __arr_ib_ext
arr_t *ARR_ReadIB( ib_file_t* );
#endif

void ARR_Write( FILE* out_handle, arr_t *arr );
void ARR_Free( arr_t *arr );
void ARR_Dump( arr_t *arr );


#ifdef __cplusplus
}
#endif
#endif
