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



// type_bspbrush.c

#include "type_bspbrush.h"

static int		stat_bspbrushnum = 0;
static int		stat_bspbrushtotal = 0;

/*
  ====================
  NewBrush

  ====================
*/
bspbrush_t* NewBrush( int surfacenum )
{
	size_t		size;
	bspbrush_t	*bb;

	if ( surfacenum > MAX_SURFACES_PER_BRUSH )
		Error( "NewBspbrush: MAX_SURFACES_PER_BRUSH (%d)\n", surfacenum );

	stat_bspbrushnum++;
	stat_bspbrushtotal++;

	size = (size_t)&(((bspbrush_t *)0)->surfaces[surfacenum]);
	bb = (bspbrush_t *) malloc( size );
	memset( bb, 0, size );

	return bb;
}



/*
  ====================
  FreeBrush

  ====================
*/
void FreeBrush( bspbrush_t *bb )
{
	int		i;

	stat_bspbrushnum--;

	if ( !bb )
		return;

	for ( i = 0; i < bb->surfacenum; i++ )
	{
		if ( bb->surfaces[i].p )
			FreePolygon( bb->surfaces[i].p );
	}
	
	free( bb );
}



/*
  ====================
  FreeBrushList

  ====================
*/
void FreeBrushList( bspbrush_t *list )
{
	bspbrush_t	*next;

	if ( !list )
		return;

	for ( ; list ;list = next )
	{
		next = list->next;
		FreeBrush( list );
	}
}



/*
  ====================
  CopyBrush

  ====================
*/
bspbrush_t* CopyBrush( bspbrush_t *in )
{
	int		i;
	size_t	size;
	bspbrush_t	*bb;

	size = (size_t)&(((bspbrush_t *)0)->surfaces[in->surfacenum]);
	bb = NewBrush( in->surfacenum );
	memcpy( bb, in, size );

	for ( i = 0; i < in->surfacenum; i++ )
	{
		if ( in->surfaces[i].p )
			bb->surfaces[i].p = CopyPolygon( in->surfaces[i].p );
	}

	return bb;
}



/*
  ====================
  BrushListLength

  ====================
*/
int BrushListLength( bspbrush_t *list )
{
	int		i;
	
	for ( i = 0; list; list=list->next, i++ )
		; /* do nothing */
	
	return i;
}



/*
  ====================
  CreateBrushPolygons

  ====================
*/
void CreateBrushPolygons( bspbrush_t *b )
{
	int		i, j;
	plane_t		*pl;

	for( i = 0; i < b->surfacenum; i++ )
	{
		pl = &p_planes[b->surfaces[i].plane];
		b->surfaces[i].p = BasePolygonForPlane( pl->norm, pl->dist );

		for ( j = 0; j < b->surfacenum; j++ )
		{
			if ( i == j )
				continue;

			if ( b->surfaces[i].p )
			{
				pl = &p_planes[b->surfaces[j].plane];
				ClipPolygonInPlace( &b->surfaces[i].p, pl->norm, pl->dist );
			}
		}
		
		if ( !b->surfaces[i].p )
			Error( " * CreateBrushPolygons: clipped away polygon. *\n" );
	}

	CalcBrushBounds( b );
}



/*
  ====================
  CalcBrushBounds

  ====================
*/
void CalcBrushBounds( bspbrush_t *bb )
{
	int		i;
	int		j;

	Vec3dInitBB( bb->min, bb->max, 999999.9 );

	for ( i = 0; i < bb->surfacenum; i++ )
	{
		if ( !bb->surfaces[i].p )
			continue;

		for ( j = 0; j < bb->surfaces[i].p->pointnum; j++ )
			Vec3dAddToBB( bb->min, bb->max, bb->surfaces[i].p->p[j] );
	}      

	for ( i = 0; i < 3; i++ )
		if ( bb->min[i] < -8000.0 || bb->max[i] > 8000.0 )
		{
			printf( " * CalcBrushBounds: critical big polygon in wbrush: %d\n", bb->original );
			break;
		}
	
	for ( i = 0; i < 3; i++ )
		if ( bb->min[i] > 99999.9 || bb->max[i] < -99999.9 ) 
		{
			printf( " * CalcBrushBounds: odd bb for brush id: %d *\n", bb->original );
			break;
		}
}



/*
  ====================
  CalcBrushListBounds

  ====================
*/
void CalcBrushListBounds( bspbrush_t *head, vec3d_t min, vec3d_t max )
{
	Vec3dInitBB( min, max, 999999.9 );
	
	for ( ; head ; head=head->next )
	{
//		Vec3dPrint( head->min );
//		Vec3dPrint( head->max );
		Vec3dAddToBB( min, max, head->min );
		Vec3dAddToBB( min, max, head->max );
	}
}



/*
  ====================
  CalcBrushVolume

  ====================
*/
fp_t CalcBrushVolume( bspbrush_t *in )
{
	int		i;
	fp_t		area, volume, d;
	polygon_t	*p;
	vec3d_t		corner;
	plane_t		*pl;

	if ( !in )
		return 0.0;

	p = NULL;
	for ( i = 0; i < in->surfacenum; i++ )
	{
		p = in->surfaces[i].p;
		if ( p )
			break;
	}

	if ( !p )
		return 0.0;

	Vec3dCopy( corner, p->p[0] );

	volume = 0;
	for ( ; i < in->surfacenum; i++ )
	{
		p = in->surfaces[i].p;
		if ( !p )
			continue;
		pl = &p_planes[in->surfaces[i].plane];
		d = -(Vec3dDotProduct( corner, pl->norm ) - pl->dist );
		area = PolygonArea( p );
		volume += d*area;
	}
	volume /= 3;
	return volume;
}



/*
  ====================
  Read_Brush

  ====================
*/
bspbrush_t*	Read_Brush( tokenstream_t *ts )
{
	int		i;
	bspbrush_t	*b;
	unsigned int	contents;
	int		surfacenum;
	int		wbrush;
	
	// get wbrush 
	GetToken( ts );
	wbrush = atoi( ts->token );

	// get contents
	GetToken( ts );
	contents = atoi( ts->token );

	// get surface num
	GetToken( ts );
	surfacenum = atoi( ts->token );

	//
	// create brush 
	//
	b = NewBrush( surfacenum );
	b->surfacenum = surfacenum;
	b->contents = contents;
	b->original = wbrush;
	
	//
	// read surfaces
	//
	for ( i = 0; i < surfacenum; i++ )
	{
		// get plane
		GetToken( ts );
		b->surfaces[i].plane = atoi( ts->token );

		// get contents
		GetToken( ts );
		b->surfaces[i].contents = atoi( ts->token );

		// get state
		GetToken( ts );
		b->surfaces[i].state = atoi( ts->token );
	}

	return b;
}



/*
  ====================
  Read_BrushList

  ====================
*/
bspbrush_t*	Read_BrushList( char *name )
{
	tokenstream_t		*ts;
	bspbrush_t		*head;
	bspbrush_t		*b;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_BrushList: can't open file '%s'\n", name );

	head = NULL;

	for(;;)
	{
		// get bspbrush or end
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		// it's a bspbrush num

		b = Read_Brush( ts );

		b->next = head;
		head = b;
	}

	EndTokenStream( ts );

	return head;
}



/*
  ====================
  Read_BrushArray

  ====================
*/
void Read_BrushArray( bspbrush_t **base, int *maxnum, char *name )
{
	tokenstream_t		*ts;
	int		i;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_BrushArray: can't open file '%s'\n", name );

	for ( i = 0; i < *maxnum; i++ )
	{
		// get bspbrush or end
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		if ( i != atoi( ts->token ) )
			Error( "Read_BrushArray: brushes not sorted.\n" );
		
		base[i] = Read_Brush( ts );
	}

	if ( i == *maxnum )
		Error( "Read_BrushArray: reached maxnum (%d)\n", *maxnum );

	EndTokenStream( ts );
	*maxnum = i;
}


/*
  ====================
  Write_Brush

  ====================
*/
void Write_Brush( FILE *h, bspbrush_t *b )
{
	int		j;

	fprintf( h, "%d %u %d\n",
		 b->original,
		 b->contents,
		 b->surfacenum );

	for ( j = 0; j < b->surfacenum; j++ )
	{
		fprintf( h, " %d %u %u\n", 
			 b->surfaces[j].plane,	// surface plane
			 b->surfaces[j].contents,	// surface contents
			 b->surfaces[j].state );	// surface state
	}	
}



/*
  ====================
  Write_BrushList

  ====================
*/
void Write_BrushList( bspbrush_t *list, char *name, char *creator )
{
	FILE		*h;
	int		i;
	bspbrush_t	*b;

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_BrushList: can't open file '%s'\n", name );	

	fprintf( h, "# bspbrush file\n" );
	fprintf( h, "# generated by %s !!! DON'T EDIT !!!\n", creator );
	fprintf( h, "# <bspbrush> <wbrushnum> <contents> <surfacenum> [ <plane> <contents> <state> ]\n" );

	i = 0;
	for ( b = list; b ; b=b->next, i++ )
	{
		fprintf( h, "%d ", i );
		Write_Brush( h, b );		
	}

	fprintf( h, "end\n" );

	fclose( h );
}



/*
  ====================
  Write_BrushArray

  ====================
*/
void Write_BrushArray( bspbrush_t **base, int num, char *name, char *creator )
{
	FILE		*h;
	int		i;
	
	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_BrushArray: can't open file '%s'\n", name );

	fprintf( h, "# bspbrush file\n" );
	fprintf( h, "# generated by %s !!! DON'T EDIT !!!\n", creator );
	fprintf( h, "# <bspbrush> <wbrushnum> <contents> <surfacenum> [ <plane> <contents> <state> ]\n" );

	for ( i = 0; i < num; i++ )
	{
		fprintf( h, "%d ", i );
		Write_Brush( h, base[i] );
	}

	fprintf( h, "end\n" );
	
	fclose( h );
}
