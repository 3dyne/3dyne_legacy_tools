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



/// filelumps.h

#ifndef __filelumps_included
#define __filelumps_included
#include <sys/types.h>

/*
  ====================
  pal lumps
  ====================
*/

#define PAL_ID ("LPAL")

typedef struct {
	char 	id[4];
	unsigned int	rgb_num;
} 	pal_header_l;
	
typedef struct {
	unsigned char	red;
	unsigned char	green; 
	unsigned char	blue;
}	rgb_l;

/*
  ====================
  array lumps
  ====================
*/

#define ARR_ID ("ARRM")

typedef struct {
	char	id[4];
	char		arr_name[32];
	char		next_name[32];
	unsigned short	mipmap_num;
	unsigned short	size_x;
	unsigned short	size_y;
	unsigned short	flag;
}	arr_header_l;

/*
  ====================
  font lumps
  ====================
*/
#define GSF_ID ("GSFT")

typedef struct {
	char	id[4];
	unsigned int	char_num;
	unsigned int	size_x;
	unsigned int	size_y;
}	gsf_header_l;
#endif
