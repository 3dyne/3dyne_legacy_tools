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



// g_mapdefs.h

#ifndef __g_mapdefs
#define __g_mapdefs

#include "interfaces.h"
//#include "lib_hobj.h"
#include "g_bmdefs.h"
#include "g_ccmap3.h"
#include "defs.h"

typedef struct g_map_s 
{
	hmanager_t		*planehm;
	hmanager_t		*vertexhm;
	hmanager_t		*brushhm;
	hmanager_t		*mapnodehm;
	hmanager_t		*lightdefhm;
	hmanager_t		*texdefhm;
	hmanager_t		*texturehm;
	hmanager_t		*visleafhm;

	hmanager_t		*csurfacehm;

	blockmap_t		*blockmap;

	ccmap3_t		*volume_ccmap3;
	ccmap3_t		*field_ccmap3;
	
} g_map_t;

#endif __g_mapdefs
