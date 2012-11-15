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



// pvs_mpi_complex.c

#include "pvs_mpi.h"
#include "visclip.h"

extern int	visleaf_num;
extern int	portal_num;
extern visleaf_t	visleafs[];
extern portal_t		portals[];


void RejectFloodRecursive( portal_t *reference, polygon_t *refpoly, portal_t *test, polygon_t *ptest, unsigned char *through_see )
{
	int		i, j;
	int		bitpos;
	visleaf_t	*leaf;
//	portal_t	*test;
	portal_t	*pt;
	visclip_t	*vis;

	bitpos = test->otherleaf;
	leaf = &visleafs[test->otherleaf];

	reference->complex_see[bitpos>>3] |= 1<<(bitpos&7);
	through_see[bitpos>>3] |= 1<<(bitpos&7);

	vis = SetupNewVisclip_2( refpoly, ptest, true );
	if ( !vis )
		return;

	for ( i = 0; i < leaf->portalnum; i++ )
	{
		pt = &portals[i+leaf->startportal];

		if ( pt->pl == test->pl->flipplane )
			continue;		// same or same plane

		if ( pt->otherleaf == -1 )
			continue;		// closed portal

		if ( ! (reference->trivial_see[pt->otherleaf>>3] & 1<<(pt->otherleaf&7) ) )
			continue;		// otherleaf is not even trivial seen

		if ( pt->state == PortalState_complex )
		{
			for ( j = 0; j < SEE_BUFFER_SIZE; j++ )
			{
				if ( (~reference->complex_see[j]) & pt->complex_see[j] )
					goto see_more;
			}
			continue;
		}

		else if ( pt->state == PortalState_trivial )
		{
			for ( j = 0; j < SEE_BUFFER_SIZE; j++ )
			{
				if ( (~reference->complex_see[j]) & pt->trivial_see[j] )
					goto see_more;
			}
			continue;
		}

	see_more:
		{
			polygon_t	*p = NULL;
			polygon_t	*refpoly2 = NULL;
			visclip_t	*vis2 = NULL;

			p = CopyPolygon( pt->p );
			ClipPolygonInPlace( &p, reference->pl->norm, reference->pl->dist );
			if ( !p )
				goto clean_up;

			refpoly2 = CopyPolygon( refpoly );
			ClipPolygonInPlace( &refpoly2, pt->pl->norm, pt->pl->dist );
			if ( !refpoly2 )
				goto clean_up;

			PolygonVisclipInPlace( vis, &p );
			if ( !p )
				goto clean_up;

			vis2 = SetupNewVisclip_2( p, test->p, true );
			if ( !vis2 )
				goto clean_up;
			
			PolygonVisclipInPlace( vis2, &refpoly2 );
			if ( !refpoly2 )
				goto clean_up;

			RejectFloodRecursive( reference, refpoly2, pt, p, through_see );

		clean_up:
			if ( p )
				FreePolygon( p );
			if ( refpoly2 ) 
				FreePolygon( refpoly2 );
			if ( vis2 )
				FreeVisclip( vis2 );
			
		}
	}
	
	FreeVisclip( vis );
}

void ComplexRejectFlood( int reference )
{
	int		i;
	visleaf_t	*vl;
	portal_t	*pt;

	vl = &visleafs[portals[reference].visleaf];

	for ( i = 0; i < vl->portalnum; i++ )
	{
		if ( reference == i+vl->startportal )
			continue;	// same portal

		pt = &portals[i+vl->startportal];

		if ( pt->otherleaf == -1 )
			continue;	// closed portal
		
		if ( pt->pl == portals[reference].pl )
			continue;	// same plane

		RejectFloodRecursive( &portals[reference], portals[reference].p, pt, pt->p, pt->through_see );
	}
}
