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



// g_ccmap3.h

/*
  =============================================================================
  compressed cluster map3

  =============================================================================
*/

#ifndef __g_ccmap3
#define __g_ccmap3

#include "interfaces.h"
#include "defs.h"
#include "shared.h"

//#include "lib_math.h"
#include "u_expand.h"

typedef struct ccluster_s
{
	ivec3d_t		pos;
	struct ccluster_s *next;	// next in hashline

	cinfo_t		cinfo;		// compression info
	unsigned char	cdata[16];	// compressed data, variable sized
	
} ccluster_t;

typedef struct ccmap3_s
{
	int		hashsize;
	int		clusternum;
	int		clustersize;
	int		cellbytes;

	int		clustercellnum;

	int		hashshift;
	int		keyshift;
	int		keymask;

	int		clusterunits; 
	int		cellunits;

	ccluster_t	*hash[256];	// variable sized
} ccmap3_t;

ccmap3_t * G_NewCCMap3( int hashsize, int clustersize, int cellbytes );
void G_FreeCCMap3( ccmap3_t *map );
void G_CCMap3InsertCCluster( ccmap3_t *map, ccluster_t *cc );
ccluster_t * G_CCMap3FindCCluster( ccmap3_t *map, ivec3d_t pos );	// pos in ccmap3 units
void G_CCMap3RemoveCCluster( ccmap3_t *map, ccluster_t *cc );
ccluster_t * G_NewCCluster( int datasize );
ccmap3_t * G_LoadCCMap3Binary( char *name );

void G_Vec3dToCCMap3Units( ccmap3_t *map, ivec3d_t out, ivec3d_t in );

#endif
