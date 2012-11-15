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



// trace.c

#include "light.h"

#include "3dnow_vec.h"

#define TRACE_MAXNODES	( 64000 )

int	 trace_nodenum;
node_t	 trace_nodes[TRACE_MAXNODES];

void CompileNodeClass( hmanager_t *nodehm, hmanager_t *planehm )
{
	int			i;
	hmanager_type_iterator_t	iters[3];
	hobj_t		*bspnode;
	hobj_t		*plane;
	hobj_t		*child;
	hpair_t		*pair;
	int		index;

	int		solid_num = 0;
	int		empty_num = 0;
	int		node_num = 0;

	printf( "setup raytracing bsptree ...\n" );

	trace_nodenum = 0;
	
	HManagerIndexClassesOfType( nodehm, &trace_nodenum, "bspnodes" );
	HManagerIndexClassesOfType( nodehm, &trace_nodenum, "bspnode_front" );
	HManagerIndexClassesOfType( nodehm, &trace_nodenum, "bspnode_back" );

	if ( trace_nodenum >= TRACE_MAXNODES )
		Error( "reached TRACE_MAXNODES.\n" );

	printf( " %d trace_nodes\n", trace_nodenum );

	HManagerInitTypeSearchIterator( &iters[0], nodehm, "bspnodes" );
	HManagerInitTypeSearchIterator( &iters[1], nodehm, "bspnode_front" );
	HManagerInitTypeSearchIterator( &iters[2], nodehm, "bspnode_back" );
	
	for ( i = 0; i < 3; i++ )
	{
		for ( ; ( bspnode = HManagerGetNextClass( &iters[i] ) ); )
		{
			pair = FindHPair( bspnode, "index" );
			if ( !pair )
				Error( "missing 'index' in bspnode '%s'.\n", bspnode->name );
			HPairCastToInt_safe( &index, pair );
			trace_nodes[index].self = bspnode;
			
			pair = FindHPair( bspnode, "plane" );
			if ( pair )
			{
				// it's a node
				trace_nodes[index].type = NodeType_node;

				plane = HManagerSearchClassName( planehm, pair->value );
				if ( !plane )
					Error( "bspnode '%s' can't find plane '%s'.\n", bspnode->name, pair->value );
				trace_nodes[index].pl = GetClassExtra( plane );

				// get children
				child = FindClassType( bspnode, "bspnode_front" );
				if ( !child )
					Error( "missing 'bspnode_front' in bspnode '%s'.\n", bspnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "bspnode '%s' missing 'index' of child '%s'.\n", bspnode->name, child->name );
				HPairCastToInt( &trace_nodes[index].child[0], pair );

				child = FindClassType( bspnode, "bspnode_back" );
				if ( !child )
					Error( "missing 'bspnode_back' in bspnode '%s'.\n", bspnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "bspnode '%s' missing 'index' of child '%s'.\n", bspnode->name, child->name );
				HPairCastToInt( &trace_nodes[index].child[1], pair );

				node_num++;
			}
			else
			{
				// it's a leaf
				trace_nodes[index].pl = NULL;
				trace_nodes[index].child[0] = -1;
				trace_nodes[index].child[1] = -1;	
				trace_nodes[index].is_sky = false;

				// is it empty or solid ?
				pair = FindHPair( bspnode, "brush" );
				if ( pair )
				{
					// solid leaf
					trace_nodes[index].type = NodeType_solid;
					solid_num++;
				}
				else
				{
					// empty leaf
					trace_nodes[index].type = NodeType_empty;
					empty_num++;
				}
			}
		}
	}	

	printf( " %d are nodes.\n", node_num );
	printf( " %d are solid leafs.\n", solid_num );
	printf( " %d are empty leafs.\n", empty_num );
	
}


void SetupForSkyTrace( face_t *list, hmanager_t *brushhm )
{
	int		i;
	face_t		*f;
	hpair_t		*pair;
	int		sky_nodes;

	for ( f = list; f ; f=f->next )
	{
		if ( f->mat.is_sky )
		{
			pair = FindHPair( f->brush, "sky_brush" );
			if ( !pair )
			{
				pair = NewHPair2( "int", "sky_brush", "1" );
				InsertHPair( f->brush, pair );
			}
		}
	}

	sky_nodes = 0;
	for ( i = 0; i < trace_nodenum; i++ )
	{
		hpair_search_iterator_t		iter;

		InitHPairSearchIterator( &iter, trace_nodes[i].self, "brush" );
		for ( ; ( pair = SearchGetNextHPair( &iter ) ) ; )
		{
			hobj_t		*brush;

			brush = HManagerSearchClassName( brushhm, pair->value );
			if ( !brush )
				Error( "can't find brush.\n" );
			pair = FindHPair( brush, "sky_brush" );
			if ( pair )
			{
				if ( trace_nodes[i].is_sky == false )
				{
					trace_nodes[i].is_sky = true;
					sky_nodes++;
				}
			}
		}
	}
	printf( " %d skynodes\n", sky_nodes );
}

/*
  ==================================================
  simple line trace

  ==================================================
*/

bool_t TraceLineRecursive( int node, vec3d_t p1, vec3d_t p2 )
{
	node_t		*n;
	cplane_t		*pl;
	fp_t		d1, d2;
	int		side;
	fp_t		scale;
	vec3d_t		mid;

	MPROF_ENTRY;

 	if ( node < 0 )
		Error( "Not a node.\n" );

	n = &trace_nodes[node];

	if ( n->type == NodeType_empty )
	{
		// it's a empty leaf
 		MPROF_LEAVE;
		return false;
	}
	if ( n->type == NodeType_solid )
	{
		// it's a solid leaf => blocks ray
		MPROF_LEAVE;
		return true;
	}

	// test node
	pl = n->pl;
	d1 = _Vec3dDotProduct( p1, pl->norm ) - pl->dist;
	d2 = _Vec3dDotProduct( p2, pl->norm ) - pl->dist;

	if ( d1 >= 0 && d2 >= 0 )
	{
		// line complete frontside
		MPROF_LEAVE;
		return TraceLineRecursive( n->child[0], p1, p2 );
	}

	if ( d1 < 0 && d2 < 0 )
	{
		// line complete backside
		MPROF_LEAVE;
		return TraceLineRecursive( n->child[1], p1, p2 );
	}

	//
	// line got split
	//

	side = d1 < 0;
	scale = d1 / ( d1-d2 );
	mid[0] = p1[0] + (p2[0]-p1[0]) * scale;
	mid[1] = p1[1] + (p2[1]-p1[1]) * scale;
	mid[2] = p1[2] + (p2[2]-p1[2]) * scale;

	if ( TraceLineRecursive( n->child[side], p1, mid ) )
	{
		MPROF_LEAVE;
		return true;
	}

	if ( TraceLineRecursive( n->child[!side], mid, p2 ) )
	{
		MPROF_LEAVE;
		return true;
	}
 
	MPROF_LEAVE;
	return false;
}

#define _FP_SIGN_IS_NEG( x )		( *((unsigned int*)(&x))&(1<<31) )

/*
  ==============================
  TraceLineRecursive_3dnow

  with 3dnow optimized assembler code
  ==============================
*/

__inline__ void trace_2dists_3dnow( float *d1, float *d2, vec3d_t p1, vec3d_t p2, vec3d_t n, float *dist )
{
//	mmx_femms();
	mmx_drsm( movq, mm0, p1[0] );
	mmx_drsm( movq, mm1, p2[0] );
	mmx_drsm( movq, mm2, n[0] );

	mmx_drsr( pfmul, mm0, mm2 );
	mmx_drsr( pfacc, mm0, mm0 );

	mmx_drsr( pfmul, mm1, mm2 );
	mmx_drsr( pfacc, mm1, mm1 );

	mmx_drsr( punpckldq, mm0, mm1 );

	mmx_drsm( movd, mm1, p1[2] );
	mmx_drsm( punpckldq, mm1, p2[2] );

	mmx_drsm( movd, mm2, n[2] );
	mmx_drsr( punpckldq, mm2, mm2 );

	mmx_drsr( pfmul, mm1, mm2 );
	mmx_drsr( pfadd, mm0, mm1 );

	mmx_drsm( movd, mm1, *dist );
	mmx_drsr( punpckldq, mm1, mm1 );
	mmx_drsr( pfsub, mm0, mm1 );

	mmx_dmsr( movd, *d1, mm0 );
	mmx_drsr( punpckhdq, mm0, mm0 );
	mmx_dmsr( movd, *d2, mm0 );
//	mmx_femms();
}

__inline__ void trace_calcscale_3dnow( float *scale, float *d1, float *d2 )
{
//	mmx_femms();
	mmx_drsm( movd, mm0, *d1 );
	mmx_drsr( movq, mm2, mm0 );
	mmx_drsm( pfsub, mm0, *d2 );
	mmx_drsr( pfrcp, mm1, mm0 );
	mmx_drsr( pfrcpit1, mm0, mm1 );
	mmx_drsr( pfrcpit2, mm0, mm1 );
	mmx_drsr( pfmul, mm0, mm2 );
	mmx_dmsr( movd, *scale, mm0 );
//	mmx_femms();
}

__inline__ void trace_split_3dnow( vec3d_t mid, vec3d_t p1, vec3d_t p2, float *scale )
{
//	mmx_femms();
	mmx_drsm( movq, mm0, p2[0] );
	mmx_drsm( movd, mm1, p2[2] );
	mmx_drsm( pfsub, mm0, p1[0] );
	mmx_drsm( pfsub, mm1, p1[2] );

	mmx_drsm( movd, mm2, *scale );
	mmx_drsr( punpckldq, mm2, mm2 );
	mmx_drsr( pfmul, mm0, mm2 );
	mmx_drsr( pfmul, mm1, mm2 );

	mmx_drsm( pfadd, mm0, p1[0] );
	mmx_drsm( pfadd, mm1, p1[2] );

	mmx_dmsr( movq, mid[0], mm0 );
	mmx_dmsr( movd, mid[2], mm1 );
//	mmx_femms();
}

bool_t TraceLineRecursive_3dnow( int node, vec3d_t p1, vec3d_t p2 )
{
	node_t		*n;
	cplane_t		*pl;
        fp_t		d1, d2;
	int		side;
	fp_t		scale;
	vec3d_t		mid; 
	vec3d_t		tmp;

	MPROF_ENTRY;

	if ( node < 0 )
		Error( "Not a node.\n" );

	n = &trace_nodes[node];

	if ( n->type == NodeType_empty || n->ignore )
	{
		// it's a empty leaf
		MPROF_LEAVE;
		return false;
	}
	if ( n->type == NodeType_solid && !n->ignore )
	{
		// it's a solid leaf => blocks ray
		MPROF_LEAVE;
		return true;
	}

	// test node
	pl = n->pl;

	mmx_femms();

	{
__asm__ __volatile__ ( "nop\n" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" );
		trace_2dists_3dnow( &d1, &d2, p1, p2, pl->norm, &pl->dist );
__asm__ __volatile__ ( "nop\n" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" );
	}

//	if ( d1 >= 0 && d2 >= 0 )
	if ( !_FP_SIGN_IS_NEG( d1 ) && !_FP_SIGN_IS_NEG( d2 ) )
	{
		// line complete frontside
		mmx_femms();
		MPROF_LEAVE;
		return TraceLineRecursive_3dnow( n->child[0], p1, p2 );
	}

//	if ( d1 < 0 && d2 < 0 )
	if ( _FP_SIGN_IS_NEG( d1 ) && _FP_SIGN_IS_NEG( d2 ) )
	{
		// line complete backside
		mmx_femms();
		MPROF_LEAVE;
		return TraceLineRecursive_3dnow( n->child[1], p1, p2 );
	}

	//
	// line got split
	//


	trace_calcscale_3dnow( &scale, &d1, &d2 );
//	scale = d1 / ( d1-d2 );

	{
		vec3d_t		mid2;
__asm__ __volatile__ ( "nop\n" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" );
		trace_split_3dnow( mid, p1, p2, &scale );
__asm__ __volatile__ ( "nop\n" : : : "st", "st(1)", "st(2)", "st(3)", "st(4)", "st(5)", "st(6)", "st(7)" );		
	}
	
	mmx_femms();

	side = _FP_SIGN_IS_NEG( d1 ) ? 1 : 0; //; //d1 < 0;
//	side = d1 < 0;

	if ( TraceLineRecursive_3dnow( n->child[side], p1, mid ) )
	{
		MPROF_LEAVE;
		return true;
	}
	if ( TraceLineRecursive_3dnow( n->child[!side], mid, p2 ) )
	{
		MPROF_LEAVE;
		return true;
	}

	
	MPROF_LEAVE;
	return false;
}


bool_t TraceLine( vec3d_t from, vec3d_t to )
{
	return TraceLineRecursive_3dnow( 0, from, to );
//	return TraceLineRecursive( 0, from, to );
}

node_t * FindLeafForPoint( vec3d_t pos )
{
	int		node;
	node_t          *n;   
	cplane_t	*pl;
	fp_t            d;
	int             side;

	node = 0;
	for (;;)
	{
		n = &trace_nodes[node];
		pl = n->pl;
		if ( !pl )
			return n;
		d = Vec3dDotProduct( pos, pl->norm ) - pl->dist;
		side = d < 0;
		node = n->child[side];
	}
	return NULL;
}

/*
  ==================================================
  
  ==================================================
*/

#define TRACE2_MAX_SPLITS		( 256 )

static int		 trace2_splitnum;
static vec3d_t		 trace2_lastmid;
static vec3d_t		 trace2_splits[TRACE2_MAX_SPLITS];
static node_t		*trace2_nodes[TRACE2_MAX_SPLITS];

void TraceLine2Recursive( int node, vec3d_t p1, vec3d_t p2 )
{
	node_t		*n;
	cplane_t		*pl;
	fp_t		d1, d2;
	int		side;
	fp_t		scale;
	vec3d_t		mid;

	if ( node < 0 )
		Error( "Not a node.\n" );

	n = &trace_nodes[node];

	if ( n->type == NodeType_empty )
	{
		return;
		
	}
	if ( n->type == NodeType_solid )
	{
		if ( trace2_splitnum == TRACE2_MAX_SPLITS )
			Error( "reache TRACE2_MAX_SPLITS.\n" );

		Vec3dCopy( trace2_splits[trace2_splitnum], trace2_lastmid );	     
		trace2_nodes[trace2_splitnum] = n;
		trace2_splitnum++;
		return;
	}
	// test node
	pl = n->pl;
	d1 = _Vec3dDotProduct( p1, pl->norm ) - pl->dist;
	d2 = _Vec3dDotProduct( p2, pl->norm ) - pl->dist;

	if ( d1 >= 0 && d2 >= 0 )
	{
		// line complete frontside
		TraceLine2Recursive( n->child[0], p1, p2 );
		return;
	}

	if ( d1 < 0 && d2 < 0 )
	{
		// line complete backside
		TraceLine2Recursive( n->child[1], p1, p2 );
		return;
	}

	//
	// line got split
	//

	side = d1 < 0;
	scale = d1 / ( d1-d2 );
	mid[0] = p1[0] + (p2[0]-p1[0]) * scale;
	mid[1] = p1[1] + (p2[1]-p1[1]) * scale;
	mid[2] = p1[2] + (p2[2]-p1[2]) * scale;

	Vec3dCopy( trace2_lastmid, mid );
	
	TraceLine2Recursive( n->child[side], p1, mid );
	TraceLine2Recursive( n->child[!side], mid, p2 );	
}

int TraceLine2( vec3d_t from, vec3d_t to )
{
	trace2_splitnum = 0;
	TraceLine2Recursive( 0, from, to );
	return trace2_splitnum;
}

void TraceLine2_GetNearestSplit( vec3d_t from, vec3d_t split, node_t **n )
{
	int		i;
	fp_t		min;
	int		best;

	min = 999999.9;
	best = -1;
	for ( i = 0; i < trace2_splitnum; i++ )
	{
		vec3d_t		v;
		fp_t		l;
		Vec3dSub( v, from, trace2_splits[i] );
		l = Vec3dLen( v );
		if ( l < min )
		{
			min = l;
			best = i;
		}
	}
	if ( best == -1 )
		Error( "can't get nearest split\n" );
	Vec3dCopy( split, trace2_splits[best] );
	*n = trace2_nodes[best];
}
