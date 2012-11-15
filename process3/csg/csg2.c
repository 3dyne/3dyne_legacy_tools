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



// csg2.c

#include "csg2.h"

int		p_planenum;
plane_t		p_planes[MAX_PLANES];

int		input_accept = 0;
int		output_accept = 0;

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
#if 0
bool_t IsBrushExactOnPlane( cbspbrush_t *in, cplane_t *pl, bool_t bsp_enable )
{
	int		i;
	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( in->surfaces[i].pl == pl->flipplane )
//		if ( (in->surfaces[i].plane&~1) == (plane&~1) )
		{
			// it's the same plane
			if ( bsp_enable )
			{
				if ( in->surfaces[i].state & SURFACE_STATE_ONNODE )
				{
					printf( " * IsBrushExactOnPlane : surface allready on node. *\n" );
					printf( " * brush id: %s *\n", in->original->name );
				}
				in->surfaces[i].state |= SURFACE_STATE_ONNODE;
			}
			return true;
		}
	}
	return false;
}
#else
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
#endif

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
cbspbrush_t * AddSurfaceToBrush( cbspbrush_t *in, cplane_t *addpl, bool_t bsp_enable )
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
	surfs[surfnum].td = NULL;	// fixme: maybe a default texdef obj ?
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
	
	*front = AddSurfaceToBrush( in, plflip, bsp_enable );
	*back = AddSurfaceToBrush( in, pl, bsp_enable );

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
		printf( "no front, no back ?\n" );
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
bool_t CSG_DoBrushesIntersect( cbspbrush_t *b1, cbspbrush_t *b2 )
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
			if ( b1->surfaces[i].pl ==
			     b2->surfaces[j].pl->flipplane )
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
bool_t CSG_IsSubstractAllowed( cbspbrush_t *b1, cbspbrush_t *b2 )
{
	unsigned int		c1, c2;

//	return true;

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
cbspbrush_t * CSG_SubstractBrush( cbspbrush_t *b1, cbspbrush_t *b2 )
{
	int		i;
	cbspbrush_t	*in, *out;
	cbspbrush_t	*front, *back;

	in = b1;
	out = NULL;

	for ( i = 0; i < b2->surfacenum; i++ )
	{
		if ( !in )
			break;

//		CSG_SplitBrush( in, b2->surfaces[i].pl, &front, &back, false );
		CSG_SplitBrush_new( in, b2->surfaces[i].pl, &front, &back );
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



cbspbrush_t * RemoveBrushFromList( cbspbrush_t *head, cbspbrush_t *bb )
{
	cbspbrush_t	*list;
	cbspbrush_t	*next;

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



cbspbrush_t * AddBrushListToTail( cbspbrush_t *tail, cbspbrush_t *list )
{
	cbspbrush_t	*b, *next;

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
cbspbrush_t* CSG_Brushes( cbspbrush_t *list )
{
	cbspbrush_t	*b1, *b2, *next;
	cbspbrush_t	*keep, *tail;
	cbspbrush_t		*sub, *sub2;
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

void CountBrushTypes( cbspbrush_t *list, int *solid, int *liquid, int *deco, int *hull )
{
	*solid = *liquid = *deco = *hull = 0;
	for ( ; list ; list=list->next )
	{
		if ( list->contents == 16 )
			(*solid)++;
		if ( list->contents == 8 )
			(*liquid)++;
		if ( list->contents == 4 )
			(*deco)++;
		if ( list->contents == 2 )
			(*hull)++;
	}
}

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
  ==================================================
  cbspbrush stuff

  ==================================================
*/

/*
  ====================
  ReadBspbrushClass

  ====================
*/
#if 1
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
			{
				bb->surfaces[i].contents = 0;
			}
			else
			{
				HPairCastToInt( &bb->surfaces[i].contents, pair );
			}

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
#endif

/*
  ====================
  WriteBspbrushClass

  ====================
*/
hobj_t * BuildPolygonClass( polygon_t *p )
{
	hobj_t		*polycls;
	hobj_t		*pointcls;
	hpair_t		*pair;
	int		i;
	char		tt[256];

	sprintf( tt, "#%u", HManagerGetFreeID() );
	polycls = NewClass( "polygon", tt );
	
	sprintf( tt, "%d", p->pointnum );
	pair = NewHPair2( "int", "num", tt );
	InsertHPair( polycls, pair );

	for ( i = 0; i < p->pointnum; i++ )
	{
//		sprintf( tt, "#%u", HManagerGetFreeID() );
//		pointcls = NewClass( "point", tt );

		pair = NewHPair();		
		sprintf( pair->value, "%f %f %f", p->p[i][0], p->p[i][1], p->p[i][2] );
		sprintf( pair->key, "%d", i );
		sprintf( pair->type, "vec3d" );
		
//		InsertHPair( pointcls, pair );		

//		InsertClass( polycls, pointcls );
		InsertHPair( polycls, pair );
	}

	return polycls;
}

void WriteBspbrushClass( char *name, cbspbrush_t *list, bool_t with_polygons, unsigned int accept_contents )
{
	FILE		*h;
	hobj_t		*bspbrushcls;
	cbspbrush_t	*b;
	char		tt[256];
	int		i;
	hpair_t		*pair;
	hobj_t		*bspbrush;
	hobj_t		*surface;
	hobj_t		*poly;	

	hmanager_t		*hm;

	bspbrushcls = NewClass( "bspbrushes", "bspbrushes1" );

	for ( b = list; b ; b=b->next )
	{
		if ( !(b->contents & accept_contents ) )
			continue;

		output_accept++;

		sprintf( tt, "#%u", HManagerGetFreeID() );
		bspbrush = NewClass( "bspbrush", tt );

		sprintf( tt, "%u", b->contents );
		pair = NewHPair2( "int", "content", tt );
		InsertHPair( bspbrush, pair );

		pair = FindHPair( b->original, "original" );
		if ( !pair )
			Error( "xxx\n" );
		pair = NewHPair2( "ref", "original", pair->value );
		InsertHPair( bspbrush, pair );

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

			if ( with_polygons )
			{
				// polygon
				poly = BuildPolygonClass( b->surfaces[i].p );
				InsertClass( surface, poly );
			}

			if ( b->surfaces[i].state & SURFACE_STATE_DONT_SPLIT )
			{
				pair = NewHPair2( "int", "bsp_dont_split", "1" );
				InsertHPair( surface, pair );
			}
			
			InsertClass( bspbrush, surface );
		}

		InsertClass( bspbrushcls, bspbrush );
	}

	DeepDumpClass( bspbrushcls );

	hm = NewHManager();
	HManagerSetRootClass( hm, bspbrushcls );
	HManagerRebuildHash( hm );
	DumpHManager( hm, false );

	h = fopen( name, "w" );
	WriteClass( bspbrushcls, h );
	fclose( h );
}

/*
  ====================
  BuildCBspbrushList

  ====================
*/
cbspbrush_t * BuildCBspbrushList( hmanager_t *hm, int accept_contents )
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

		if ( !(b->contents & accept_contents ) )
			continue;
		input_accept++;

		b->next = list;
		list = b;
	}

	return list;
}

void PrintHelp( void )
{
	puts( "" );
}

int main( int argc, char *argv[] )
{
	char		*in_name;
	char		*out_name;
	char		*plane_name;
	char		*texdef_name;
	bool_t		ignore_texdef;

	cbspbrush_t	*in_list;
	cbspbrush_t	*out_list;
	cbspbrush_t	*b;

	cbspbrush_t	*bspbrushlist;

	hmanager_t	*planecls;
	hmanager_t	*texdefcls;
	hmanager_t	*bspbrushcls;

	int		solid_num, liquid_num, deco_num, hull_num;

	printf( "===== csg - constructiv solid geometry on bsp brushes  =====\n" );

	SetCmdArgs( argc, argv );
	
	in_name = GetCmdOpt2( "-i" );
	out_name = GetCmdOpt2( "-o" );
	plane_name = GetCmdOpt2( "-pl" );
	texdef_name = GetCmdOpt2( "-td" );
	ignore_texdef = CheckCmdSwitch2( "--ignore-texdef" );

	if ( !in_name )
	{
		in_name = "_bspbrush.hobj";
		printf( " default input bspbrush class: %s\n", in_name );
	}
	else
	{
		printf( " input bspbrush class: %s\n", in_name );
	}
	if ( !out_name )
	{
		out_name = "_csgout_bspbrush.hobj";
		printf( " default output bspbrush class: %s\n", out_name );
	}
	else
	{
		printf( " output bspbrush class: %s\n", out_name );
	}
	if ( !plane_name )
	{
		plane_name = "_plane.hobj";
		printf( " default plane class: %s\n", plane_name );
	}
	else
	{
		printf( " plane class: %s\n", plane_name );
	}

	if ( ignore_texdef )
	{
		texdef_name = NULL;
		printf( "switch: ignore texdef on.\n" );
	}
	else
	{
		if ( !texdef_name )
		{
			texdef_name = "_texdef.hobj";
			printf( " default texdef class: %s\n", texdef_name );
		}
		else
		{
			printf( " texdef class: %s\n", texdef_name );
		}
	}

	planecls = ReadPlaneClass( plane_name );
	if ( texdef_name )
	{
		texdefcls = ReadTexdefClass( texdef_name );
	}
	else
	{
		texdefcls = NULL;
	}
	bspbrushcls = ReadBspbrushClass( in_name, planecls, texdefcls );


	if ( GetCmdOpt2( "--input-accept" ) )
	{
		printf( "Switch: --input-accept\n" );
		bspbrushlist = BuildCBspbrushList( bspbrushcls, atoi(GetCmdOpt2( "--input-accept" )) );
	}
	else
	{
		bspbrushlist = BuildCBspbrushList( bspbrushcls, 16+8+4+2 );
	}

	printf( " accepted input brushes: %d\n", input_accept );

	//
	// create polygons for in_list
	// 
	for ( b = bspbrushlist; b ; b=b->next )
		CreateBrushPolygons( b );
	printf( " %d input brushes\n", BrushListLength( bspbrushlist ) );

	CountBrushTypes( bspbrushlist, &solid_num, &liquid_num, &deco_num, &hull_num );
	printf( "input statistic:\n" );
	printf( " %d solid, %d liquid, %d deco, %d hull\n", solid_num, liquid_num, deco_num, hull_num );

	if ( CheckCmdSwitch2( "--no-csg" ) )
	{
		printf( "Switch: --no-csg\n" );
		out_list = bspbrushlist;
	}
	else
	{
		printf( "run csg ...\n" );
		out_list = CSG_Brushes( bspbrushlist );
		printf( " %d output brushes\n", BrushListLength( out_list ) );
	}

	CountBrushTypes( out_list, &solid_num, &liquid_num, &deco_num, &hull_num );
	printf( "output statistic:\n" );
	printf( " %d solid, %d liquid, %d deco, %d hull\n", solid_num, liquid_num, deco_num, hull_num );


	printf( "build output bspbrush class ...\n" );
	if ( CheckCmdSwitch2( "--save-with-polygons" ) )
	{
		printf( "Switch: --save-with-polygons\n" );
		WriteBspbrushClass( out_name, out_list, true, 2+4+8+16 );
	}
	else
	{
		if ( GetCmdOpt2( "--output-accept" ) )
			WriteBspbrushClass( out_name, out_list, false, atoi(GetCmdOpt2( "--output-accept" ))  );
		else
			WriteBspbrushClass( out_name, out_list, false, 2+4+8+16 );
	}

	printf( " accepted output brushes: %d\n", output_accept );

	HManagerSaveID();
	
	exit(0);
}
