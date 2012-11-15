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



// bsp.c

#include "bsp2.h"

#undef BSP_MID_SPLIT_PASS0		// disabled

//int		p_planenum;
//plane_t		p_planes[MAX_PLANES];

//int		p_planecheck[MAX_PLANES]; // BSP_FindSplitPlane needs this for quick test

int		p_nodenum = 0;
//cbspnode_t	p_nodes[MAX_NODES];

//int		p_brushnum = 0;
//bspbrush_t	*p_brushes[MAX_BRUSHES];


static int		stat_solidnum = 0;	// brush leaf
static int		stat_emptynum = 0;	// real empty
static int		stat_empty2num = 0;	// empty leafs with contents
static int		stat_inempty2num = 0;	// number of brushes in 'empty leafs with contents'
// for p_plane quick test in BSP_FindSplitPlane
static int		bsp_counter;

/*
  ==================================================
  bspnode stuff

  ==================================================
*/
#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

cbspnode_t * NewNode( void )
{
	cbspnode_t	*node;

	node = NEW( cbspnode_t );
	p_nodenum++;

	return node;
}

void FreeNode( cbspnode_t *node )
{
	free( node );
}

/*
  ========================================
  
  brush split stuff

  ========================================
*/

/*
  ====================
  IsBrushExactOnPlane

  Needed by CSG_SplitBrush in bsp mode
  ====================
*/
bool_t IsBrushExactOnPlane( cbspbrush_t *in, cplane_t *pl, bool_t bsp_enable )
{
	int		i;
	cplane_t	*pl2;

	if ( !(pl->type & PLANE_POS ))
		pl = pl->flipplane;

	for ( i = 0; i < in->surfacenum; i++ )
	{
		pl2 = in->surfaces[i].pl;
		if ( !(pl2->type & PLANE_POS ) )
			pl2 = pl2->flipplane;

		if ( pl == pl2 )
		{
			// it's the same plane
			if ( bsp_enable )
			{
				if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
				{
					printf( " * IsBrushExactOnPlane : surface allready on node. *\n" );
					printf( " * brush id: %s*\n", in->original->name );
				}
				in->surfaces[i].state |= SURFACE_STATE_ONNODE;
			}
			return true;
		}
	}
	return false;
}

/*
  ====================
  CheckBrushWithPlane2

  Needed by CSG_SplitBrush
  ====================
*/
int CheckBrushWithPlane2( cbspbrush_t *in, vec3d_t norm, fp_t dist, bool_t bsp_enable )
{
	int		i;
	bool_t		front, back, on;
	int		what;
	
	back = front = on = false;

	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( !in->surfaces[i].p )
			continue; 

		what = CheckPolygonWithPlane( in->surfaces[i].p, norm, dist );

		if ( what == POLY_SPLIT )
			return BRUSH_SPLIT;

		if ( what == POLY_ON )
		{
			on = true;
#if 0
			if ( bsp_enable )
			{
				if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
				{
//					Error( "CheckBrushWithPlane2 in bsp mode: surface allready on node.\n" );
					printf( " * CheckBrushWithPlane2 in bsp mode: surface allready on node. *\n" );
					printf( " * brush id: %d *\n", in->original->id );
				}
				in->surfaces[i].state |= SURFACE_STATE_ONNODE;
			}
#endif			
			continue;
		}

		if ( what == POLY_FRONT )
		{
			if ( back )
				return BRUSH_SPLIT;
			front = true;
			continue;
		}

		if ( what == POLY_BACK )
		{
			if ( front )
				return BRUSH_SPLIT;
			back = true;
			continue;
		}
	}

	if ( !back )
	{
		if ( on )
			return BRUSH_FRONT_ON;
		else
			return BRUSH_FRONT;
	}

	if ( !front )
	{
		if ( on )
			return BRUSH_BACK_ON;
		else
			return BRUSH_BACK;
	}
	
	// can't happen
	Error( "CheckBrushWithPlane2: can't specify brush.\n" );
	return BRUSH_SPLIT;
}



/*
  ====================
  AddSurfaceToBrush

  Needed by CSG_SplitBrush
  ====================
*/
cbspbrush_t* AddSurfaceToBrush( cbspbrush_t *in, cplane_t *addpl, bool_t bsp_enable )
{
	int		i, j;
	int		surfnum;
	csurface_t	surfs[MAX_SURFACES_PER_BRUSH];
	cplane_t		*pl;
	cbspbrush_t	*bnew;

	// todo: contents
	
	// copy the original surfaces local
	for ( i = 0; i < in->surfacenum; i++ )
	{
		memcpy( &surfs[i], &in->surfaces[i], sizeof( csurface_t ) );
		surfs[i].p = NULL; // don't touch the original polys
	}
	surfnum = in->surfacenum;
		

	surfs[surfnum].pl = addpl;
	surfs[surfnum].td = NULL;
	surfs[surfnum].state = SURFACE_STATE_BYSPLIT; //bysplit = true;
	surfs[surfnum].contents = 0;	// or should it be set to NOT_VISIBLE
	if ( bsp_enable )
		surfs[surfnum].state |= SURFACE_STATE_ONNODE;
	surfnum++;

	for( i = 0; i < surfnum; i++ )
	{
		pl = surfs[i].pl;
		surfs[i].p = BasePolygonForPlane( pl->norm, pl->dist );

		for ( j = 0; j < surfnum; j++ )
		{
			if ( i == j )
				continue;

			if ( surfs[i].p )
			{
				pl = surfs[j].pl;
				ClipPolygonInPlace( &surfs[i].p, pl->norm, pl->dist );
			} 
		}
	}

	//
	// build new brush from all surfaces still got a poly
	//

	bnew = NewBrush( surfnum );
	bnew->original = in->original; 
	bnew->contents = in->contents;
	
	for ( i = 0, j = 0; i < surfnum; i++ )
	{
		if ( surfs[i].p )
		{
			memcpy( &bnew->surfaces[j], &surfs[i], sizeof( csurface_t ) );
			j++;
		}
	}
	bnew->surfacenum = j;

	if ( j == 0 )
	{
		FreeBrush( bnew );
		return NULL;
	}

	if ( j < 4 )
	{
		printf( " * AddSurfaceToBrush: new brush surfacenum %d < 4\n *", j );
		printf( " brush id: %s\n", in->original->name );
		FreeBrush( bnew );
		return NULL;
	}

	CalcBrushBounds( bnew );
	return bnew;
}


/*
  ====================
  CSG_SplitBrush

  the input brush is split
  front and back can contain a copy of the input brush or NULL
  or two new brushes.

  if bsp enabled, surfaces exactly on plane are set onnode=true.
  if onnode allready true, a warning is printed
  ====================
*/
void CSG_SplitBrush( cbspbrush_t *in, cplane_t *plane, cbspbrush_t **front, cbspbrush_t **back, bool_t bsp_enable )
{
	int		what;
	cplane_t		*pl;
	cplane_t		*plflip;
	bool_t		on;

	fp_t		v1, v2;
	polygon_t	*poly; // for debug

	on = false;
	*front = *back = NULL;

	if ( !in )
		return;

	pl = plane;
	plflip = plane->flipplane;

	on = IsBrushExactOnPlane( in, plane, bsp_enable );

	what = CheckBrushWithPlane2( in, pl->norm, pl->dist, bsp_enable );

	if ( on && ( what!=BRUSH_BACK_ON && what!=BRUSH_FRONT_ON ) )
	{
		Error( "on, what: %d\n", what );
	}

	if ( what == BRUSH_BACK ||
	     what == BRUSH_BACK_ON )
	{
		*back = CopyBrush( in );
		return;
	}

	if ( what == BRUSH_FRONT ||
	     what == BRUSH_FRONT_ON )
	{
		*front = CopyBrush( in );
		return;
	}

	//
	// split brush
	//

	if ( on )
		Error( "Can't happen.\n" );
	
	*front = AddSurfaceToBrush( in, plane->flipplane, bsp_enable );
	*back = AddSurfaceToBrush( in, plane, bsp_enable );

	if ( *front )
	{
		if ( CalcBrushVolume( *front ) < 16.0 )
		{
//			FreeBrush( *front );
//			*front = NULL;
//			printf( " small front.\n" );
		}
	}

	if ( *back )
	{
		if ( CalcBrushVolume( *back ) < 16.0 )
		{
//			FreeBrush( *back );
//			*back = NULL;
//			printf( " small back.\n" );
		}
	}

	if ( !*front && *back )
	{
		FreeBrush( *back );
		*back = CopyBrush( in );

	}
	if ( *front && !*back )
	{
		FreeBrush( *front );
		*front = CopyBrush( in );

	}
	if ( !*front && !*back )
	{
		Error( " * no front, no back: in volume %f\n", CalcBrushVolume( in ) );
	}
}




/*
  ========================================
  
  bsp stuff

  ========================================
*/

/*
  ====================
  BSP_CheckSurfacesWithPlane

  ====================
*/
void BSP_CheckSurfacesWithPlane( cbspbrush_t *in, vec3d_t norm, fp_t dist,
				 int *oncnt, int *frontcnt, int *backcnt, int *splitcnt, int *forbidsplitcnt )
{
	int		i;
	int		what;

//	*oncnt = *frontcnt = *backcnt = *splitcnt = 0;

	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( !in->surfaces[i].p )
			continue;

		if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
			continue;

		what = CheckPolygonWithPlane( in->surfaces[i].p, norm, dist );

		if ( what == POLY_SPLIT )
		{
			if ( in->surfaces[i].state & SURFACE_STATE_DONT_SPLIT )
			{
				printf( "forbidsplit " );
				(*forbidsplitcnt)++;
			}
			
			(*splitcnt)++;
			continue;
		}

		if ( what == POLY_ON )
		{
			(*oncnt)++;
			continue;
		}

		if ( what == POLY_FRONT )
		{
			(*frontcnt)++;
			continue;
		}

		if ( what == POLY_BACK )
		{
			(*backcnt)++;
			continue;
		}
	}	
}



/*
  ====================
  BSP_FindSplitPlane

  ====================
*/
cplane_t * BSP_FindSplitPlane( cbspbrush_t *head, vec3d_t min, vec3d_t max )
{
	int		i;
	cbspbrush_t	*b1, *b2;
	cplane_t		*plane;
	cplane_t		*pl;
//	int		what;

	int		splitcnt;
	int		frontcnt;
	int		backcnt;
	int		oncnt;
	int		forbidsplitcnt;
	int		value;

	int		bestvalue;
	cplane_t 	*bestplane;
	int		bestsplitcnt;
	int		bestfrontcnt;
	int		bestbackcnt;
	int		bestforbidsplitcnt;

	int		pass;

	if ( !head )
		return NULL;

	bestvalue = 999999;
	bestplane = NULL;

	//
	// pass 0: axial vis-solid
	// pass 1: non-axial vis-solid
	// pass 2: axial solid
	// pass 3: non-axial solid
	// pass 4: axial deco
	// pass 5: non-axial deco
	// pass 6: axial hull
	// pass 7: non-axial hull
	//
	
	for ( pass = 0; pass < 8; pass++ )
	{

		// no axial test 
//		if ( !(pass&1) )
//			continue;

		// for every brush
		for ( b1 = head; b1 ; b1=b1->next )
		{		      


#if 0
			// hack: only allow solids
			if ( b1->contents == BRUSH_CONTENTS_HINT ||
			     b1->contents == BRUSH_CONTENTS_LIQUID ||
			     b1->contents == BRUSH_CONTENTS_DECO )
				continue;
			

			if ( b1->contents == BRUSH_CONTENTS_DECO )
				continue;
#endif

			//
			// brush relevant for this brush
			//
			if ( (pass&~1) == 0 )
			{
				// solid may split
				if ( b1->contents != BRUSH_CONTENTS_SOLID )
					continue;
			}
			else if ( (pass&~1) == 2 )
			{
				// liquid may split
				if ( b1->contents != BRUSH_CONTENTS_LIQUID )
					continue;
			}
			else if ( (pass&~1) == 4 )
			{
				// deco may split
				if ( b1->contents != BRUSH_CONTENTS_DECO )
					continue;
			}
			else if ( (pass&~1) == 6 )
			{
				if ( b1->contents != BRUSH_CONTENTS_HULL )
					continue;
			}
			

			// for every surface plane
			for ( i = 0; i < b1->surfacenum; i++ )
			{
				
				if ( b1->surfaces[i].state & SURFACE_STATE_ONNODE )
					continue;

				// use only the front facing planes
			       
//				plane = b1->surfaces[i].plane&~1;

				// should the surface not be used for bsp ?
				if ( b1->surfaces[i].state & SURFACE_STATE_IGNORE )
					continue;

				plane = b1->surfaces[i].pl;

				if (!( plane->type & PLANE_POS) )
				{
					plane = plane->flipplane;
				}

				if ( !(pass&1) )
				{
					// axial check
					if ( (plane->type&PLANE_AXIS_MASK) > PLANE_Z )
						continue; // non-axial
				}
				
				if ( plane->count == bsp_counter )
				// this plane was still tested 
					continue;
				plane->count = bsp_counter;
				
				
				pl = plane;
				splitcnt = frontcnt = backcnt = oncnt = forbidsplitcnt = 0;
				
#if defined(BSP_MID_SPLIT_PASS0)
				if ( pass == 0 )
				{					
					// axial vis-solid pass, plane should mid split 
					
					int		l, j;
					fp_t		d;

					value = 0;

					l = pl->type&PLANE_AXIS_MASK;
					if ( l > PLANE_Z )
						continue;	// can't happen

					d = pl->dist*pl->norm[l];
					for ( j = 0; j < 3; j++ )
					{
						if ( j == l )
						{
							value += ( max[l]-d ) * ( max[l]-d );
							value += ( d-min[l] ) * ( d-min[l] );
						}
						else
						{
							value += 2 * ( max[j]-min[j] ) * ( max[j]-min[j] );
						}
					}

					if ( value < bestvalue )
					{
						bestvalue = value;
						bestplane = plane;
					}
				}
				else
#endif
				{
				// check all brushes
					for ( b2 = head; b2 ; b2=b2->next )
					{
						BSP_CheckSurfacesWithPlane( b2, pl->norm, pl->dist,
									    &oncnt, &frontcnt, &backcnt, &splitcnt, &forbidsplitcnt );
						
					}
					
					value = 5*splitcnt + abs( frontcnt-backcnt );
					value += forbidsplitcnt * 10000;	// try to avoid forbidden splits
				      

					if ( value < bestvalue )
					{
						bestvalue = value;
						bestplane = plane;
						bestforbidsplitcnt = forbidsplitcnt;
					}
				}
				
			}
		}

		// init planes for next pass
		bsp_counter++;

		// was a plane found in this pass
		if ( bestplane != NULL )
			break;
	}


//	printf( "plane %d: splitcnt = %d, frontcnt = %d, backcnt = %d, value = %d\n", 
//		bestplane, bestsplitcnt, bestfrontcnt, bestbackcnt, bestvalue );

	if ( bestplane )
	{
		if ( bestforbidsplitcnt > 0 )
		{
			printf( "WARNING: can't avoid forbidden split\n" );
		}
	}

	return bestplane;
}



/*
  ====================
  BSP_SplitBrushList

  ====================
*/
void MarkOnNode( cbspbrush_t *in, cplane_t *plane )
{
	int		i;

	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( in->surfaces[i].pl == plane ||
		     in->surfaces[i].pl == plane->flipplane )
		{
			in->surfaces[i].state |= SURFACE_STATE_ONNODE;
		}
	}
}

void BSP_SplitBrushList( cbspbrush_t *in, cplane_t *plane, cbspbrush_t **frontb, cbspbrush_t **backb )
{
	cbspbrush_t	*b, *bnext;
	cbspbrush_t	*front, *back;
	cbspbrush_t	*frontlist, *backlist;

	frontlist = backlist = NULL;

	for ( b = in; b ; b=bnext )
	{
		bnext = b->next;
//		CSG_SplitBrush( b, plane, &front, &back, true );
		CSG_SplitBrush_new( b, plane, &front, &back );

		if ( front )
		{
//			front->partof = b;
			MarkOnNode( front, plane );
			front->next = frontlist;
			frontlist = front;
		}
		if ( back )
		{
//			back->partof = b;
			MarkOnNode( back, plane );
			back->next = backlist;
			backlist = back;
		}

		FreeBrush( b );

		continue;
	}

	*frontb = frontlist;
	*backb = backlist;
}



/*
  ====================
  BrushListInfo

  needed for determine the type of leaf:
  1) brushnum = 0: empty_leaf
  2) brushnum != 0 && surfacenum == onnodenum: brush_leaf
  3) brushnum != 0 && surfacenum =! 0 && onnodenum == 0: empty_leaf with contents ( deco brushes )
  ====================
*/
void BrushListInfo( cbspbrush_t *list, int *surfacenum, int *onnodenum, int *brushnum )
{
	int		i;
	
	*surfacenum = *onnodenum = *brushnum = 0;

	if ( !list )
		return;
	
	for( ; list; list=list->next, (*brushnum)++ )
		for ( i = 0; i < list->surfacenum; i++, (*surfacenum)++ )
			if ( list->surfaces[i].state & SURFACE_STATE_ONNODE )
				(*onnodenum)++;
}



/*
  ====================
  BSP_MakeTreeRecursive

  ====================
*/
static int	stat_clusternum = 1;	// 0 = node is not part of any cluster

static int brush_contents_fixed_num = 0;
 
void BSP_MakeTreeRecursive( cbspnode_t *node, cbspbrush_t *list, int cluster )
{
	cplane_t	*splitplane;

	cbspbrush_t	*frontlist, *backlist;

	int		num;
	int		cluster1, cluster2;

	int		surfacenum, onnodenum, brushnum;


	splitplane = BSP_FindSplitPlane( list, node->min, node->max );
//	printf( "split: %p\n", split );

	if ( splitplane == NULL )
	{
		//
		// it's a leaf
		//
		
		BrushListInfo( list, &surfacenum, &onnodenum, &brushnum );

#if 0
		if ( brushnum == 0 )
		{
			// it's a real empty leaf

			node->type = NodeType_emptyleaf;
			stat_emptynum++;

			return;			
		}
		else if ( brushnum > 0 && surfacenum == onnodenum )
		{
			// it's one solid brush leaf

			node->type = NodeType_solidleaf;
			node->solid = list;
			stat_solidnum++;

			return;
		}
		else if ( brushnum > 0 && surfacenum != onnodenum )
		{
			// it's a empty leaf with un-bsp-ed contents

#if 1
			node->type = NodeType_contentsleaf;
			node->type = NodeType_solidleaf;
			node->solid = list;
			stat_empty2num++;
			stat_inempty2num+=brushnum;

			printf( "obsolete !\n" );

#else
			//hack, ignore un-bsp-ed contents
			node->type = NodeType_emptyleaf;
			
			stat_emptynum++;
#endif
			return;
		}

#endif

		// only real empty leafs or
		// solid leafs with at least one brush
		if ( brushnum == 0 )
		{
			node->type = NodeType_emptyleaf;
			stat_emptynum++;
			node->solid = NULL;
			node->contents = 0;
			return;		
		}
		else
		{
			unsigned int	max_contents;
			cbspbrush_t	*b;

			node->type = NodeType_emptyleaf;
			stat_solidnum++;
			node->solid = list;

			//
			// needs a contents fix ?
			//
			max_contents = 0;
			for ( b = list ; b ; b=b->next )
			{
//				printf( "%d ", b->contents );
				if ( b->contents > max_contents )
					max_contents = b->contents;
			}
//			printf( "\n" );
			
			for ( b = list ; b ; b=b->next )
			{
				if ( b->contents < max_contents )
				{
//					Error( "mulit contents leaf !\n" );

					b->contents = max_contents;
					brush_contents_fixed_num++;
				}
			}
			
			node->contents = max_contents;

			return;
		}

		Error( "can't determine leaf type: brushnum %d, surfacenum %d, onnodenum %d.\n",
		       brushnum, surfacenum, onnodenum );
	}

	node->plane = splitplane;	

	BSP_SplitBrushList( list, splitplane, &frontlist, &backlist );

	node->child[0] = NewNode();
	node->child[1] = NewNode();
	
	cluster1 = cluster2 = cluster;
	if ( cluster == 0 )
	{
		num = BrushListLength( frontlist );
		if ( num <= 64 )
		{
			cluster1 = stat_clusternum++;
//			printf( "new cluster %d for %d brushes.\n", cluster1, num );			
		}
	}
	if ( cluster == 0 )
	{
		num = BrushListLength( backlist );
		if ( num <= 64 )
		{
			cluster2 = stat_clusternum++;
//			printf( "new cluster %d for %d brushes.\n", cluster2, num );
		}
	}
	

	//
	// split node volume brush and add frags to children
	//
#if 0


	if ( CalcBrushVolume( node->child[0]->volume ) < 16.0 )
		printf( "small child[0]. volume %f\n", CalcBrushVolume( node->child[0]->volume ) );
	if ( CalcBrushVolume( node->child[1]->volume ) < 16.0 )
		printf( "small child[1]. volume %f\n", CalcBrushVolume( node->child[1]->volume ) );
#endif
	if ( node->volume )
	{
		CSG_SplitBrush_new( node->volume, splitplane, &node->child[0]->volume, &node->child[1]->volume );
		FreeBrush( node->volume );
		node->volume = NULL;
	}
	CalcBrushListBounds( frontlist, node->child[0]->min, node->child[0]->max );
	CalcBrushListBounds( backlist, node->child[1]->min, node->child[1]->max );

	BSP_MakeTreeRecursive( node->child[0], frontlist, cluster1 );
	BSP_MakeTreeRecursive( node->child[1], backlist, cluster2 );
}

/*
  ==================================================
  class stuff

  ==================================================
*/

/*
  ====================
  ReadPlaneClass

  ====================
*/
hmanager_t * ReadPlaneClass( char *name )
{
	tokenstream_t	*ts;
	hobj_t		*planecls;
	hmanager_t	*hm;
	hobj_search_iterator_t	iter;
	hobj_t		*plane;
	hobj_t		*flipplane;
	cplane_t		*pl;
	int		num;
	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	//
	// create compiled planes
	//

	fprintf( stderr, "load plane class and compile ...\n" );

	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		pl = NewCPlane();
		pl->count = 0;

		// plane norm
		pair = FindHPair( plane, "norm" );
		if ( !pair )
			Error( "missing plane normal.\n" );
		HPairCastToVec3d_safe( pl->norm, pair );

		// plane dist
		pair = FindHPair( plane, "dist" );
		if ( !pair )
			Error( "missing plane distance.\n" );
		HPairCastToFloat_safe( &pl->dist, pair );
		
		// plane type
		pair = FindHPair( plane, "type" );
		if ( !pair )
			Error( "missing plane type.\n" );
		HPairCastToInt_safe( &pl->type, pair );

		pl->self = plane;
		SetClassExtra( plane, pl );
		
	}

	//
	// resolve clsref_flipplane
	//
	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		// plane flipplane clsref
		pair = FindHPair( plane, "flipplane" );
		if ( !pair )
			Error( "missinig clsref flipplane" );

		flipplane = HManagerSearchClassName( hm, pair->value );
		if ( !flipplane )
			Error( "can't resolve clsref flipplane.\n" );

		pl = GetClassExtra( plane );
		pl->flipplane = GetClassExtra( flipplane );
	}

	printf( " %d planes\n", num );

	return hm;	
}

/*
  ====================
  ReadTexdefClass

  ====================
*/

hmanager_t * ReadTexdefClass( char *name )
{
	tokenstream_t		*ts;
	hobj_t			*texdefcls;
	hmanager_t		*hm;
	hobj_search_iterator_t	iter;
	hobj_t			*texdef;
	ctexdef_t		*td;
	int			num;

	fprintf( stderr, "load texdef class and compile ...\n" );

	ts = BeginTokenStream( name );
	texdefcls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, texdefcls );
	HManagerRebuildHash( hm );

	InitClassSearchIterator( &iter, texdefcls, "texdef" );

	for ( num = 0; ( texdef = SearchGetNextClass( &iter ) ); num++ )
	{
		td = NewCTexdef();

		td->self = texdef;
		SetClassExtra( texdef, td );
	}

	printf( " %d texdefs.\n", num );

	return hm;
}

/*
  ====================
  ReadBspbrushClass

  ====================
*/
hmanager_t * ReadBspbrushClass( char *name, hmanager_t *planecls, hmanager_t *texdefcls )
{
	tokenstream_t		*ts;
	hobj_t			*bspbrushcls;
	hmanager_t		*hm;
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	surfiter;
	hpair_t			*pair;
	hobj_t			*bspbrush;
	hobj_t			*surface;
	hobj_t			*plane;
	hobj_t			*texdef;
	int		num, i;
	cbspbrush_t		*bb;

	fprintf( stderr, "load and compile bspbrush class ...\n" );

	ts = BeginTokenStream( name );
	bspbrushcls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, bspbrushcls );
	HManagerRebuildHash( hm );

	//
	// create compiled brushes
	// 

	InitClassSearchIterator( &iter, bspbrushcls, "bspbrush" );

	for ( num = 0; ( bspbrush = SearchGetNextClass( &iter ) ); num++ )
	{
	
		//
		// count surfaces
		//
		InitClassSearchIterator( &surfiter, bspbrush, "surface" );
		for ( i = 0; ( surface = SearchGetNextClass( &surfiter ) ); i++ )
		{ }

		//
		// create bspbrush
		//
		bb = NewBrush( i );
		bb->surfacenum = i;

		pair = FindHPair( bspbrush, "content" );
		if ( !pair )
			Error( "missing content.\n" );
		HPairCastToInt_safe( &bb->contents, pair );

		InitClassSearchIterator( &surfiter, bspbrush, "surface" );
		for ( i = 0; ( surface = SearchGetNextClass( &surfiter ) ); i++ )
		{ 
			// get content
			pair = FindHPair( surface, "content" );
			if ( !pair )
				Error( "missing content.\n" );
			HPairCastToInt( &bb->surfaces[i].contents, pair );

			// get clsref_plane
			pair = FindHPair( surface, "plane" );
			if ( !pair )
				Error( "missing clsref plane.\n" );
  			plane = HManagerSearchClassName( planecls, pair->value );
			bb->surfaces[i].pl = GetClassExtra( plane );

			// get clsref_texdef if available
			pair = FindHPair( surface, "texdef" );
			if ( !pair || !texdefcls )
			{
				bb->surfaces[i].td = NULL;
			}
			else
			{
				texdef = HManagerSearchClassName( texdefcls, pair->value );
				bb->surfaces[i].td = GetClassExtra( texdef );
			}

			bb->surfaces[i].state = 0;

#if 0
			// get surface state
			pair = FindHPair( surface, "bsp_onnode" );
			if ( pair )
			{
				// surface allready on a bsp node
				bb->surfaces[i].state |= SURFACE_STATE_ONNODE;
			}
#endif
			pair = FindHPair( surface, "bsp_ignore" );
			if ( pair )
			{
				// surface should be ignored for split plane search
				bb->surfaces[i].state |= SURFACE_STATE_IGNORE;
			}

			pair = FindHPair( surface, "bsp_dont_split" );
			if ( pair )
			{
				bb->surfaces[i].state |= SURFACE_STATE_DONT_SPLIT;
			}


		}
		bb->original = bspbrush;	   
		SetClassExtra( bspbrush, bb );
	}

	printf( " %d bspbrushes.\n", num );

	return hm;
}

/*
  ====================
  BuildCBspbrushList

  ====================
*/
cbspbrush_t * BuildCBspbrushList( hmanager_t *hm )
{
	hobj_t		*bspbrushcls;
	hobj_search_iterator_t	iter;
	hobj_t		*bspbrush;
	cbspbrush_t	*b;
	cbspbrush_t	*list;
	int		num;

	bspbrushcls = HManagerGetRootClass( hm );

	InitClassSearchIterator( &iter, bspbrushcls, "bspbrush" );
	list = NULL;
	for ( num = 0; ( bspbrush = SearchGetNextClass( &iter ) ); num++ )
	{
		b = GetClassExtra( bspbrush );
		b->next = list;
		list = b;
	}

	return list;
}


/*
  ====================
  WriteBspnodeClass

  ====================
*/
hobj_t * BuildBspbrushClass( cbspbrush_t *b )
{
	hobj_t		*self;
	int		i;
	hpair_t		*pair;
	hobj_t		*surface;
	char		tt[256];

	sprintf( tt, "#%u", HManagerGetFreeID() );
	self = NewClass( "bspbrush", tt );

	sprintf( tt, "%u", b->contents );
	pair = NewHPair2( "int", "content", tt );
	InsertHPair( self, pair );

	pair = NewHPair2( "ref", "original", b->original->name );
	InsertHPair( self, pair );

	for ( i = 0; i < b->surfacenum; i++ )
	{
		sprintf( tt, "#%u", HManagerGetFreeID() );
		surface = NewClass( "surface", tt );
		
		// clsref_plane
		sprintf( tt, "%s", b->surfaces[i].pl->self->name );
		pair = NewHPair2( "ref", "plane", tt );
		InsertHPair( surface, pair );
		
		// clsref_texdef
		if ( b->surfaces[i].td )
		{
			sprintf( tt, "%s", b->surfaces[i].td->self->name );
			pair = NewHPair2( "ref", "texdef", tt );
			InsertHPair( surface, pair );
		}
		
		// content
		sprintf( tt, "%u", b->surfaces[i].contents );
		pair = NewHPair2( "int", "content", tt );
		InsertHPair( surface, pair );

#if 0
		// state
		if ( b->surface[i].state & SURFACE_STATE_ONNODE )
		{
			pair = NewHPair2( "" );
		}
#endif

		if ( b->surfaces[i].state & SURFACE_STATE_DONT_SPLIT )
		{
			pair = NewHPair2( "int", "bsp_dont_split", "1" );
			InsertHPair( surface, pair );
		}

		InsertClass( self, surface );
	}
	
	return self;
}

void BuildBspnodeClassRecursive( cbspnode_t *node, hobj_t *self, hobj_t *brushcls )
{
	hobj_t		*nodecls;
	hpair_t		*pair;
	hobj_t		*brush;
	cbspbrush_t	*b;
	char		tt[256];

//	sprintf( tt, "#%u", HManagerGetFreeID() );
//	nodecls = NewClass( "bspnode", tt );

	// type of brush, the plane is from

	if ( node->type == NodeType_node )
	{
		//
		// node
		//

		// clsref plane
		sprintf( tt, "%s", node->plane->self->name );
		pair = NewHPair2( "ref", "plane", tt );
		InsertHPair( self, pair );


		// front node
		sprintf( tt, "#%u", HManagerGetFreeID() );
		nodecls = NewClass( "bspnode_front", tt );
		BuildBspnodeClassRecursive( node->child[0], nodecls, brushcls );
		InsertClass( self, nodecls );

		// back node
		sprintf( tt, "#%u", HManagerGetFreeID() );
		nodecls = NewClass( "bspnode_back", tt );		
		BuildBspnodeClassRecursive( node->child[1], nodecls, brushcls );
		InsertClass( self, nodecls );
	}
	else
	{
		//
		// leaf
		//


		sprintf( tt, "%d", node->contents );
		pair = NewHPair2( "int", "contents", tt );
		InsertHPair( self, pair );		

		for ( b = node->solid; b ; b=b->next )
		{
			brush = BuildBspbrushClass( b );
			InsertClass( brushcls, brush );

			pair = NewHPair2( "ref", "brush", brush->name );
			InsertHPair( self, pair );
		}

		{
			fp_t	volume;
			if ( node->volume )
			{
				volume = CalcBrushVolume( node->volume );
				sprintf( tt, "%.2e", volume );
				pair = NewHPair2( "float", "volume", tt );
				InsertHPair( self, pair );
			}
		}
		
	}
	

//	InsertClass( parent, nodecls );
	return;

}

void BuildBspnodeClass( cbspnode_t *topnode, char *node_name, char *brush_name )
{
	hobj_t		*nodecls;
	hobj_t		*brushcls;
	char		tt[256];
	FILE		*h;

	nodecls = NewClass( "bspnodes", "topnode" );
	brushcls = NewClass( "bspbrushes", "bspbrushes" );

	printf( "build output bspnode and bspbrush class ...\n" );
	BuildBspnodeClassRecursive( topnode, nodecls, brushcls );

	DeepDumpClass( nodecls );
	DeepDumpClass( brushcls );

	printf( "writing classes ...\n" );

	h = fopen( node_name, "w" );
	WriteClass( nodecls, h );
	fclose( h );

	h = fopen( brush_name, "w" );
	WriteClass( brushcls, h );
	fclose( h );
}

/*
  ====================
  FixBrushSurfaces

  ====================
*/
int	fixed_brushes = 0;
int	fixed_surfaces = 0;
void FixBrushSurfaces( cbspbrush_t *b )
{
	int		i, j;
	int	surfnum;

	surfnum = b->surfacenum;
restart:
	for ( i = 0; i < b->surfacenum; i++ )
	{
		if ( !b->surfaces[i].p )
		{
			for ( j = b->surfacenum-1; j > i; j++ )
			{
				if ( b->surfaces[j].p )
				{
					memcpy( &b->surfaces[i], &b->surfaces[j], sizeof( csurface_t ) );
					b->surfacenum--;
					fixed_surfaces++;
					goto restart;
				}
			}
		}
	}

	if ( surfnum > b->surfacenum )
		fixed_brushes++;

	if ( b->surfacenum < 4 )
		printf( "fixed brush surfacenum below 4.\n" );
}

cbspbrush_t * BuildBigBox( char *name, hmanager_t *planehm )
{
	hmanager_t	*bigboxhm;
	bigboxhm = ReadBspbrushClass( name, planehm, NULL );
	if ( !bigboxhm )
		Error( "can't load bigbox class.\n" );
	return BuildCBspbrushList( bigboxhm );	
}

void PrintHelp( void )
{
	puts( "usage:" );
	puts( " -i\t input bspbrush class" );
	puts( " -n\t output bspnode class" );
	puts( " -o\t output bspbrush class" );
	puts( " -pl\t input plane class" );
	puts( " -td\t input texdef class" );
}

int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_node_name;
	char		*out_brush_name;

	char		*plane_name;
	char		*texdef_name;

	hmanager_t	*planecls;
	hmanager_t	*texdefcls;
	hmanager_t	*bspbrushcls;

	cbspbrush_t	*bspbrushlist;
	cbspbrush_t		*b;

	cbspnode_t		*topnode;

	printf( "===== bsp - build a bsp tree from brushes =====\n" );

	if ( argc == 1 )
	{
		PrintHelp();
		exit(-1);
	}

	SetCmdArgs( argc, argv );

	// init planes for quick test
//	memset( p_planecheck, 0, sizeof(int)*MAX_PLANES );
	bsp_counter = 1;
		
	in_brush_name = GetCmdOpt2( "-i" );
	out_node_name = GetCmdOpt2( "-n" );
	out_brush_name = GetCmdOpt2( "-o" );
	texdef_name = GetCmdOpt2( "-pl" );
	plane_name = GetCmdOpt2( "-td" );

	if ( !in_brush_name )
	{
		PrintHelp();
		Error( "no input bspbrush class.\n" );
	}
	else
	{
		printf( " input bspbrush class: %s\n", in_brush_name );
	}

	if ( !out_node_name )
	{
		out_node_name = "_bspout_bspnode.hobj";
		printf( " default output bspnode class: %s\n", out_node_name );
	}
	else
	{
		printf( " output bspnode class: %s\n", out_node_name );
	}

	if ( !out_brush_name )
	{
		out_brush_name = "_bspout_bspbrush.hobj";
		printf( " default output bspbrush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output bspbrush class: %s\n", out_brush_name );
	}

	if ( !plane_name )
	{
		plane_name = "_plane.hobj";
		printf( " default input plane class: %s\n", plane_name );
	}
	else
	{
		printf( " input plane class: %s\n", plane_name );
	}

	if ( !texdef_name )
	{
		texdef_name = "_texdef.hobj";
		printf( " default input texdef class: %s\n", texdef_name );
	}
	else
	{
		printf( " input texdef class: %s\n", texdef_name );
	}

	planecls = ReadPlaneClass( plane_name );
	texdefcls = ReadTexdefClass( texdef_name );
	bspbrushcls = ReadBspbrushClass( in_brush_name, planecls, texdefcls );
	bspbrushlist = BuildCBspbrushList( bspbrushcls );

	//
	// create polygons for in_list
	// 
	for ( b = bspbrushlist; b ; b=b->next )
		CreateBrushPolygons( b );
	//
	// fix surfaces or remove brush
	//
	{
	    cbspbrush_t		*next, *head;
	    
	    head = NULL;
	    for ( b = bspbrushlist; b ; b=next )
	    {
		next=b->next;
	        FixBrushSurfaces( b );
		if ( b->surfacenum < 4 )
		{
		    printf( "remove brush '%s'.\n", b->original->name );
		    continue;
		}
		b->next = head;
		head = b;
	    }
	    bspbrushlist = head;
	}
	printf( " %d brushes need a surface fix, %d surfaces removed.\n", fixed_brushes, fixed_surfaces );

	// p_nodes[0] is headnode
	p_nodenum = 1;

	printf( "build bsp tree ...\n" );
	topnode = NewNode();

	topnode->volume = BuildBigBox( "_bigbox.hobj", planecls );
	CreateBrushPolygons( topnode->volume );
	CalcBrushListBounds( bspbrushlist, topnode->min, topnode->max );
	BSP_MakeTreeRecursive( topnode, bspbrushlist, 0 );

//	printf( " %d output brushes\n", p_brushnum );
	printf( " %d nodes: %d solid, %d empty\n", p_nodenum, stat_solidnum, stat_emptynum );
	printf( " %d leafs with %d brushes\n", stat_empty2num, stat_inempty2num );

	printf( " %d brushes need a contents-fix\n", brush_contents_fixed_num );
	
	BuildBspnodeClass( topnode, out_node_name, out_brush_name );
//	Write_BrushArray( p_brushes, p_brushnum, out_brush_name, "bsp" );
//	Write_NodeArray( p_nodes, p_nodenum, out_node_name, "bsp" );

	HManagerSaveID();

	exit(0);
}
