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



// complex.c

#include "pvs.h"
#include "visclip.h"

/*
  ==================================================
  complex pvs

  ==================================================
*/

/*
  ====================
  ComplexRejectFlood

  ====================
*/

static int total_complex_num = 0;

int	vis1_fail = 0;
int	vis2_fail = 0;
int	p2_fail = 0;
int	p1_num = 0;

bool_t	debug_pvs;

void RejectFloodRecursive( portal_t *reference, polygon_t *refpoly, portal_t *test, polygon_t *ptest, int count, unsigned char *see_through )
{
	visleaf_t	*leaf;
	visclip_t	*vis;
	visclip_t	*vis2;
	portal_t	*pt;
	polygon_t	*p, *p2;
	polygon_t	*refpoly2;
	int		i;

	polygon_t	*front, *back;
	polygon_t	*tmp;

//	printf( "ref %p, test %p\n", reference, test );
       

	if ( !refpoly || !reference || !test )
		Error( "null refpoly\n" );

	leaf = test->otherleaf;    

	if ( !leaf )
		Error( "test portal not open.\n" );

	if ( leaf->count == count )
	{
//		printf( "." );
//		return;
	}
	leaf->count = count;

//	test->test_num++;

	total_complex_num++;

	reference->complex_see[leaf->bitpos>>3] |= 1<<(leaf->bitpos&7);
	see_through[leaf->bitpos>>3] |= 1<<(leaf->bitpos&7);

#ifdef GLC
	if ( debug_pvs )
	{
		GLC_ConnectServer("");
		GLC_BeginList( "p1", 1 );
		GLC_SetColor( GLC_COLOR_RED );
		GLC_DrawPolygon( refpoly );
		GLC_SetColor( GLC_COLOR_GREEN );
		GLC_DrawPolygon( test->p );
		GLC_EndList();
		GLC_DisconnectServer();
	}
#endif

	vis = SetupNewVisclip_2( refpoly, ptest, true );
//	vis = SetupNewVisclip_2( refpoly, test->p, true );
//	vis = SetupNewVisclip( refpoly, test->p, true );
	if ( !vis )
	{
		vis1_fail++;
//		printf( "visclip setup failed.\n" );
		return;
	}

#ifdef GLC
	if ( debug_pvs )
	{
		GLC_ConnectServer("");
		GLC_BeginList( "vis1", 1 );
		GLC_SetColor( GLC_COLOR_BLUE );
		DrawVisclip( vis );
		GLC_EndList();
		GLC_DisconnectServer();
//		getchar();
	}
#endif

	for ( pt = leaf->portals; pt ; pt=pt->next )
	{

#ifdef GLC
		if ( debug_pvs )
		{
			GLC_ConnectServer("");
			GLC_BeginList( "pt", 1 );
			GLC_SetColor( GLC_COLOR_YELLOW );
			GLC_DrawPolygon( pt->p );
			GLC_EndList();
			GLC_DisconnectServer();
			getchar();
		}
#endif		
//	printf( ".");
		if ( pt->pl == test->pl->flipplane )
		{
//			printf( "plane\n" );
			continue;
		}

		if ( !pt->otherleaf )
		{
//			printf( "closed\n" );
			continue;
		}

		if ( ! (reference->trivial_see[pt->otherleaf->bitpos>>3] & 1<<(pt->otherleaf->bitpos&7) ) )
		{
			// otherleaf is not even trivial seen
			// ignore it
//			printf( "not trivial\n" );
			continue;
		}

#if 1
		if ( pt->state == PortalState_complex )
		{
			for ( i = 0; i < SEE_BUFFER_SIZE; i++ )
			{
				if ( (~reference->complex_see[i]) & pt->complex_see[i] )
					goto see_more;
			}
//			printf( "complex not more\n" );
			continue;
		}

		else if ( pt->state == PortalState_trivial )
		{
			for ( i = 0; i < SEE_BUFFER_SIZE; i++ )
			{
				if ( (~reference->complex_see[i]) & pt->trivial_see[i] )
					goto see_more;
			}
//			printf( "trivial not more\n" );
			continue;
		}

#endif
see_more:
// 		printf( "*" );

		p1_num++;

#if 1
		// 'to' back of 'from' ?
		p2 = CopyPolygon( pt->p );

		ClipPolygonInPlace( &p2, reference->pl->norm, reference->pl->dist );
		if ( !p2 )
			continue;

 		p = PolygonVisclip( vis, p2 );
		FreePolygon( p2 );

#ifdef GLC
		if ( debug_pvs )
		{
			GLC_ConnectServer("");
			GLC_BeginList( "to", 1 );
			GLC_SetColor( GLC_COLOR_YELLOW );
			GLC_DrawPolygon( p );
			GLC_EndList();
			GLC_DisconnectServer();
			getchar();
		}
#endif	

#endif

		// 'from' back of 'to'
		refpoly2 = CopyPolygon( refpoly );
#if 1
		ClipPolygonInPlace( &refpoly2, pt->pl->norm, pt->pl->dist );
		if ( !refpoly2 )
		{
			FreePolygon( p );
			continue;
		}
#endif

#if 1

		if ( p )
		{
//			printf( "yes " );
		}
		else
		{
//			printf( "no " );
			FreePolygon( refpoly2 );
			continue;
		}

		// look back
		vis2 = SetupNewVisclip_2( p, test->p, true );
//		vis2 = SetupNewVisclip( p, test->p );

#ifdef GLC
		if ( debug_pvs )
		{
			GLC_ConnectServer("");
			GLC_BeginList( "vis2", 1 );
			GLC_SetColor( GLC_COLOR_YELLOW );
			DrawVisclip( vis2 );
			GLC_EndList();
			GLC_DisconnectServer();
//		getchar();
		}
#endif
		if ( !vis2 )
		{
			vis2_fail++;
//			printf( "visclip setup failed.\n" );
			FreePolygon( p );
			FreePolygon( refpoly2 );
			continue;
		}
				

		tmp = refpoly2;
		refpoly2 = PolygonVisclip( vis2, refpoly2 );
		FreePolygon( tmp );

		FreeVisclip( vis2 );

		if ( refpoly2 )
		{
//			printf( "yes2 " );
		}
		else
		{
//			printf( "no2 " );
			p2_fail++;
			FreePolygon( p );
			continue;
		}

      
		RejectFloodRecursive( reference, refpoly2, pt, p, count, see_through );
		FreePolygon( refpoly2 );
		FreePolygon( p );

#endif
	}

	FreeVisclip( vis );
//	printf( "\n" );
}

void ComplexRejectFlood( visleaf_t *startleaf, portal_t *reference, int count )
{
	portal_t	*pt;
	int		i;



	for ( pt = startleaf->portals; pt ; pt=pt->next )
	{
		if ( pt == reference )
		{
//			printf( "self " );
			continue;
		}
		if ( pt->pl == reference->pl )
		{
//			printf( "same-plane " );
			continue;
		}
		if ( !pt->otherleaf )
		{
//			printf( "closed " );
			continue;
		}
//		printf( "open: " );

		if ( !strcmp( pt->self->name, "#158269" ) &&
		     !strcmp( reference->self->name, "#158276" ) )			
		{
//			debug_pvs = true;
		}
		else
		{
			debug_pvs = false;
		}

		RejectFloodRecursive( reference, reference->p, pt, pt->p, count, pt->see_through );
		count++;
	}
}

/*
  ====================
  GetBestTrivial

  ====================
*/
void GetBestTrivial( visleaf_t *list, visleaf_t **bestleaf, portal_t **bestportal )
{
	int		min;
	portal_t	*pt;
	visleaf_t	*vl;

	min = 1<<30;
	*bestleaf = NULL;
	*bestportal = NULL;

	for ( vl = list; vl ; vl=vl->next )
	{
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			if ( pt->state != PortalState_trivial )
				continue;

			if ( pt->trivial_see_num < min )
			{
				*bestportal = pt;
				*bestleaf = vl;
				min = pt->trivial_see_num;
			}
		}
	}
}

visleaf_t	*vl_lookup[32000];
int		vl_num;

int CountTrivials( visleaf_t *list, portal_t *test )
{
	visleaf_t	*vl;
	portal_t	*pt;
	int		i, j;
	int		bitpos, sum;

	sum = 0;
		
	for ( i = 0; i < vl_num; i++ )
	{
		if ( test->trivial_see[i>>3] & (1<<(i&7) ) )
		{
			if ( vl_lookup[i] )
			{
				sum+=vl_lookup[i]->trivial_seen_portal_num;
			}
		}
	}
	return sum;
}

void InitVisleafLookup( visleaf_t *list )
{
	visleaf_t	*vl;

	memset( vl_lookup, 0, sizeof( vl_lookup ) );
	vl_num = 0;
	for ( vl = list; vl ; vl=vl->next )
	{
		vl_lookup[vl->bitpos] = vl;
		vl_num++;
	}
}

void GetBestTrivial2( visleaf_t *list, visleaf_t **bestleaf, portal_t **bestportal )
{
	int		min, min2;
	portal_t	*pt;
	visleaf_t	*vl;
	int		count;

	min2 = min = 1<<30;
	*bestleaf = NULL;
	*bestportal = NULL;

	for ( vl = list; vl ; vl=vl->next )
	{
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			if ( pt->state != PortalState_trivial )
				continue;

			count = CountTrivials( list, pt );
			if ( count <= min )
			{
				if ( pt->trivial_see_num < min2 )
				{
					*bestportal = pt;
					*bestleaf = vl;
					min2 = pt->trivial_see_num;
				}
				min = count;
			}
#if 0
			if ( pt->trivial_see_num < min )
			{
				*bestportal = pt;
				*bestleaf = vl;
				min = pt->trivial_see_num;
			}
#endif
		}
	}
}

bool_t CheckSame( vec3d_t v1, vec3d_t v2 )
{
	int		i;
	for ( i = 0; i < 3; i++ )
	{
		if ( fabs(v1[i]-v2[i]) > 0.5 )
			return false;
	}
	return true;
}

portal_t * FindAppropriatePortalInVisleaf( visleaf_t *otherleaf, portal_t *ref )
{
	int		i, j;
	portal_t		*pt;
	
	for ( pt = otherleaf->portals; pt; pt=pt->next )
	{
		if ( pt->pl != ref->pl->flipplane )
			continue;

		if ( pt->p->pointnum != ref->p->pointnum )
			continue;

		for ( i = 0; i < ref->p->pointnum; i++ )
		{
			for ( j = 0; j < pt->p->pointnum; j++ )
			{
				if ( CheckSame( pt->p->p[j], ref->p->p[i] ) )
					break;
			}
			if ( j == pt->p->pointnum )
			{
				break;
			}
		}
		if ( i == ref->p->pointnum )
			return pt;
	}	
	return NULL;
}

void CalcLeafVisibility( visleaf_t *list )
{
	visleaf_t	*vl;
	portal_t	*pt;
	int		i, j;
	int		leafnum;
	hpair_t		*pair;
	portal_t	*ap;
	int		diff_num;
	char		tt[256];
	unsigned char	complex_see[SEE_BUFFER_SIZE];
	unsigned char	through_see[SEE_BUFFER_SIZE];

	for ( vl = list, leafnum = 0; vl ; vl=vl->next, leafnum++ )
	{ 
		vl->count = 0;
	}

	for ( vl = list; vl ; vl=vl->next )
	{
		memset( complex_see, 0, SEE_BUFFER_SIZE );	   
		memset( through_see, 0, SEE_BUFFER_SIZE );	   

		// sees own leaf
		complex_see[vl->bitpos>>3] |= 1<<(vl->bitpos&7);

		printf( "visleaf: " );
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			pt->see_through_num = 0;
			pt->complex_see_num = 0;
			for ( i = 0; i < SEE_BUFFER_SIZE; i++ )
			{
				if ( pt->state == PortalState_complex )
				{
					complex_see[i] |= pt->complex_see[i];
				}
				else if ( pt->state == PortalState_trivial )
				{
					
				}
				
				// count complex see
				for ( j = 0; j < 8; j++ )
				{
					if ( pt->complex_see[i] & 1<<j )
						pt->complex_see_num++;
				}

				through_see[i] |= pt->see_through[i];
			}

			if ( pt->otherleaf )
			{
//				ap = FindAppropriatePortalInVisleaf( pt->otherleaf, pt );
//				if ( !ap )
//					Error( "can't find appropriate portal in otherleaf.\n" );
//				pt->see_through_num = ap->complex_see_num;
//				printf( "t%d-c%d ", pt->see_through_num, pt->complex_see_num );
				RemoveAndDestroyAllHPairsOfKey( vl->self, "through_see" );	
				pair = NewHPair2( "bstring", "through_see", "x" );
				BstringCastToHPair( pt->see_through, leafnum/8+1, pair );
				InsertHPair( pt->self, pair );
			}
			else
			{
				printf( "c%d ", pt->complex_see_num );
				pt->see_through_num = 0;
			}

			sprintf( tt, "%d", pt->complex_see_num );
			pair = NewHPair2( "int", "complex_num", tt );
			InsertHPair( pt->self, pair );

			sprintf( tt, "%d", pt->see_through_num );
			pair = NewHPair2( "int", "through_num", tt );
			InsertHPair( pt->self, pair );


		}
		printf( "\n" );

		RemoveAndDestroyAllHPairsOfKey( vl->self, "complex_see" );

		pair = NewHPair2( "bstring", "complex_see", "x" );
		BstringCastToHPair( complex_see, leafnum/8+1, pair );
		InsertHPair( vl->self, pair );	

		diff_num = 0;
		for ( i = 0; i < SEE_BUFFER_SIZE; i++ )
		{
			for ( j = 0; j < 8; j++ )
			{
				if ( ( through_see[i] & (1<<j)) ^ ( complex_see[i] & (1<<j) ) )
					diff_num++;
			}
		}
		printf( "diff_num: %d\n", diff_num );
	}
}

#include <signal.h>
extern hmanager_t *global_visleafhm;
visleaf_t *global_visleaflist;
void SaveComplex( void )

{
	FILE		*h;

	printf( "SIGHUP: save current work ...\n" );

	CalcLeafVisibility( global_visleaflist );	

	h = fopen( "_pvs_sighup.hobj", "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( HManagerGetRootClass( global_visleafhm ), h );
	fclose( h );	
}

void ComplexReject( visleaf_t *list )
{
	portal_t	*bestportal;
	visleaf_t	*bestleaf;
	int		count = 1;
 
	printf( "complex reject ...\n" );

	// SIGHUP, safe current work
	global_visleaflist = list;

	signal( 1, SaveComplex );

	InitVisleafLookup( list );
 
	for (;;)
	{
		GetBestTrivial( list, &bestleaf, &bestportal );

		if ( !bestportal || !bestleaf )
			break;

		bestleaf->trivial_seen_portal_num--;
		if ( !strcmp( bestleaf->self->name, "#158266" ) &&
		     !strcmp( bestportal->self->name, "#158276" ) )			
		{
			printf( "debug pvs\n" );
//			debug_pvs = true;
		}
		else
		{
			debug_pvs = false;
		}
		printf( "best: %d ", bestportal->trivial_see_num );
		ComplexRejectFlood( bestleaf, bestportal, count );
		printf( "\n" );
		bestportal->state = PortalState_complex;
//		count++;
	}

	printf( "vis1_fail: %d\n", vis1_fail );
	printf( "vis2_fail: %d\n", vis2_fail );
	printf( "p2_fail: %d\n", p2_fail );
	printf( "p1_num: %d\n", p1_num );

	CalcLeafVisibility( list );

	printf( "total complex: %d\n", total_complex_num );
}
