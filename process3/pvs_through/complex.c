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

int	vis1_fail = 0;
int	vis2_fail = 0;
int	p2_fail = 0;
int	p3_fail = 0;
int	p4_fail = 0;
int	p1_fail = 0;

bool_t	debug_pvs;

void RejectFloodRecursive_old( portal_t *reference, polygon_t *refpoly, portal_t *test, polygon_t *ptest, int count, unsigned char *see_through )
{
#if 0
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
#endif
}

void ComplexRejectFlood_old( visleaf_t *startleaf, portal_t *reference, int count )
{
#if 0
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
#endif
}

/*
  ==============================
  ComplexRejectFlood

  ==============================
*/
static int g_leafnum;
static int	total_complex_num = 0;

void ComplexRejectFlood( portal_t *reference, visleaf_t *enter_leaf, portal_t *enter_portal, polygon_t *enter_poly, portal_t *from, polygon_t *pfrom, int count )
{
	int		bitpos;
	visclip_t       *vis;
	portal_t	*pt;
	int		see_bytes;
	int	i;

	total_complex_num++;

	see_bytes = g_leafnum/8+1;

	enter_leaf->count = count;
	bitpos = enter_leaf->bitpos;
	
	reference->complex_see[bitpos>>3] |= 1U<<(bitpos&7);	

	vis = SetupNewVisclip_2( pfrom, enter_poly, true );
	if ( !vis )
	{
		vis1_fail++;
		return;
	}
#ifdef GLDBG
	GLD_BeginList( gldbg, "vis1", "line" );

	GLD_Color3f( gldbg, 1, 0, 0 );
	DrawVisclip( vis );

	GLD_EndList( gldbg );


	GLD_BeginList( gldbg, "poly1", "fill" );

	GLD_Color3f( gldbg, 0.3, 0.3, 0.3 );
	GLD_EasyPolygon( gldbg, reference->p );

	GLD_Color3f( gldbg, 0, 0, 1 );
	GLD_EasyPolygon( gldbg, pfrom );

	GLD_Color3f( gldbg, 1, 0, 0 );
	GLD_EasyPolygon( gldbg, enter_poly );

	

	GLD_EndList( gldbg );


	// clear
	GLD_BeginList( gldbg, "poly2", "fill" );
	GLD_EndList( gldbg );
	GLD_BeginList( gldbg, "vis2", "line" );
	GLD_EndList( gldbg );	
	
	GLD_Update( gldbg );
#endif
	
	for ( pt = enter_leaf->portals; pt; pt=pt->next )
	{

		// same plane
		if ( pt->pl == enter_portal->pl->flipplane )
			continue;

		// don't look through closed portals
		if ( !pt->otherleaf )
			continue;

//		if ( pt->otherleaf->count == count )
//			continue;
		

		// otherleaf not even trivial seen
		if ( ! ( reference->trivial_see[pt->otherleaf->bitpos>>3] & (1U<<(pt->otherleaf->bitpos&7)) ) )
			continue;

#if 1	
		if ( pt->state == PortalState_complex )
		{
			for ( i = 0; i < see_bytes; i++ )
			{
				if ( ( ~reference->complex_see[i] &
				       pt->complex_see[i] ) )
					goto see_more;
			}
			continue;
		}
#endif	

#if 1
		if ( pt->state == PortalState_trivial ) 
		{
			for ( i = 0; i < see_bytes; i++ )
			{
				if ( ( ~reference->complex_see[i] &
				       pt->trivial_see[i] ) )
					goto see_more;
			}
			continue;
		}
#endif
		
	see_more:
		
#ifdef GLDBG
		GLD_BeginList( gldbg, "poly2", "fill" );
		GLD_Color3f( gldbg, 1, 1, 0 );
		GLD_EasyPolygon( gldbg, pt->p );
		GLD_EndList( gldbg );
		
		GLD_Update( gldbg );
#endif			
		{
			polygon_t	*ptcopy = NULL;
			polygon_t	*pfromcopy = NULL;
			visclip_t	*vis2 = NULL;

			ptcopy = CopyPolygon( pt->p );
			ClipPolygonInPlace( &ptcopy, from->pl->norm, from->pl->dist );
//			ClipPolygonInPlace( &ptcopy, enter_portal->pl->flipplane->norm, enter_portal->pl->flipplane->dist );
			
			if ( !ptcopy )
			{
				p1_fail++;
				goto clean_up;
			}

			pfromcopy = CopyPolygon( pfrom );
//			ClipPolygonInPlace( &pfromcopy, pt->pl->norm, pt->pl->dist );
			ClipPolygonInPlace( &pfromcopy, pt->pl->norm, pt->pl->dist );
			
			if ( !pfromcopy )
			{
				p2_fail++;
				goto clean_up;
			}

			PolygonVisclipInPlace( vis, &ptcopy );
			if ( !ptcopy )
			{
				p3_fail++;
				goto clean_up;
			}

			vis2 = SetupNewVisclip_2( ptcopy, enter_poly, true );
			
			if ( !vis2 )
			{
				vis2_fail++;
				goto clean_up;
			}
#ifdef GLDBG
			GLD_BeginList( gldbg, "vis2", "line" );
			GLD_Color3f( gldbg, 0, 1, 0 );
			DrawVisclip( vis2 );
			GLD_EndList( gldbg );
			GLD_Update( gldbg );			
#endif

			PolygonVisclipInPlace( vis2, &pfromcopy );
			if ( !pfromcopy )
			{
				p4_fail++;
				goto clean_up;
			}
			ComplexRejectFlood( reference, pt->otherleaf, pt, ptcopy, from, pfromcopy, count );

		clean_up:
			if ( ptcopy )
				FreePolygon( ptcopy );
			if ( pfromcopy )
				FreePolygon( pfromcopy );
			if ( vis2 )
				FreeVisclip( vis2 );
		}
	}
	
	FreeVisclip( vis );
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
			// don't look through closed portals
			if ( !pt->otherleaf )
				continue;
			
			// already done
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


void BuildThroughSeePair( visleaf_t *list )
{
	visleaf_t	*vl;
	portal_t	*pt;
	int		i, j;
	hpair_t		*pair;
	portal_t	*ap;
	int		diff_num;
	char		tt[256];

	for ( vl = list; vl ; vl=vl->next )
	{
		for ( pt = vl->portals; pt ; pt=pt->next )
		{
			if ( pt->otherleaf )
			{
				pair = NewHPair2( "bstring", "through_see", "x" );
				BstringCastToHPair( pt->complex_see, g_leafnum/8+1, pair );
				InsertHPair( pt->self, pair );
			}
		}
	}
}

void ComplexReject( visleaf_t *list, char *debug_visleaf )
{
	portal_t	*bestportal;
	portal_t	*pt;
	visleaf_t	*bestleaf, *vl;
	int		count = 1;
 
	g_leafnum = 0;
	for ( vl = list; vl ; vl=vl->next )
	{
		vl->count = 0;
		g_leafnum++;
	}
	
	printf( "complex reject ...\n" );

	for (;;)
	{
		GetBestTrivial( list, &bestleaf, &bestportal );

		if ( !bestportal || !bestleaf )
			break;

		printf( "best: %d ", bestportal->trivial_see_num );



		for ( pt = bestleaf->portals; pt ; pt=pt->next )
		{

			if ( debug_visleaf )
			{
				if ( !strcmp( bestleaf->self->name, debug_visleaf ) &&
				     !strcmp( bestportal->self->name, "#564" ) &&
				     !strcmp( pt->self->name, "#560" ) )
				{
					printf( "***** found visleaf to debug *****\n" );
					GLD_StartRecord( gldbg );
				}
				else
				{
					GLD_StopRecord( gldbg );	
				}
			}

			// same plane, also portal itself
			if ( pt->pl == bestportal->pl )
				continue;
			
			// don't flood back into start visleaf
			bestleaf->count = count;
			ComplexRejectFlood( bestportal, bestportal->otherleaf, bestportal, bestportal->p, pt, pt->p, count );			
			count++;				
		}

		printf( "\n" );
		bestportal->state = PortalState_complex;
	}

	printf( "vis1_fail: %d\n", vis1_fail );
	printf( "vis2_fail: %d\n", vis2_fail );
	printf( "p1_fail: %d\n", p1_fail );
	printf( "p2_fail: %d\n", p2_fail );
	printf( "p3_fail: %d\n", p3_fail );
	printf( "p4_fail: %d\n", p4_fail );

	BuildThroughSeePair( list );
	printf( "total complex: %d\n", total_complex_num );
}
