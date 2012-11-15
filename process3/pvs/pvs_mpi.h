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



// pvs_mpi.h

#ifndef __pvs_mpi

#include <stdio.h>
#include <stdlib.h>

#include <mpi.h>

#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#include "../csg/cbspbrush.h"

// shared
#define TAG_READY	( 1 )
#define TAG_IN_PLANE_NAME	( 2 )
#define TAG_IN_VISLEAF_NAME	( 3 )
#define TAG_LOAD_CLASSES	( 4 )
#define TAG_RUN_TRIVIAL_REJECT	( 5 )
#define TAG_QUIT		( 6 )

#define TAG_PORTALNUM_REQ	( 7 )
#define TAG_PORTALNUM		( 8 )
#define TAG_PORTAL_TRIVIAL_SEE_REQ	( 9 )
#define TAG_PORTAL_TRIVIAL_SEE		( 10 )

#define TAG_RUN_COMPLEX_SEE_ON_PORTAL	( 11 )
#define TAG_DONE_COMPLEX_SEE_ON_PORTAL	( 12 )
#define TAG_COMPLEX_SEE_DATA	( 13 )
#define TAG_UPDATE_COMPLEX_SEE	( 14 )

#define TAG_SAVE_CLASS		( 15 )

#define TAG_THROUGH_SEE_REQ	( 16 )
#define TAG_THROUGH_SEE_DATA	( 17 )
#define TAG_UPDATE_THROUGH_SEE	( 18 )

#define ABORT_VISLEAF_LOAD_FAILED	( 1 )
#define ABORT_PLANE_LOAD_FAILED	( 1 )

typedef enum
{
	PortalState_none = 0,
	PortalState_trivial,
	PortalState_complex,
	PortalState_run_complex		// a node is working on it
} portalState;

#define SEE_BUFFER_SIZE		( 512 )
#define MAX_PORTALS		( 8 * SEE_BUFFER_SIZE )

#define MAX_VISLEAFS		( 4096 )

// slave
typedef struct visleaf_s
{
	hobj_t		*self;

	int		count;
	int		portalnum;
	int		startportal;
} visleaf_t;

typedef struct portal_s
{
	hobj_t		*self;

	int		visleaf;	// portal is part of this visleaf

	cplane_t	*pl;
	polygon_t	*p;

	portalState	state;

	int		trivial_see_num;
	unsigned char	trivial_see[SEE_BUFFER_SIZE];
	int		complex_see_num;
	unsigned char	complex_see[SEE_BUFFER_SIZE];	
	
	// open portals
	int		through_see_num;
	unsigned char	through_see[SEE_BUFFER_SIZE];

	int		otherleaf;
} portal_t;

typedef struct mportal_s
{
	portalState	state;
	int		run_on_node;
	int		trivial_see_num;
	unsigned char	complex_see[SEE_BUFFER_SIZE];
} mportal_t;

#endif
