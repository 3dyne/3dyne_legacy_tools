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

#include "bsp.h"

int		p_planenum;
plane_t		p_planes[MAX_PLANES];

int		p_planecheck[MAX_PLANES]; // BSP_FindSplitPlane needs this for quick test

int		p_nodenum = 0;
bspnode_t	p_nodes[MAX_NODES];

int		p_brushnum = 0;
bspbrush_t	*p_brushes[MAX_BRUSHES];


static int		stat_solidnum = 0;	// brush leaf
static int		stat_emptynum = 0;	// real empty
static int		stat_empty2num = 0;	// empty leafs with contents
static int		stat_inempty2num = 0;	// number of brushes in 'empty leafs with contents'
// for p_plane quick test in BSP_FindSplitPlane
static int		bsp_counter;

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
bool_t IsBrushExactOnPlane( bspbrush_t *in, int plane, bool_t bsp_enable )
{
	int		i;
	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( (in->surfaces[i].plane&~1) == (plane&~1) )
		{
			// it's the same plane
			if ( bsp_enable )
			{
				if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
				{
					printf( " * IsBrushExactOnPlane : surface allready on node. *\n" );
					printf( " * brush id: %d *\n", in->original );
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
int CheckBrushWithPlane2( bspbrush_t *in, vec3d_t norm, fp_t dist, bool_t bsp_enable )
{
	int		i;
	bool_t		front, back, on;
	int		what;
	
	back = front = on = false;

	for ( i = 0; i < in->surfacenum; i++ )
	{
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
bspbrush_t* AddSurfaceToBrush( bspbrush_t *in, int plane, bool_t bsp_enable )
{
	int		i, j;
	int		surfnum;
	surface_t	surfs[MAX_SURFACES_PER_BRUSH];
	plane_t		*pl;
	bspbrush_t	*bnew;

	// todo: contents
	
	// copy the original surfaces local
	for ( i = 0; i < in->surfacenum; i++ )
	{
		memcpy( &surfs[i], &in->surfaces[i], sizeof( surface_t ) );
		surfs[i].p = NULL; // don't touch the original polys
	}
	surfnum = in->surfacenum;
		

	surfs[surfnum].plane = plane;
	surfs[surfnum].state = SURFACE_STATE_BYSPLIT; //bysplit = true;
	surfs[surfnum].contents = 0;	// or should it be set to NOT_VISIBLE
	if ( bsp_enable )
		surfs[surfnum].state |= SURFACE_STATE_ONNODE;
	surfnum++;

	for( i = 0; i < surfnum; i++ )
	{
		pl = &p_planes[surfs[i].plane];
		surfs[i].p = BasePolygonForPlane( pl->norm, pl->dist );

		for ( j = 0; j < surfnum; j++ )
		{
			if ( i == j )
				continue;

			if ( surfs[i].p )
			{
				pl = &p_planes[surfs[j].plane];
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
			memcpy( &bnew->surfaces[j], &surfs[i], sizeof( surface_t ) );
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
		printf( " brush id: %d\n", in->original );
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
void CSG_SplitBrush( bspbrush_t *in, int plane, bspbrush_t **front, bspbrush_t **back, bool_t bsp_enable )
{
	int		what;
	plane_t		*pl;
	plane_t		*plflip;
	bool_t		on;

	fp_t		v1, v2;
	polygon_t	*poly; // for debug

	on = false;
	*front = *back = NULL;

	if ( !in )
		return;

	pl = &p_planes[plane];
	plflip = &p_planes[plane^1];

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
	
	*front = AddSurfaceToBrush( in, plane^1, bsp_enable );
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
void BSP_CheckSurfacesWithPlane( bspbrush_t *in, vec3d_t norm, fp_t dist,
				 int *oncnt, int *frontcnt, int *backcnt, int *splitcnt )
{
	int		i;
	int		what;

//	*oncnt = *frontcnt = *backcnt = *splitcnt = 0;

	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
			continue;

		what = CheckPolygonWithPlane( in->surfaces[i].p, norm, dist );

		if ( what == POLY_SPLIT )
		{
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
int BSP_FindSplitPlane( bspbrush_t *head )
{
	int		i;
	bspbrush_t	*b1, *b2;
	int		plane;
	plane_t		*pl;
//	int		what;

	int		splitcnt;
	int		frontcnt;
	int		backcnt;
	int		oncnt;
	int		value;

	int		bestvalue;
	int		bestplane;
	int		bestsplitcnt;
	int		bestfrontcnt;
	int		bestbackcnt;

	int		pass;

	if ( !head )
		return -1;

	bestvalue = 999999;
	bestplane = -1;

	//
	// pass 0: axial solid and liquid may split 
	// pass 1: non-axial solid and liquid may split
	// pass 2: axial deco may split
	// pass 3: non-axial deco may split
	// pass 4: axial hint may split
	// pass 5: non-axial hint may split
	
	//
	
	for ( pass = 0; pass < 6; pass++ )
	{

		// no axial test 
//		if ( !(pass&1) )
//			continue;

		// for every brush
		for ( b1 = head; b1 ; b1=b1->next )
		{
			
			//
			// brush relevant for this brush
			//
			if ( (pass&~1) == 0 )
			{
				// solid and liquid may split
				if ( b1->contents != BRUSH_CONTENTS_SOLID &&
				     b1->contents != BRUSH_CONTENTS_LIQUID )
					continue;
			}
			else if ( (pass&~1) == 2 )
			{
				// deco may split

				// test: deco never split
//				if ( b1->contents != BRUSH_CONTENTS_DECO )
					continue;
			}
			else if ( (pass&~1) == 4 )
			{
				// hint may split
				if ( b1->contents != BRUSH_CONTENTS_HINT )
					continue;
			}
			
			// for every surface plane
			for ( i = 0; i < b1->surfacenum; i++ )
			{
				
				if ( b1->surfaces[i].state & SURFACE_STATE_ONNODE )
					continue;

				// use only the front facing planes
				plane = b1->surfaces[i].plane&~1;

				if ( !(pass&1) )
				{
					// axial check
					if (p_planes[plane].type > PLANE_Z )
						continue; // non-axial
				}
				
				if ( p_planecheck[plane] == bsp_counter )
				// this plane was still tested 
					continue;
				p_planecheck[plane] = bsp_counter;
				
				
				pl = &p_planes[plane];
				splitcnt = frontcnt = backcnt = oncnt = 0;
				
				// check all brushes
				for ( b2 = head; b2 ; b2=b2->next )
				{
					BSP_CheckSurfacesWithPlane( b2, pl->norm, pl->dist,
								    &oncnt, &frontcnt, &backcnt, &splitcnt );
					
//				what = CheckBrushWithPlane2( b2, pl->norm, pl->dist );
				}
				
				value = 5*splitcnt + abs( frontcnt-backcnt );
//			value = 1*splitcnt + abs( frontcnt-backcnt );
				
//			printf( "plane %d: splitcnt = %d, frontcnt = %d, backcnt = %d, oncnt = %d, value = %d\n", 
//				plane, splitcnt, frontcnt, backcnt, oncnt, value );
				
				if ( value < bestvalue )
				{
					bestvalue = value;
					bestfrontcnt = frontcnt;
					bestbackcnt = backcnt;
					bestsplitcnt = splitcnt;
					bestplane = plane;
				}
				
			}
		}

		// init planes for next pass
		bsp_counter++;

		// was a plane found in this pass
		if ( bestplane != -1 )
			break;
	}


//	printf( "plane %d: splitcnt = %d, frontcnt = %d, backcnt = %d, value = %d\n", 
//		bestplane, bestsplitcnt, bestfrontcnt, bestbackcnt, bestvalue );

	return bestplane;
}



/*
  ====================
  BSP_SplitBrushList

  ====================
*/
void BSP_SplitBrushList( bspbrush_t *in, int plane, bspbrush_t **frontb, bspbrush_t **backb )
{
	bspbrush_t	*b;
	bspbrush_t	*front, *back;
	bspbrush_t	*frontlist, *backlist;

	frontlist = backlist = NULL;

	for ( b = in; b ; b=b->next )
	{
		CSG_SplitBrush( b, plane, &front, &back, true );
		if ( front )
		{
//			front->partof = b;
			front->next = frontlist;
			frontlist = front;
		}
		if ( back )
		{
//			back->partof = b;
			back->next = backlist;
			backlist = back;
		}
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
void BrushListInfo( bspbrush_t *list, int *surfacenum, int *onnodenum, int *brushnum )
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
 
void BSP_MakeTreeRecursive( int n, bspbrush_t *list, int cluster )
{
	bspnode_t	*node;
	int		split;
	bspbrush_t	*frontlist, *backlist;

	int		num;
	int		cluster1, cluster2;

	int		surfacenum, onnodenum, brushnum;

//-	node->cluster = cluster;

	node = &p_nodes[n];

	split = BSP_FindSplitPlane( list );

	if ( split == -1 )
	{
		//
		// it's not a node
		//
		
		BrushListInfo( list, &surfacenum, &onnodenum, &brushnum );

		if ( brushnum == 0 )
		{
			// it's a real empty leaf

			node->plane = NODE_PLANE_LEAF_EMPTY;
			stat_emptynum++;

			node->firstbrush = -1;
			node->brushnum = 0;

			return;			
		}
		else if ( brushnum > 0 && surfacenum == onnodenum )
		{
			// it's one solid brush leaf
			node->plane = NODE_PLANE_LEAF_BRUSH;
			stat_solidnum++;

			if ( p_brushnum == MAX_BRUSHES )
				Error( "reached MAX_BRUSHES.\n" );

			node->firstbrush = p_brushnum;
			node->brushnum = 1;			
			p_brushes[p_brushnum] = list;

			p_brushnum++;
			
			return;
		}
		else if ( brushnum > 0 && surfacenum != onnodenum )
		{
			// it's a empty leaf with un-bsp-ed contents
			node->plane = NODE_PLANE_LEAF_EMPTY; // _BRUSH ?
			stat_empty2num++;
			stat_inempty2num+=brushnum;

			node->firstbrush = p_brushnum;
			for ( ; list ; list=list->next )
				p_brushes[p_brushnum++] = list;
			node->brushnum = p_brushnum - node->firstbrush;
			
			return;
		}

		Error( "can't determine leaf type: brushnum %d, surfacenum %d, onnodenum %d.\n",
		       brushnum, surfacenum, onnodenum );
	}

	BSP_SplitBrushList( list, split, &frontlist, &backlist );

	node->plane = split;

	if ( p_nodenum+2 >= MAX_NODES )
		Error( "reached MAX_NODES.\n" );
	node->child[0] = p_nodenum++;
	node->child[1] = p_nodenum++;
	
	cluster1 = cluster2 = cluster;
	if ( cluster == 0 )
	{
		num = BrushListLength( frontlist );
		if ( num <= 64 )
		{
			cluster1 = stat_clusternum++;
			printf( "new cluster %d for %d brushes.\n", cluster1, num );
		}
	}
	if ( cluster == 0 )
	{
		num = BrushListLength( backlist );
		if ( num <= 64 )
		{
			cluster2 = stat_clusternum++;
			printf( "new cluster %d for %d brushes.\n", cluster2, num );
		}
	}
	

	//
	// split node volume brush and add frags to children
	//
#if 0
	CSG_SplitBrush( node->volume, split, &node->child[0]->volume, &node->child[1]->volume, false );

	if ( CalcBrushVolume( node->child[0]->volume ) < 16.0 )
		printf( "small child[0]. volume %f\n", CalcBrushVolume( node->child[0]->volume ) );
	if ( CalcBrushVolume( node->child[1]->volume ) < 16.0 )
		printf( "small child[1]. volume %f\n", CalcBrushVolume( node->child[1]->volume ) );
#endif

	BSP_MakeTreeRecursive( node->child[0], frontlist, cluster1 );
	BSP_MakeTreeRecursive( node->child[1], backlist, cluster2 );
}


int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_node_name;
	char		*out_brush_name;

	bspbrush_t	*in_list;
	bspbrush_t		*b;

	printf( "===== bsp - build a bsp tree from brushes =====\n" );

	SetCmdArgs( argc, argv );

	// init planes for quick test
	memset( p_planecheck, 0, sizeof(int)*MAX_PLANES );
	bsp_counter = 1;
		
	in_brush_name = GetCmdOpt2( "-i" );
	out_node_name = GetCmdOpt2( "-n" );
	out_brush_name = GetCmdOpt2( "-o" );
	
	if ( !in_brush_name )
		Error( "no input brush file.\n" );
	printf( " input brush file: %s\n", in_brush_name );

	if ( !out_node_name )
		Error( "no output node file.\n" );
	if ( !out_brush_name )
		Error( "no output brush file.\n" );
		
	printf( " output node file: %s\n", out_node_name );
	printf( " output brush file: %s\n", out_brush_name );

	p_planenum = MAX_PLANES;
	Read_Planes( "planes.asc", p_planes, &p_planenum );
	printf( " %d planes\n", p_planenum );

	in_list = Read_BrushList( in_brush_name );
	printf( " %d input brushes\n", BrushListLength( in_list ) );

	//
	// create polygons for in_list
	// 
	for ( b = in_list; b ; b=b->next )
		CreateBrushPolygons( b );

	// p_nodes[0] is headnode
	p_nodenum = 1;
	BSP_MakeTreeRecursive( 0, in_list, 0 );
	
	printf( " %d output brushes\n", p_brushnum );
	printf( " %d nodes: %d solid, %d empty\n", p_nodenum, stat_solidnum, stat_emptynum );
	printf( " %d leafs with %d brushes\n", stat_empty2num, stat_inempty2num );
	
	printf( " writing files ...\n" );
	Write_BrushArray( p_brushes, p_brushnum, out_brush_name, "bsp" );
	Write_NodeArray( p_nodes, p_nodenum, out_node_name, "bsp" );

}
