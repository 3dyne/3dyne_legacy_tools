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



// pvs.h

#ifndef __pvs
#define __pvs

//#define GLC

#include <stdio.h>
#include <stdlib.h>
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
#include "gl_client.h"


#define SEE_BUFFER_SIZE		( 512 )
#define MAX_PORTALS		( 8 * SEE_BUFFER_SIZE )

typedef enum
{
	PortalState_none = 0,
	PortalState_trivial,
	PortalState_complex
} portalState;

typedef struct portal_s
{
	hobj_t		*self;
	cplane_t		*pl;
	polygon_t		*p;

	portalState	state;

	int		trivial_see_num;
	unsigned char	trivial_see[SEE_BUFFER_SIZE];
	int		complex_see_num;
	unsigned char	complex_see[SEE_BUFFER_SIZE];

	// open portal
	int		see_through_num;
	unsigned char	see_through[SEE_BUFFER_SIZE];

	struct visleaf_s	*otherleaf;
	struct portal_s		*next;
} portal_t;

typedef struct visleaf_s
{
	hobj_t		*self;
	int		bitpos;

	int		trivial_seen_portal_num;

	int		count;		// for flood stop
	struct visleaf_s *next;

	struct portal_s	*portals;
} visleaf_t;


#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

void TrivialReject( visleaf_t *list );
void ComplexReject( visleaf_t *list );

#endif
