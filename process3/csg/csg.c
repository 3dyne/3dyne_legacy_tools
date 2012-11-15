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



// csg.c

#include "csg.h"

int		p_planenum;
plane_t		p_planes[MAX_PLANES];


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

  common brush operations

  ========================================
*/

/*
  ====================
  CSG_DoBrushesIntersect

  returns false if the brushes definitly not intersect
  ====================
*/
bool_t CSG_DoBrushesIntersect( bspbrush_t *b1, bspbrush_t *b2 )
{
	int		i;
	int			j;

	//
	// check bounding box
	//

	for ( i = 0; i < 3; i++ )
		if ( b1->min[i] > b2->max[i] ||
		     b1->max[i] < b2->min[i] )
			break;

	if ( i!=3 )
		return false;

	for ( i = 0; i < b1->surfacenum; i++ )
	{
		for ( j = 0; j < b2->surfacenum; j++ )
		{
			if ( b1->surfaces[i].plane ==
			     (b2->surfaces[j].plane^1) )
				return false; // brushes just touch
		}
	}

	return true;
}



/*
  ====================
  CSG_IsSubstractAllowed

  returns true if b1 - b2 is allowed
  it's brush contents dependent
  ====================
*/
bool_t CSG_IsSubstractAllowed( bspbrush_t *b1, bspbrush_t *b2 )
{
	unsigned int		c1, c2;

	c1 = b1->contents;
	c2 = b2->contents;

	// solid can always
	if ( c2 == BRUSH_CONTENTS_SOLID )
		return true;

	// if c2 is stronger or equal c1 it can
	if ( c2 >= c1 )
		return true;

	return false;
}

/*
  ====================
  CSG_SubtractBrush

  returns a list of new brushes
  list = b1 - b2
  ====================
*/
bspbrush_t* CSG_SubstractBrush( bspbrush_t *b1, bspbrush_t *b2 )
{
	int		i;
	bspbrush_t	*in, *out;
	bspbrush_t	*front, *back;

	in = b1;
	out = NULL;

	for ( i = 0; i < b2->surfacenum; i++ )
	{
		if ( !in )
			break;

		CSG_SplitBrush( in, b2->surfaces[i].plane, &front, &back, false );
		if ( in != b1 )
			FreeBrush( in );
		if ( front )
		{
			front->next = out;
			out = front;
		}
		in = back;
	}

	if ( in )
		FreeBrush( in );
	else
	{
		FreeBrushList( out );
		return b1;
	}

	return out;
}



bspbrush_t* RemoveBrushFromList( bspbrush_t *head, bspbrush_t *bb )
{
	bspbrush_t	*list;
	bspbrush_t	*next;

	list = NULL;
	
	for ( ; head; head=next )
	{
		next = head->next;
		if ( head == bb )
			continue;

		head->next = list;
		list = head;
	}
	return list;
}



bspbrush_t* AddBrushListToTail( bspbrush_t *tail, bspbrush_t *list )
{
	bspbrush_t	*b, *next;

	for ( b = list; b; b=next )
	{
		next = b->next;
		b->next = NULL;
		tail->next = b;
		tail = b;
	}

	return tail;
}



/*
  ====================
  CSG_Brushes

  ====================
*/
bspbrush_t* CSG_Brushes( bspbrush_t *list )
{
	bspbrush_t	*b1, *b2, *next;
	bspbrush_t	*keep, *tail;
	bspbrush_t		*sub, *sub2;
	int		num, num2;

	int		stat_not_substract = 0;

	keep = NULL;
	
restart:

	if ( !list )
		return NULL;

	for ( tail=list; tail->next; tail=tail->next )
	{ /* do nothing */ }

	for ( b1 = list; b1; b1=next )
	{
		next = b1->next;

		for ( b2=b1->next; b2; b2=b2->next )
		{
			if ( CSG_DoBrushesIntersect( b1, b2 ) == false )
				continue;

			sub = sub2 = NULL;
			num = num2 = 999999;

			//
			// sub = b1 - b2
			//
			if ( CSG_IsSubstractAllowed( b1, b2 ) )
			{
				sub = CSG_SubstractBrush( b1, b2 ); 
				if ( sub == b1 )
					continue;	// didn't intersect
				if ( !sub )
				{
					list = RemoveBrushFromList( b1, b1 );
					FreeBrush( b1 );
					goto restart;
				}
				num = BrushListLength( sub );
			}
			else
				stat_not_substract++;

//			printf( "num = %d\n", num );
			
			//
			// sub2 = b2 - b1
			//
			if ( CSG_IsSubstractAllowed( b2, b1 ) )
			{
				sub2 = CSG_SubstractBrush( b2, b1 );
				if ( sub2 == b2 )
					continue;
				if ( !sub2 )
				{
					FreeBrushList( sub );
					list = RemoveBrushFromList( b1, b2 );
					FreeBrush( b2 );
					goto restart;
				}
				num2 = BrushListLength( sub2 );
			}
			else
				stat_not_substract++;
//			printf( "num2 = %d\n", num2 );

			if ( !sub && !sub2 )
				continue;

#if 0
			if ( num > 1 && num2 > 1 )
	  		{
				FreeBrushList( sub );
				FreeBrushList( sub2 );
				continue;
			}
#endif

			if ( num < num2 )
			{
				FreeBrushList( sub2 );
				tail = AddBrushListToTail( tail, sub );
				list = RemoveBrushFromList( b1, b1 );
				FreeBrush( b1 );
				goto restart;
			}
			else
			{
				FreeBrushList( sub );
				tail = AddBrushListToTail( tail, sub2 );
				list = RemoveBrushFromList( b1, b2 );
				FreeBrush( b2 );
				goto restart;
			}
		}

		if ( !b2 )
		{
			b1->next = keep;
			keep = b1;
		}
	}

	printf( " not allowed substraction: %d\n", stat_not_substract );

	return keep;
	
}

int main( int argc, char *argv[] )
{
	char		*in_name;
	char		*out_name;
	bspbrush_t	*in_list;
	bspbrush_t	*out_list;
	bspbrush_t	*b;

	printf( "===== csg - constructiv solid geometry on bsp brushes  =====\n" );

	SetCmdArgs( argc, argv );
	
	in_name = GetCmdOpt2( "-i" );
	out_name = GetCmdOpt2( "-o" );

	if ( !in_name )
		Error( "no input file.\n" );
	if ( !out_name )
		Error( "no output file.\n" );

	printf( " input file: %s\n", in_name );
	printf( " output file: %s\n", out_name );

	p_planenum = MAX_PLANES;
	Read_Planes( "planes.asc", p_planes, &p_planenum );
	printf( " %d planes.\n", p_planenum );

	in_list = Read_BrushList( in_name );
	printf( " %d input brushes\n", BrushListLength( in_list ) );

	//
	// create polygons for in_list
	// 
	for ( b = in_list; b ; b=b->next )
		CreateBrushPolygons( b );


	out_list = CSG_Brushes( in_list );
	printf( " %d output brushes\n", BrushListLength( out_list ) );

	Write_BrushList( out_list, out_name, "csg" );
}
