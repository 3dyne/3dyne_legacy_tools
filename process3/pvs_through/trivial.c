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



// trivial.c

#include "pvs.h"

int	flood_num;

void TrivialRejectFlood( portal_t *ref, visleaf_t *leaf, int count )
{
	portal_t	*test;
	visleaf_t	*otherleaf;
	int		i;
	fp_t		d;

	leaf->count = count;

	// this leaf can be trivialy seen through 'ref'
	ref->trivial_see[leaf->bitpos>>3] |= 1<<(leaf->bitpos&7);

	for ( test = leaf->portals; test; test=test->next )
	{
		otherleaf = test->otherleaf;

		// don't flood through closed portals
		if ( !otherleaf )
			continue;

		// don't flood into already flooded visleafs
		if ( otherleaf->count == count )
			continue;
		
		// at least one point of 'ref' has to be behind 'test'
		for ( i = 0; i < ref->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( ref->p->p[i], test->pl->norm ) - test->pl->dist;
			if ( d < 0 )
				break;
		}
		if ( i == ref->p->pointnum )
		{
			// no point of refportal is backside
			continue;
		}

		// at least one point of 'test' has to be in front of 'ref'
		for ( i = 0; i < test->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( test->p->p[i], ref->pl->norm ) - ref->pl->dist;
			if ( d > 0 )
				break;
		}
		if ( i == test->p->pointnum )
		{
			// no point of testportal frontside of refportal
			continue;
		}
//		printf( "*" );

		flood_num++;
		TrivialRejectFlood( ref, otherleaf, count );
	}
}

void TrivialRejectFlood_old( portal_t *ref, visleaf_t *leaf, int count )
{
	portal_t		*test;
	visleaf_t		*otherleaf;
	int			i;
	fp_t		d;

	leaf->count = count;      

	// mark leaf as trivial seen in portal
	ref->trivial_see[leaf->bitpos>>3] |= 1<<(leaf->bitpos&7);

	for ( test = leaf->portals; test ; test=test->next )
	{
#if 0	
		GLC_ConnectServer("");
		GLC_BeginList( "p1", 1 );
		GLC_SetColor( GLC_COLOR_RED );
		GLC_DrawPolygon( ref->p );
		GLC_SetColor( GLC_COLOR_GREEN );
		GLC_DrawPolygon( test->p );
		GLC_EndList();
		GLC_DisconnectServer();
		getchar();
#endif
		if ( ref == test )
			continue;

		otherleaf = test->otherleaf;
		if ( !otherleaf )
		{
			// portal is closed
			continue;
		}

		if ( otherleaf->count == count )
		{
			// leaf allready flooded
			continue;
		}
		
		// is any point of refportal backside of testportal
		for ( i = 0; i < ref->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( ref->p->p[i], test->pl->norm ) - test->pl->dist;
			if ( d < 0 )
				break;
		}
		if ( i == ref->p->pointnum )
		{
			// no point of refportal is backside
			continue;
		}

		// is any point of testportal backside of refportal
		for ( i = 0; i < test->p->pointnum; i++ )
		{
			d = Vec3dDotProduct( test->p->p[i], ref->pl->norm ) - ref->pl->dist;
			if ( d < 0 )
				break;
		}
		if ( i == test->p->pointnum )
		{
			// no point of testportal frontside of refportal
			continue;
		}

//		printf( "ok\n" );

		// ok, flood through portal
		flood_num++;
		TrivialRejectFlood( ref, otherleaf, count );
	}
}

void TrivialReject( visleaf_t *list )
{
	visleaf_t	*vl;
	portal_t	*pt;

	int		count;

	int		seen_leafnum;
	unsigned char	trivial_see[SEE_BUFFER_SIZE];

	int		i, j, leafnum;
	char		tt[256];
	hpair_t		*pair;

	printf( "trivial reject ...\n" );

	// reset count
	count = 1;
	for ( vl = list, leafnum = 0; vl ; vl=vl->next, leafnum++ )
		vl->count = 0;

	// for all visleafs
	for ( vl = list; vl ; vl=vl->next )
	{
		printf( "visleaf: " );

		// look through all portals
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			// don't look through closed portals
			if ( !pt->otherleaf )
				continue;
			
			// don't flood back into start visleaf
			vl->count = count;
			flood_num = 1;	// don't forget visleaf itself
			memset( pt->trivial_see, 0, SEE_BUFFER_SIZE );
			TrivialRejectFlood( pt, pt->otherleaf, count );
			printf( "%d ", flood_num );
			count++;

			pt->trivial_see_num = flood_num;
			pt->state = PortalState_trivial;

			// insert trivial_see as through_see1 into visportal
			pair = NewHPair2( "bstring", "through_see1", "x" );
			BstringCastToHPair( pt->trivial_see, leafnum/8+1, pair );
			InsertHPair( pt->self, pair );	
		}
		printf( "\n" );

		// insert bitpos intp visleaf
		sprintf( tt, "%d", vl->bitpos );
		pair = NewHPair2( "int", "bitpos", tt );
		InsertHPair( vl->self, pair );
#if 0		
		pair = NewHPair2( "bstring", "trivial_see1", "x" );
		BstringCastToHPair( trivial_see, leafnum/8+1, pair );
		InsertHPair( vl->self, pair );
#endif
	}
}
