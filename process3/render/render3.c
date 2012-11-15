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



// render3.c

#include "render.h"
#include "r_hvis.h"
#include "r_facesetup.h"

#include "sys_dep.h"

/*
  ==================================================
  vertex rep

  ==================================================
*/

// typedef of vertex_t moved to r_defs.h

#define	MAX_VERTICES		( 8192 )
int			r_vertexnum;
vertex_t		r_vertices[MAX_VERTICES];

vec3d_t			r_scls[MAX_VERTICES];
vec4d_t			r_eyes[MAX_VERTICES];

void CompileVertexClass( hmanager_t *vertexhm )
{
	hobj_search_iterator_t	iter;
	hobj_t		*vertex;
	hpair_t		*pair;
	int		num;
	char		tt[256];

	InitClassSearchIterator( &iter, HManagerGetRootClass( vertexhm ), "vertex" );
	for ( num = 0; ( vertex = SearchGetNextClass( &iter ) ); num++ )
	{
		if ( num == MAX_VERTICES )
			Error( "reached MAX_VERTICES.\n" );

		pair = FindHPair( vertex, "point" );
		if ( !pair )
			Error( "missing 'point' in vertex '%s'.\n", vertex->name );
		HPairCastToVec3d_safe( r_vertices[num].v, pair );

		// setup vertex array test
//		Vec3dScale(r_scls[num], 1.0/16.0, r_vertices[num].v );
		r_vertices[num].count = 0;
		r_vertices[num].fs_count = 0;
//		r_scls[num][1] = -r_scls[num][1];

		// insert own index into class
		sprintf( tt, "%d", num );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( vertex, pair );
	}
	r_vertexnum = num;
	
	printf( " %d vertices\n", num );
}

/*
  ==================================================
  brush & face rep

  ==================================================
*/

// typedef of vertexref_t moved to r_defs.h

// typedef of fixpolygon_t moved to r_defs.h

// typedef of fixpolygonref_t moved to r_defs.h

typedef struct brush_s
{
	bool_t	debug_draw;
	int	mapnode;	// for debug
	int	fixpolygonnum;
	int	startfixpolygonref;
	int	count;
} brush_t;

#define MAX_VERTEXREFS		( 64000 )
#define MAX_FIXPOLYGONS		( 32000 )
#define MAX_FIXPOLYGONREFS	( 16000 )
#define MAX_BRUSHES		(  8000 )

int	r_vertexrefnum;
int	r_fixpolygonnum;
int	r_fixpolygonrefnum;
int	r_brushnum;

vertexref_t	r_vertexrefs[MAX_VERTEXREFS];
fixpolygon_t	r_fixpolygons[MAX_FIXPOLYGONS];
fixpolygonref_t	r_fixpolygonrefs[MAX_FIXPOLYGONREFS];
brush_t		r_brushes[MAX_BRUSHES];

void CompileBrushClass( hmanager_t *brushhm, hmanager_t *vertexhm, hmanager_t *planehm, hmanager_t *lightdefhm, hmanager_t *texdefhm )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	surfiter;
	hobj_search_iterator_t	polyiter;
	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*texdef;
	hobj_t		*plane;
	hobj_t		*poly;
	hobj_t		*lightdef;
	hobj_t		*vertex;
	hpair_t		*pair;
	int		brushnum;
	int		surfnum;
	int		polynum;
	int		pointnum;
	int		num, i;
	int		texdefindex;
	char		tt[256];

	//
	// first build vertexrefs and fixpolygons
	//

	r_vertexrefnum = 0;
	r_fixpolygonnum = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( brushhm ), "bspbrush" );	
	for ( brushnum = 0; ( brush = SearchGetNextClass( &iter ) ); num++ )
	{
		// surface

		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( surfnum = 0; ( surface = SearchGetNextClass( &surfiter ) ); surfnum++ )
		{
			pair = FindHPair( surface, "texdef" );
			if ( pair )
			{
				texdef = HManagerSearchClassName( texdefhm, pair->value );
				if ( !texdef )
					Error( "surface '%s' can't find texdef '%s'.\n", surface->name, pair->value );
				pair = FindHPair( texdef, "index" );
				if ( !pair )
					Error( "missing 'index' in texdef '%s'.\n", texdef->name );
				HPairCastToInt_safe( &texdefindex, pair );
			}
			else
			{
				texdefindex = -1;
			}

			pair = FindHPair( surface, "plane" );
			if ( !pair )
				Error( "missing 'plane' in surface '%s'.\n", surface->name );
			plane = HManagerSearchClassName( planehm, pair->value );

			// fixpolys

			InitClassSearchIterator( &polyiter, surface, "fixpolygon" );
			for ( polynum = 0; ( poly = SearchGetNextClass( &polyiter ) ); polynum++ )
			{
				r_fixpolygons[r_fixpolygonnum].count = 0;
				
				// get lightdef
				pair = FindHPair( poly, "lightdef" );
				if ( !pair )
				{
					r_fixpolygons[r_fixpolygonnum].lightdef = -1;
				}
				else
				{
					lightdef = HManagerSearchClassName( lightdefhm, pair->value );
					if ( !lightdef )
						Error( "fixpolygon '%s' can't find lightdef '%s'.\n", poly->name, pair->value );
					pair = FindHPair( lightdef, "index" );
					if ( !pair )
						Error( "missing 'index' in lightdef '%s'.\n", lightdef->name );
					HPairCastToInt_safe( &r_fixpolygons[r_fixpolygonnum].lightdef, pair );
				}

				// set texdef 
				r_fixpolygons[r_fixpolygonnum].texdef = texdefindex;

				// set plane
				r_fixpolygons[r_fixpolygonnum].pl = GetClassExtra( plane );
				
				// get pointnum
				
				pair = FindHPair( poly, "pointnum" );
				if ( !pair )
					Error( "missing 'pointnum' in fixpolygon '%s'.\n", poly->name );
				HPairCastToInt_safe( &pointnum, pair );
				
				r_fixpolygons[r_fixpolygonnum].pointnum = pointnum;
				r_fixpolygons[r_fixpolygonnum].startvertexref = r_vertexrefnum;

				// get vertexrefs
				for ( i = 0; i < pointnum; i++ )
				{
					sprintf( tt, "%d", i );
					pair = FindHPair( poly, tt );
					if ( !pair )
						Error( "missing vertex clsref '%s' in fixpolygon '%s'.\n", tt, poly->name );

					vertex = HManagerSearchClassName( vertexhm, pair->value );
					if ( !vertex )
						Error( "fixpolygon '%s' can't find vertex '%s'.\n", poly->name, pair->value );
					
					// get index of vertex
					pair = FindHPair( vertex, "index" );
					if ( !pair )
						Error( "missing 'index' of vertex '%s'.\n", vertex->name );
					HPairCastToInt_safe( &r_vertexrefs[r_vertexrefnum++].vertex, pair );
				}
				
				// insert own index into class
				sprintf( tt, "%d", r_fixpolygonnum );
				pair = NewHPair2( "int", "index", tt );
				InsertHPair( poly, pair );
				
				r_fixpolygonnum++;
			}
		}
	}		

	printf( " %d vertexrefs\n", r_vertexrefnum );
	printf( " %d fixpolygons\n", r_fixpolygonnum );



	//
	// build brushes and fixpolygonrefs
	//
	

	r_fixpolygonrefnum = 0;
	r_brushnum = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( brushhm ), "bspbrush" );	
	for ( brushnum = 0; ( brush = SearchGetNextClass( &iter ) ); num++ )
	{
		// debug hack
		if ( !strcmp( brush->name, "#105823" ) ||
		     !strcmp( brush->name, "#105811" ) ||
		     !strcmp( brush->name, "#105798" ) )
		{
			r_brushes[r_brushnum].debug_draw = true;
			printf( "debug brush '%s' !\n", brush->name );
		}
		else
		{
			r_brushes[r_brushnum].debug_draw = false;
		}


		r_brushes[r_brushnum].startfixpolygonref = r_fixpolygonrefnum;
		r_brushes[r_brushnum].fixpolygonnum = 0;
		r_brushes[r_brushnum].count = 0;

		// surface
		
		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( surfnum = 0; ( surface = SearchGetNextClass( &surfiter ) ); surfnum++ )
		{						
			// fixpolys
			
			InitClassSearchIterator( &polyiter, surface, "fixpolygon" );
			for ( polynum = 0; ( poly = SearchGetNextClass( &polyiter ) ); polynum++ )
			{				
				// get fixpoly index
				
				pair = FindHPair( poly, "index" );
				if ( !pair )
					Error( "missing 'index' in fixpolygon '%s'.\n", poly->name );
				HPairCastToInt_safe( &r_fixpolygonrefs[r_fixpolygonrefnum++].fixpolygon, pair );
				r_brushes[r_brushnum].fixpolygonnum++;
			}
		}
		
		// insert own index into class
		sprintf( tt, "%d", r_brushnum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( brush, pair );

		r_brushnum++;
	}

	printf( " %d fixpolygonrefs\n", r_fixpolygonrefnum );
	printf( " %d brushnum\n", r_brushnum );
				
}


void CompileBrushClass_debug( hmanager_t *brushhm, hmanager_t *vertexhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;
	hobj_search_iterator_t	surfiter;
	hobj_search_iterator_t	polyiter;
	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*texdef;
	hobj_t		*plane;
	hobj_t		*poly;
	hobj_t		*lightdef;
	hobj_t		*vertex;
	hpair_t		*pair;
	int		brushnum;
	int		surfnum;
	int		polynum;
	int		pointnum;
	int		num, i;
	int		texdefindex;
	char		tt[256];

	//
	// first build vertexrefs and fixpolygons
	//

	r_vertexrefnum = 0;
	r_fixpolygonnum = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( brushhm ), "bspbrush" );	
	for ( brushnum = 0; ( brush = SearchGetNextClass( &iter ) ); num++ )
	{
		// surface

		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( surfnum = 0; ( surface = SearchGetNextClass( &surfiter ) ); surfnum++ )
		{
			pair = FindHPair( surface, "plane" );
			if ( !pair )
				Error( "missing 'plane' in surface '%s'.\n", surface->name );
			plane = HManagerSearchClassName( planehm, pair->value );

			// fixpolys

			InitClassSearchIterator( &polyiter, surface, "fixpolygon" );
			for ( polynum = 0; ( poly = SearchGetNextClass( &polyiter ) ); polynum++ )
			{
				// set plane
				r_fixpolygons[r_fixpolygonnum].pl = GetClassExtra( plane );
				
				// get pointnum
				
				pair = FindHPair( poly, "pointnum" );
				if ( !pair )
					Error( "missing 'pointnum' in fixpolygon '%s'.\n", poly->name );
				HPairCastToInt_safe( &pointnum, pair );
				
				r_fixpolygons[r_fixpolygonnum].pointnum = pointnum;
				r_fixpolygons[r_fixpolygonnum].startvertexref = r_vertexrefnum;

				// get vertexrefs
				for ( i = 0; i < pointnum; i++ )
				{
					sprintf( tt, "%d", i );
					pair = FindHPair( poly, tt );
					if ( !pair )
						Error( "missing vertex clsref '%s' in fixpolygon '%s'.\n", tt, poly->name );

					vertex = HManagerSearchClassName( vertexhm, pair->value );
					if ( !vertex )
						Error( "fixpolygon '%s' can't find vertex '%s'.\n", poly->name, pair->value );
					
					// get index of vertex
					pair = FindHPair( vertex, "index" );
					if ( !pair )
						Error( "missing 'index' of vertex '%s'.\n", vertex->name );
					HPairCastToInt_safe( &r_vertexrefs[r_vertexrefnum++].vertex, pair );
				}
				
				// insert own index into class
				sprintf( tt, "%d", r_fixpolygonnum );
				pair = NewHPair2( "int", "index", tt );
				InsertHPair( poly, pair );
				
				r_fixpolygonnum++;
			}
		}
	}		

	printf( " %d vertexrefs\n", r_vertexrefnum );
	printf( " %d fixpolygons\n", r_fixpolygonnum );



	//
	// build brushes and fixpolygonrefs
	//
	

	r_fixpolygonrefnum = 0;
	r_brushnum = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( brushhm ), "bspbrush" );	
	for ( brushnum = 0; ( brush = SearchGetNextClass( &iter ) ); num++ )
	{
		// debug hack
		if ( !strcmp( brush->name, "#105823" ) ||
		     !strcmp( brush->name, "#105811" ) ||
		     !strcmp( brush->name, "#105798" ) )
		{
			r_brushes[r_brushnum].debug_draw = true;
			printf( "debug brush '%s' !\n", brush->name );
		}
		else
		{
			r_brushes[r_brushnum].debug_draw = false;
		}


		r_brushes[r_brushnum].startfixpolygonref = r_fixpolygonrefnum;
		r_brushes[r_brushnum].fixpolygonnum = 0;
		r_brushes[r_brushnum].count = 0;

		// surface
		
		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( surfnum = 0; ( surface = SearchGetNextClass( &surfiter ) ); surfnum++ )
		{						
			// fixpolys
			
			InitClassSearchIterator( &polyiter, surface, "fixpolygon" );
			for ( polynum = 0; ( poly = SearchGetNextClass( &polyiter ) ); polynum++ )
			{				
				// get fixpoly index
				
				pair = FindHPair( poly, "index" );
				if ( !pair )
					Error( "missing 'index' in fixpolygon '%s'.\n", poly->name );
				HPairCastToInt_safe( &r_fixpolygonrefs[r_fixpolygonrefnum++].fixpolygon, pair );
				r_brushes[r_brushnum].fixpolygonnum++;
			}
		}
		
		// insert own index into class
		sprintf( tt, "%d", r_brushnum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( brush, pair );

		r_brushnum++;
	}

	printf( " %d fixpolygonrefs\n", r_fixpolygonrefnum );
	printf( " %d brushnum\n", r_brushnum );
				
}



/*
  ==================================================
  mapnode rep

  ==================================================
*/

// typedef of face_t moved to r_defs.h

typedef struct brushref_s
{
	int		brush;
} brushref_t;

typedef struct leafref_s
{
	int		mapnode;
} leafref_t;

#define MAX_MAPNODES	( 8192 )
#define MAX_LEAFREFS	( 32000 )
#define MAX_BRUSHREFS	( 16000 )

#define MAX_FACES		( 16000 )

int		r_facenum;
face_t		r_faces[MAX_FACES];

int		r_leafrefnum;
int		r_brushrefnum;
leafref_t	r_leafrefs[MAX_LEAFREFS];
brushref_t	r_brushrefs[MAX_BRUSHREFS];

int		r_mapnodenum;
mapnode_t	r_mapnodes[MAX_MAPNODES];

int		r_visleafnum;
int		r_leafbitpos[MAX_MAPNODES];

void CompileMapnodeClass( hmanager_t *mapnodehm, hmanager_t *brushhm, hmanager_t *vertexhm, hmanager_t *planehm )
{
	int		i;
	hmanager_type_iterator_t	iters[3];
	hpair_search_iterator_t		iter;
	hobj_t		*mapnode;
	hobj_t		*brush, *leaf, *plane, *child;
	hpair_t		*pair;
	int		index;
	int		bitpos;
	int		see_buffer_size;

	r_leafrefnum = 0;
	r_brushrefnum = 0;	
	r_mapnodenum = 0;
	r_visleafnum = 0;

	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnodes" );
	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnode_front" );
	HManagerIndexClassesOfType( mapnodehm, &r_mapnodenum, "mapnode_back" );

	printf( "r_mapnodenum: %d\n", r_mapnodenum );

	HManagerInitTypeSearchIterator( &iters[0], mapnodehm, "mapnodes" );
	HManagerInitTypeSearchIterator( &iters[1], mapnodehm, "mapnode_front" );
	HManagerInitTypeSearchIterator( &iters[2], mapnodehm, "mapnode_back" );

	for ( i = 0; i < 3; i++ )
	{
		for ( ; ( mapnode = HManagerGetNextClass( &iters[i] ) ); )
		{
			pair = FindHPair( mapnode, "index" );
			if ( !pair )
				Error( "missing 'index' in mapnode '%s'.\n", mapnode->name );
			HPairCastToInt_safe( &index, pair );

			r_mapnodes[index].self = mapnode;

			pair = FindHPair( mapnode, "plane" );
			if ( pair )
			{
				// it's a node
				plane = HManagerSearchClassName( planehm, pair->value );
				if ( !plane )
					Error( "mapnode '%s' can't find plane '%s'.\n", mapnode->name, pair->value );
				r_mapnodes[index].pl = GetClassExtra( plane );

				child = FindClassType( mapnode, "mapnode_front" );
				if ( !child )
					Error( "missing 'mapnode_front' in mapnode '%s'.\n", mapnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "mapnode '%s' missing 'index' of child '%s'.\n", mapnode->name, child->name );
				HPairCastToInt( &r_mapnodes[index].child[0], pair );


				child = FindClassType( mapnode, "mapnode_back" );
				if ( !child )
					Error( "missing 'mapnode_back' in mapnode '%s'.\n", mapnode->name );
				pair = FindHPair( child, "index" );
				if ( !pair )
					Error( "mapnode '%s' missing 'index' of child '%s'.\n", mapnode->name, child->name );
				HPairCastToInt( &r_mapnodes[index].child[1], pair );				
			}
			else
			{
				// it's a leaf
				r_mapnodes[index].visleaf = -1;	// for debug draw
				r_mapnodes[index].pl = NULL;
				r_mapnodes[index].child[0] = -1;
				r_mapnodes[index].child[1] = -1;

				// get pvs
				pair = FindHPair( mapnode, "bitpos" );
				if ( pair )
				{
					HPairCastToInt( &bitpos, pair );
					r_leafbitpos[bitpos] = index;

					pair = FindHPair( mapnode, "complex_see" );
					if ( !pair )
					{
						pair = FindHPair( mapnode, "trivial_see" );
						if ( !pair )
							Error( "missing 'trivial_see'.\n" );
					
					}
					see_buffer_size = 4096;
					HPairCastToBstring_safe( r_mapnodes[index].can_see, &see_buffer_size, pair );

					r_visleafnum++;
					r_mapnodes[index].visinfo = true;

				// get center, min, max
					pair = FindHPair( mapnode, "center" );
					if ( !pair )
						Error( "missing 'center' in mapnode '%s'.\n", mapnode->name );
					HPairCastToVec3d_safe( r_mapnodes[index].center, pair );
					
					pair = FindHPair( mapnode, "min" );
					if ( !pair )
						Error( "missing 'min' in mapnode '%s'.\n", mapnode->name );
					HPairCastToVec3d_safe( r_mapnodes[index].min, pair );
					
					pair = FindHPair( mapnode, "max" );
					if ( !pair )
						Error( "missing 'max' in mapnode '%s'.\n", mapnode->name );
					HPairCastToVec3d_safe( r_mapnodes[index].max, pair );
				}
				else
				{
					r_mapnodes[index].visinfo = false;
				}

				// get brushes
				r_mapnodes[index].brushrefnum = 0;
				r_mapnodes[index].startbrushref = r_brushrefnum;
				InitHPairSearchIterator( &iter, mapnode, "brush" );
				for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
				{
					int		brushindex;

					brush = HManagerSearchClassName( brushhm, pair->value );
					if ( !brush )
						Error( "mapnode '%s' can't find brush '%s'.\n", mapnode->name, pair->value );
					
					pair = FindHPair( brush, "index" );
					if ( !pair )
						Error( "missing 'index' in brush '%s'.\n", brush->name );	
					HPairCastToInt_safe( &brushindex, pair );
					r_brushrefs[r_brushrefnum++].brush = brushindex;
					r_brushes[brushindex].mapnode = index;	// for debug draw
					r_mapnodes[index].brushrefnum++;
				}

				// get touchleafs
				r_mapnodes[index].leafrefnum = 0;
				r_mapnodes[index].startleafref = r_leafrefnum;
				InitHPairSearchIterator( &iter, mapnode, "touchleaf" );
				for ( ; ( pair = SearchGetNextHPair( &iter ) ); )
				{
					leaf = HManagerSearchClassName( mapnodehm, pair->value );
					if ( !leaf )
						Error( "mapnode '%s' can't find brush '%s'.\n", mapnode->name, pair->value );
					
					pair = FindHPair( leaf, "index" );
					if ( !pair )
						Error( "missing 'index' in mapnode '%s'.\n", leaf->name );
					HPairCastToInt_safe( &r_leafrefs[r_leafrefnum++].mapnode, pair );
					r_mapnodes[index].leafrefnum++;
				}


			}
			
		}
	}
}

void SetupNodeBoundsRecursive( int node, fp_t *min, fp_t *max )
{
	mapnode_t		*n;

	n = &r_mapnodes[node];
	
	if ( !n->pl )
	{
		// it's a leaf
	    
		if ( n->visinfo )
		{
			// it's a visleaf, add bound box 
			Vec3dAddToBB( min, max, n->min );
			Vec3dAddToBB( min, max, n->max );			
		}
	}
	else
	{
		// it's a node
		Vec3dInitBB( n->min, n->max, 999999.9 );
		SetupNodeBoundsRecursive( n->child[0], n->min, n->max );
		SetupNodeBoundsRecursive( n->child[1], n->min, n->max );

		Vec3dAddToBB( min, max, n->min );
		Vec3dAddToBB( min, max, n->max );

//		Vec3dPrint( n->min );
//		Vec3dPrint( n->max );
	}
}

void SetupNodeBounds( void )
{
	vec3d_t		min, max;

	SetupNodeBoundsRecursive( 0, min, max );
}

void SetupNodeLeafbitsRecursive( int node, int *firstbit, int *lastbit )
{
	mapnode_t		*n;

	n = &r_mapnodes[node];

	if ( !n->pl )
	{
		// it's a leaf

		int		bitpos;
		hpair_t		*pair;

		pair = FindHPair( n->self, "bitpos" );
		if ( pair )
		{
			// it's a visleaf			
			HPairCastToInt_safe( &bitpos, pair );
			*firstbit = *lastbit = bitpos;
		}
		else
		{
			*firstbit = *lastbit = -1;
		}
	}
	else
	{
		// it's a node
		int	f1, l1;
		int	f2, l2;

		SetupNodeLeafbitsRecursive( n->child[0], &f1, &l1 );
		SetupNodeLeafbitsRecursive( n->child[1], &f2, &l2 );

		if ( f1 != -1 && f2 != -1 )
		{
			n->firstbit = f1 < f2 ? f1 : f2;
			n->lastbit = l1 > l2 ? l1 : l2;
		}
		else if ( f1 != -1 && f2 == -1 )
		{
			n->firstbit = f1;
			n->lastbit = l1;
		}
		else if ( f1 == -1 && f2 != -1 )
		{
			n->firstbit = f2;
			n->lastbit = l2;
		}
		else
		{
			n->firstbit = -1;
			n->lastbit = -1;
		}

		*firstbit = n->firstbit;
		*lastbit = n->lastbit;

//		printf( "%d %d\n", n->firstbit, n->lastbit );
	}
}

void SetupNodeLeafbits( void )
{
	int		first, last;
	SetupNodeLeafbitsRecursive( 0, &first, &last );		
}

void SetupVisleafFaces( void )
{
	int		i, j, k, l;
	brush_t		*b;
	fixpolygon_t	*p;
	int		leaf_num = 0;

	r_facenum = 0;


	for ( i = 0; i < r_mapnodenum; i++ )
	{
		if ( r_mapnodes[i].visinfo )
		{
			r_mapnodes[i].facenum = 0;
			r_mapnodes[i].startface = r_facenum;			
			// collect all own brushes
			for ( j = 0; j < r_mapnodes[i].brushrefnum; j++ )
			{
				b = &r_brushes[r_brushrefs[j+r_mapnodes[i].startbrushref].brush];
				for ( k = 0; k < b->fixpolygonnum; k++ )
				{
					p = &r_fixpolygons[r_fixpolygonrefs[k+b->startfixpolygonref].fixpolygon];
					r_faces[r_facenum].fixpolygon = p;
					r_facenum++;
					r_mapnodes[i].facenum++;
				}
			}

			// collect all faces of touch leafs, that faceing to the leaf center
			for ( j = 0; j < r_mapnodes[i].leafrefnum; j++ )
			{
				mapnode_t	*n;

				n = &r_mapnodes[r_leafrefs[r_mapnodes[i].startleafref+j].mapnode];
				for ( k = 0; k < n->brushrefnum; k++ )
				{
					b = &r_brushes[r_brushrefs[k+n->startbrushref].brush];
					for ( l = 0; l < b->fixpolygonnum; l++ )
					{
						// do polygon face the leaf center ?
						p = &r_fixpolygons[r_fixpolygonrefs[l+b->startfixpolygonref].fixpolygon];
						if ( Vec3dDotProduct( p->pl->norm, r_mapnodes[i].center ) - p->pl->dist < 0.0 )
							continue;

						r_faces[r_facenum].fixpolygon = p;
						r_facenum++;
						r_mapnodes[i].facenum++;
					}
				}
			}
			leaf_num++;
		}
	}

	printf( " %d faces in %d visleafs\n", r_facenum, leaf_num );
}

/*
  =============================================================================
  visleaf 

  =============================================================================
*/


#define MAX_POINTS	( 32000 )
#define MAX_PORTALS	( 16000 )
#define MAX_VISLEAFS	( 1024 )

int	r_pointnum = 0;
int	r_portalnum = 0;
int	r_visleafnum = 0;

vec3d_t	r_points[MAX_POINTS];
portal_t r_portals[MAX_PORTALS];
visleaf_t r_visleafs[MAX_VISLEAFS];


void CompileVisleafClass( hmanager_t *visleafhm, hmanager_t *planehm )
{
	hobj_search_iterator_t	iter;
	hobj_t		*visleaf;
	hobj_t		*plane;
	hpair_t		*pair;
	int		i, num;
			

	r_pointnum = 0;
	r_portalnum = 0;
	r_visleafnum = 0;

	InitClassSearchIterator( &iter, HManagerGetRootClass( visleafhm ), "visleaf" );
	for ( ; ( visleaf = SearchGetNextClass( &iter ) ) ; )
	{
		hobj_search_iterator_t	portaliter;		
		hobj_t		*portal;
		char		tt[256];

		pair = FindHPair( visleaf, "bitpos" );
		if ( !pair )
			Error( "missing 'bitpos' in visleaf '%s'.\n", visleaf->name );
		HPairCastToInt_safe( &r_visleafs[r_visleafnum].bitpos, pair );
		
		// set own index
		sprintf( tt, "%d", r_visleafnum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( visleaf, pair );

		r_visleafs[r_visleafnum].startportal = r_portalnum;
		r_visleafs[r_visleafnum].portalnum = 0;
		r_visleafs[r_visleafnum].count = 0;

		InitClassSearchIterator( &portaliter, visleaf, "portal" );
		for ( ; ( portal = SearchGetNextClass( &portaliter ) ) ; )
		{

			// portal plane
			pair = FindHPair( portal, "plane" );
			if ( !pair )
				Error( "missing 'plane' in portal '%s'.\n", portal->name );
			plane = HManagerSearchClassName( planehm, pair->value );
			if ( !plane )
				Error( "portal '%s' can't find plane '%s'.\n", portal->name, pair->value );
			r_portals[r_portalnum].pl = GetClassExtra( plane );
			
			// through_num
			pair = FindHPair( portal, "through_num" );
			if ( !pair )
				Error( "missing 'through_num' in portal '%s'.\n", portal->name );
			HPairCastToInt_safe( &r_portals[r_portalnum].through_num, pair );

			// complex_num
			pair = FindHPair( portal, "complex_num" );
			if ( !pair ) 
				Error( "missing 'complex_num' in portal '%s'.\n", portal->name );
			HPairCastToInt_safe( &r_portals[r_portalnum].complex_num, pair );
			

			// see_through
			pair = FindHPair( portal, "through_see" );
			if ( !pair )
			{
				r_portals[r_portalnum].see_through = NULL;
			}
			else
			{
				int		see_buffer_size;

				see_buffer_size = strlen(pair->value)/2 + 2;
				r_portals[r_portalnum].see_through = (unsigned char*) malloc( see_buffer_size );
				HPairCastToBstring_safe( r_portals[r_portalnum].see_through, &see_buffer_size, pair );				
			}

			r_portals[r_portalnum].startpoint = r_pointnum;
			r_portals[r_portalnum].pointnum = 0;
			
			pair = FindHPair( portal, "pointnum" );
			if ( !pair ) 
				Error( "missing 'pointnum' in portal '%s'.\n", portal->name );
			HPairCastToInt_safe( &num, pair );

			for ( i = 0; i < num; i++ )
			{
				sprintf( tt, "%d", i );

				pair = FindHPair( portal, tt );
				if ( !pair )
					Error( "missing point '%s' in portal '%s'.\n", tt, portal->name );
				HPairCastToVec3d_safe( r_points[r_pointnum++], pair );
				r_portals[r_portalnum].pointnum++;
			}
			r_visleafs[r_visleafnum].portalnum++;
			r_portalnum++;
		}
		r_visleafnum++;
	}

	printf( " %d visleafs\n", r_visleafnum );
	printf( " %d visportals\n", r_portalnum );
	printf( " %d visportalpoints\n", r_pointnum );


	//
	// set visleafs in mapnodes
	//

	for ( i = 0; i < r_mapnodenum; i++ )
	{
		pair = FindHPair( r_mapnodes[i].self, "visleaf" );
		if ( !pair )
			continue;
		visleaf = HManagerSearchClassName( visleafhm, pair->value );
		if ( !visleaf )
			Error( "mapnode '%s' can't find visleaf '%s'.\n", r_mapnodes[i].self->name, pair->value );

		pair = FindHPair( visleaf, "index" );
		if ( !pair )
			Error( "missing 'index' in visleaf '%s'.\n", visleaf->name );
		HPairCastToInt_safe( &r_mapnodes[i].visleaf, pair );				
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
  =============================================================================
  bsp stuff

  =============================================================================
*/

mapnode_t * FindLeafForPoint( vec3d_t point )
{
	int		node;
	mapnode_t	*n;
	fp_t		d;
	
	node = 0;

	for(;;)
	{
		n = &r_mapnodes[node];
		if ( !n->pl )
		{
			return n;
		}

		d = Vec3dDotProduct( n->pl->norm, point ) - n->pl->dist;
		if ( d >= 0 )
		{
			// go to front child
			node = n->child[0];
		}
		else
		{
			// go to back child
			node = n->child[1];
		}
	}
}




/*
  ==================================================
  visleaf

  ==================================================
*/

#define MAX_CACHED	( 2048 )
#define MAX_CACHEDP	( 4096 )
#define MAX_CACHEDV	( 8192 )
mapnode_t *r_cachedleaf = NULL;

// pvs cached brushes
int r_cachednum = 0;
brush_t *r_cached[MAX_CACHED];

// visleaf debug
int r_cachedvlnum = 0;
visleaf_t	*r_cachedvl[MAX_CACHED];

// pvs cached fixpolygons
int r_cachedpnum = 0;
fixpolygon_t	*r_cachedp[MAX_CACHEDP];

// cached vertices
int	r_cachedvnum = 0;
int	r_cachedvref[MAX_CACHEDV];
vec4d_t	r_cachedv[MAX_CACHEDV];

// faces sorted by lightmaps
int	num_map0[MAX_LIGHTPAGES];
int	num_map1[MAX_LIGHTPAGES];
int	lightsort_map0[MAX_LIGHTPAGES][1024];
int	lightsort_map1[MAX_LIGHTPAGES][1024];

// faces sorted by texmaps
int	num_tex[MAX_TEXTURES];
int	sort_tex[MAX_TEXTURES][2048];

vec3d_t	r_flatcolors[256];

bool_t	debug_disable_specular = false;
bool_t	debug_disable_diffuse = false;
bool_t  debug_enable_trans = false;

bool_t	debug_wire_only = false;
bool_t	debug_visleaf_allow = false;
bool_t	debug_draw_visleaf = false;
bool_t	debug_draw_fieldcache = false;

vec3d_t		r_origin;
matrix3_t	r_matrix;
matrix3_t	r_invmatrix;
frustum_t	r_frustum;

int		r_frame_count = 1;
int		r_tri_count = 0;


void DrawWire( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	vertex_t		*v;

	glDisable( GL_TEXTURE_2D );
	glColor3f( 1.0, 1.0, 1.0 );
	glPolygonMode( GL_FRONT, GL_FILL );
	for ( i = 0; i < r_brushnum; i++ )
	{
		b = &r_brushes[i];

		if ( b->debug_draw )
		{
			glColor3f( 1.0, 0.0, 0.0 );
		}
		else
		{
			glColor3f( 1.0, 1.0, 1.0 );
		}


		for ( j = 0; j < b->fixpolygonnum; j++ )
		{
			p = &r_fixpolygons[r_fixpolygonrefs[b->startfixpolygonref+j].fixpolygon];

//			glColor3fv( r_flatcolors[ (((int)(p))>>3) & 255 ] );
			glColor3fv( r_flatcolors[ r_fixpolygonrefs[b->startfixpolygonref+j].fixpolygon& 255 ] );
			
//			glBegin( GL_LINE_LOOP );
			glBegin( GL_TRIANGLE_FAN );

			for ( k = 0; k < p->pointnum; k++ )
			{
				v = &r_vertices[r_vertexrefs[p->startvertexref+k].vertex];
				glVertex3f( v->v[0]/16.0, -v->v[1]/16.0, v->v[2]/16.0 );
			}
			glEnd();
		}
	}
}

void DrawCachedVisleafs( void )
{
	int		i, j, k;
	visleaf_t	*visleaf;
	portal_t	*portal;

	if ( !debug_draw_visleaf )
		return

	glDisable( GL_TEXTURE_2D );
	glColor3f( 1.0, 1.0, 1.0 );
	glPolygonMode( GL_FRONT, GL_LINE );


	for ( i = 0; i < r_cachedvlnum; i++ )
	{
		visleaf = r_cachedvl[i];
		glColor3fv( r_flatcolors[((int)(visleaf)>>3)&255] );

		for ( j = 0; j < visleaf->portalnum; j++ )
		{
			portal = &r_portals[j+visleaf->startportal];
			
			glBegin( GL_LINE_LOOP );
			for ( k = 0; k < portal->pointnum; k++ )
			{
				glVertex3f( r_points[k+portal->startpoint][0]/16.0,
					    -r_points[k+portal->startpoint][1]/16.0,
					    r_points[k+portal->startpoint][2]/16.0 );
			}
			glEnd();
		}
	}
}

void DrawCached( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	vertex_t	*v;

	vec3d_t		flat[8] = { { 0, 0, 0 },
				    { 1, 0, 0 },
				    { 0, 1, 0 },
				    { 0, 0, 1 },
				    { 1, 1, 0 },
				    { 1, 0, 1 },
				    { 0, 1, 1 },
				    { 1, 1, 1 } };


//	glPolygonMode( GL_FRONT, GL_LINE );
	glPolygonMode( GL_FRONT, GL_FILL );
	for ( i = 0; i < r_cachednum; i++ )
	{
	
		b = r_cached[i];
	
		
		for ( j = 0; j < b->fixpolygonnum; j++ )
		{
			p = &r_fixpolygons[r_fixpolygonrefs[b->startfixpolygonref+j].fixpolygon];
			
//			glBegin( GL_LINE_LOOP );
			glColor3fv( r_flatcolors[ (((int)(b)/12)+j) & 255] );
			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < p->pointnum; k++ )
			{
				v = &r_vertices[r_vertexrefs[p->startvertexref+k].vertex];
				glVertex3f( v->v[0]/16.0, -v->v[1]/16.0, v->v[2]/16.0 );
			}
			glEnd();
		}
	}
}



void DrawCachedTextureMap( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	texdef_t	*td;
	vertex_t	*v;
	vec2d_t		texel;

	int		l;

//	glEnable(GL_POLYGON_SMOOTH);	
	glEnable( GL_TEXTURE_2D );

	

	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );

	if ( !debug_enable_trans )
	{
		glDisable(GL_BLEND);
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
	}
	else
	{
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 1.0, 1.0, 1.0, 0.5 );
	}

	for ( i = 0; i < MAX_TEXTURES; i++ )
	{
		if ( !num_tex[i] )
			continue;
		
		glBindTexture( GL_TEXTURE_2D, r_textures[i].texobj );
		
		for ( j = 0; j < num_tex[i]; j++ )
		{
			p = r_cachedp[sort_tex[i][j]];
				
			if ( p->texdef == -1 )
				continue;
			
			td = &r_texdefs[p->texdef];
			
			for ( l = 0; l < 1; l++ )	// time test, draw texture more than once
			{

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < p->pointnum; k++ )
			{
				v = &r_vertices[r_vertexrefs[p->startvertexref+k].vertex];
				ProjectVec3d( texel, v->v, td->projection );
				
				if ( td->projection & ProjectionType_Vecs )
				{
					vec2d_t		tmp;
					tmp[0] = texel[0];
					tmp[1] = texel[1];
					texel[0] = tmp[0]*td->vecs[0][0] + tmp[1]*td->vecs[0][1];
					texel[1] = tmp[0]*td->vecs[1][0] + tmp[1]*td->vecs[1][1];
				}

				if ( td->projection & ProjectionType_Shift )
				{
					texel[0] += td->shift[0];
					texel[1] -= td->shift[1];
				}

				texel[0] *= r_textures[td->texture].inv_width;
				texel[1] *= -r_textures[td->texture].inv_height;
				
				glTexCoord2fv( texel );			       
//				glVertex3f( v->v[0]/16.0, -v->v[1]/16.0, v->v[2]/16.0 );
				glArrayElement( v->cached );
			}
			glEnd();
			
			r_tri_count += p->pointnum-2;

			}
//			printf( "\n" );
		}
	}
}



void DrawCachedDiffuseLightmap( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	lightdef_t	*ld;
	vertex_t	*v;
	vec2d_t		texel;

	if ( debug_disable_diffuse )
		return;

//	glEnable(GL_POLYGON_SMOOTH);	
	glEnable( GL_TEXTURE_2D );
	glEnable( GL_BLEND );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);
	glColor3f( 1.0, 1.0, 1.0 );

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		if ( !num_map0[i] )
			continue;
		
		glBindTexture( GL_TEXTURE_2D, r_lightpages[i].texobj );
		
		for ( j = 0; j < num_map0[i]; j++ )
		{
			p = r_cachedp[lightsort_map0[i][j]];
			
			if ( p->lightdef == -1 )
				continue;
			ld = &r_lightdefs[p->lightdef];
			
			if ( ld->lightmapnum < 1 )
				continue;
			
//			printf( "%d X %d, %d %d: ", ld->width, ld->height, ld->xofs, ld->yofs );

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < p->pointnum; k++ )
			{
				v = &r_vertices[r_vertexrefs[p->startvertexref+k].vertex];
				ProjectVec3d( texel, v->v, ld->projection );
				
				texel[0] -= ld->shift[0];
				texel[0] *= ld->scale; // /=( ld->patchsize * 128.0 );
				texel[0] += ld->lightmaps[0].xofs2; // += ((ld->lightmaps[0].xofs)/128.0);

				texel[1] -= ld->shift[1];
				texel[1] *= ld->scale; // /= ( ld->patchsize * 128.0 );
				texel[1] += ld->lightmaps[0].yofs2; // += ((ld->lightmaps[0].yofs)/128.0);				
				
//				printf( "%f %f, ", texel[0], texel[1] );
				glTexCoord2fv( texel );
//				glTexCoord2f( 0.5, 0.5 );
			       
//				glVertex3f( v->v[0]/16.0, -v->v[1]/16.0, v->v[2]/16.0 );
//				glArrayElement( r_vertexrefs[p->startvertexref+k].vertex );
				glArrayElement( v->cached );
			}
			glEnd();

			r_tri_count += p->pointnum-2;
//			printf( "\n" );
		}
	}
}



void DrawCachedSpecularLightmap( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	lightdef_t	*ld;
	vertex_t	*v;
	vec2d_t		texel;

	if ( debug_disable_specular )
		return;

//	glEnable(GL_POLYGON_SMOOTH);	
	glEnable( GL_TEXTURE_2D );
	glEnable(GL_BLEND);
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glBlendFunc( GL_ONE, GL_ONE );
	glColor3f( 1.0, 1.0, 1.0 );


	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		if ( !num_map1[i] )
			continue;
		
		glBindTexture( GL_TEXTURE_2D, r_lightpages[i].texobj );
		
		for ( j = 0; j < num_map1[i]; j++ )
		{
			p = r_cachedp[lightsort_map1[i][j]];

			if ( p->lightdef == -1 )
				continue;
			ld = &r_lightdefs[p->lightdef];
			
			if ( ld->lightmapnum < 2 )
				continue;

//			printf( "%d X %d, %d %d: ", ld->width, ld->height, ld->xofs, ld->yofs );

			glBegin( GL_TRIANGLE_FAN );
			for ( k = 0; k < p->pointnum; k++ )
			{
				v = &r_vertices[r_vertexrefs[p->startvertexref+k].vertex];
				ProjectVec3d( texel, v->v, ld->projection );
				
				texel[0] -= ld->shift[0];
				texel[1] -= ld->shift[1];
				texel[0] *= ld->scale; // /=( ld->patchsize * 128.0 );
				texel[1] *= ld->scale; // /= ( ld->patchsize * 128.0 );

//				texel[0] /= 128.0;
//				texel[1] /= 128.0;

				texel[0] += ld->lightmaps[1].xofs2; // += ((ld->lightmaps[1].xofs)/128.0);
				texel[1] += ld->lightmaps[1].yofs2; // += ((ld->lightmaps[1].yofs)/128.0);

				
//				printf( "%f %f, ", texel[0], texel[1] );
				glTexCoord2fv( texel );
//				glTexCoord2f( 0.5, 0.5 );
			       
//				glVertex3f( v->v[0]/16.0, -v->v[1]/16.0, v->v[2]/16.0 );
//				glArrayElement( r_vertexrefs[p->startvertexref+k].vertex );
				glArrayElement( v->cached );
			}
			glEnd();

			r_tri_count += p->pointnum-2;
//			printf( "\n" );
		}
	}
}


void CachedBrushsToCachedFixpolygons( void )
{
	int		i;
	int		j;
	int		lightpage;
	int		texture;
	brush_t		*b;

	fixpolygon_t	*p;

	r_cachedpnum = 0;

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		num_map0[i] = 0;
		num_map1[i] = 0;
	}

	for ( i = 0; i < MAX_TEXTURES; i++ )
	{
		num_tex[i] = 0;
	}
	
	for ( i = 0; i < r_cachednum; i++ )
	{
		b = r_cached[i];
		for ( j = 0; j < b->fixpolygonnum; j++ )
		{
			p = r_cachedp[r_cachedpnum] = &r_fixpolygons[r_fixpolygonrefs[b->startfixpolygonref+j].fixpolygon];

			if ( Vec3dDotProduct( p->pl->norm, r_origin ) - p->pl->dist < 0.0 )
				continue;

			if ( p->texdef != -1 )
			{
				texture = r_texdefs[p->texdef].texture;
				sort_tex[texture][num_tex[texture]++] = r_cachedpnum;
			}

			if ( r_lightdefs[p->lightdef].lightmapnum >= 1 )
			{
				lightpage = r_lightdefs[p->lightdef].lightmaps[0].lightpage;
				lightsort_map0[lightpage][num_map0[lightpage]++] = r_cachedpnum;
			}
			if ( r_lightdefs[p->lightdef].lightmapnum >= 2 )
			{
				lightpage = r_lightdefs[p->lightdef].lightmaps[1].lightpage;
				lightsort_map1[lightpage][num_map1[lightpage]++] = r_cachedpnum;
			}

			r_cachedpnum++;
		}
	}
	printf( "%d cached fixpolygons\n", r_cachedpnum );

}

void SortCachedFaces( void )
{
	int		i;
	int		lightpage;
	int		texture;
	brush_t		*b;

	fixpolygon_t	*p;

	for ( i = 0; i < MAX_LIGHTPAGES; i++ )
	{
		num_map0[i] = 0;
		num_map1[i] = 0;
	}

	for ( i = 0; i < MAX_TEXTURES; i++ )
	{
		num_tex[i] = 0;
	}
	
	for ( i = 0; i < r_cachedpnum; i++ )
	{
		p = r_cachedp[i];

//		if ( Vec3dDotProduct( p->pl->norm, r_origin ) - p->pl->dist < 0.0 )
//			continue;
		
		if ( p->texdef != -1 )
		{
			texture = r_texdefs[p->texdef].texture;
			sort_tex[texture][num_tex[texture]++] = i;
		}
		
		if ( r_lightdefs[p->lightdef].lightmapnum >= 1 )
		{
			lightpage = r_lightdefs[p->lightdef].lightmaps[0].lightpage;
			lightsort_map0[lightpage][num_map0[lightpage]++] = i;
		}
		if ( r_lightdefs[p->lightdef].lightmapnum >= 2 )
		{
			lightpage = r_lightdefs[p->lightdef].lightmaps[1].lightpage;
			lightsort_map1[lightpage][num_map1[lightpage]++] = i;
		}
	}
//	printf( "%d cached fixpolygons\n", r_cachedpnum );

}

static int	culled_faces;
static int	clipped_faces;
static int	passed_faces;

void CacheVisibleFaces( unsigned char *vis )
{
	int		i, j, k;
	mapnode_t	*n;
	face_t		*f;

	r_cachedpnum = 0;
	r_cachedvnum = 0;
	for ( i = 0; i < r_visleafnum; i++ )
	{
		if ( vis[i>>3] & (1<<(i&7)) )
		{
			frustumClipMask		clip;		// stored by CullVisleafs
			n = &r_mapnodes[r_leafbitpos[i]];
			clip = n->clip;

			for ( j = 0; j < n->facenum; j++ )
			{
				f = &r_faces[j+n->startface];

				// special sky hack
				if ( r_textures[r_texdefs[f->fixpolygon->texdef].texture].is_sky )
					continue;

				if ( Vec3dDotProduct( f->fixpolygon->pl->norm, r_origin ) - f->fixpolygon->pl->dist < 0.0 )
					continue;

				if ( f->fixpolygon->count == r_frame_count )
					continue;
				f->fixpolygon->count = r_frame_count;

				if ( clip == Frustum_cnone )
				{
					R_FaceSetup_NoClip( f );
					passed_faces++;
				}
				else
				{
					frustumClipMask		clip2;	
					clip2 = R_FrustumTestDedicatedPlanes( &r_frustum, f->fixpolygon, clip );
					if ( clip2 != Frustum_call )
					{
						R_FaceSetup_NoClip( f );	
							clipped_faces++;
					}
					else
					{
						culled_faces++;
					}
				}


#if 0			
				// cache vertices
				for ( k = 0; k < f->fixpolygon->pointnum; k++ )
				{
					vertex_t	*v;
					v = &r_vertices[r_vertexrefs[f->fixpolygon->startvertexref+k].vertex];
					if ( v->count == count )
						continue;
					v->count = count;
					r_cachedvref[r_cachedvnum] = r_vertexrefs[f->fixpolygon->startvertexref+k].vertex;
					v->cached = r_cachedvnum++;
				}				
				r_cachedp[r_cachedpnum] = f->fixpolygon;
				r_cachedpnum++;
#endif
			}
		}
	}

}

void CullVisleafs( unsigned char *vis )
{
	int		i;
	mapnode_t	*n;
	int		pv_leafs = 0;
	int		culled_leafs = 0;
	frustumClipMask		clip;
	int		clip_num = 0;
	int		noclip_num = 0;

	for ( i = 0; i < r_visleafnum; i++ )
	{
		if ( vis[i>>3] & (1<<(i&7)) )
		{
			n = &r_mapnodes[r_leafbitpos[i]];
			clip = R_FrustumTestBB( &r_frustum, n->min, n->max);
			n->clip = clip;		// can be reused
			if ( clip == Frustum_call )
			{
				culled_leafs++;
				vis[i>>3] &= ~(1<<(i&7));
			}
			else if ( clip == Frustum_cnone )
			{
				noclip_num++;
			}
			else
			{
				clip_num++;
			}
			pv_leafs++;
		}
	}
	
	printf( " %d leafs culled of %d potential visible of %d visleafs (%d, %d)\n", culled_leafs, pv_leafs, r_visleafnum, noclip_num, clip_num );                                            
}

void DumpVisleaf( int visleaf )
{
	int		i;
	visleaf_t	*vl;
	portal_t	*pt;

	if ( visleaf == -1 )
	{
		printf( "no visleaf ?\n" );
		return;
	}

	vl = &r_visleafs[visleaf];

	for ( i = 0; i < vl->portalnum; i++ )
	{
		pt = &r_portals[i+vl->startportal];
		printf( "c%d-t%d ", pt->complex_num, pt->through_num );
	}
	printf( "\n" );
}

bool_t PVS_UpdateCache( vec3d_t origin )
{
	mapnode_t	*leaf;
	unsigned char	see[4096];

	leaf = FindLeafForPoint( origin );

	if ( leaf->visinfo )
	{
//		DumpVisleaf( leaf->visleaf );

		memcpy( see, leaf->can_see, r_visleafnum/8+1 );
		CullVisleafs( see );

#ifndef __NO_VISLEAF
		if ( debug_disable_specular )
			R_HVIS_CullVisleafs( leaf->visleaf, see, &r_frustum );
#endif

		culled_faces = 0;
		clipped_faces = 0;
		passed_faces = 0;
		CacheVisibleFaces( see );
		printf( " %d culled, %d clipped, %d passed\n", culled_faces, clipped_faces, passed_faces );

//		SortCachedFaces();
		return true;
	}
	return false;

	if ( r_cachedleaf == leaf )
	{
		printf( "( draw data cached )\n" );		
		return true;
	}

	if ( !leaf->visinfo )
	{
		printf( "( no vis info )\n" );
		return false;
	}	

	r_cachedleaf = leaf;

	CacheVisibleFaces( leaf->can_see );
	SortCachedFaces();
	printf( "( cached %d faces, %d vertices )\n", r_cachedpnum, r_cachedvnum );
	
	return true;
}


/*
  ==================================================
  load map

  ==================================================
*/
void LoadMap( void )
{
	char		*in_vertex_name;
	char		*in_brush_name;
	char		*in_mapnode_name;
	char		*in_plane_name;
	char		*in_lightdef_name;
	char		*in_texdef_name;
	char		*in_texture_name;
	char		*in_visleaf_name;		// for debug draw of visleaf mapnodes

	// field stuff
	char		*in_fplane_name;
	char		*in_fnode_name;
	char		*in_fmap_name;

	hmanager_t	*planehm;
	hmanager_t	*vertexhm;
	hmanager_t	*brushhm;
	hmanager_t	*mapnodehm;
	hmanager_t	*lightdefhm;
	hmanager_t	*texdefhm;
	hmanager_t	*texturehm;
	hmanager_t	*visleafhm;

	tokenstream_t	*ts;

	printf( "===== render3 - pvs test engine =====\n" );

	in_vertex_name = "_fface_vertex.hobj";
	in_brush_name = "_fface_bspbrush.hobj";
	in_mapnode_name = "_mapnode.hobj";
	in_plane_name = "_plane.hobj";
	in_lightdef_name = "_light_lightdef.hobj";
	in_texdef_name = "_texdef.hobj";
	in_texture_name = "_texture.hobj";
	in_visleaf_name = "_pvsout_visleaf.hobj";

	in_fplane_name = "./field/_plane.hobj";
	in_fnode_name = "./field/_fieldtree.hobj";
	in_fmap_name = "_fieldmap.bin";
	
	printf( " default vertex class: %s\n", in_vertex_name );
	printf( " default brush class: %s\n", in_brush_name );
	printf( " default mapnode class: %s\n", in_mapnode_name );
	printf( " default plane class: %s\n", in_plane_name );
	printf( " default lightdef class: %s\n", in_lightdef_name );
	printf( " default texdef class: %s\n", in_texdef_name );
	printf( " default texture class: %s\n", in_texture_name );
	printf( " default visleaf class: %s\n", in_visleaf_name );
	printf( " default field plane class: %s\n", in_fplane_name );
	printf( " default field node class: %s\n", in_fnode_name );

	printf( "load classes ...\n" );

	planehm = ReadPlaneClass( in_plane_name );

	// vertex class
	ts = BeginTokenStream( in_vertex_name );
	if ( !ts )
		Error( "can't open vertex class.\n" );
	vertexhm = NewHManager();
	HManagerSetRootClass( vertexhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( vertexhm );	

	
	// brush class
	ts = BeginTokenStream( in_brush_name ); 
	if ( !ts )
		Error( "can't open bspbrush class.\n" );
	brushhm = NewHManager();
	HManagerSetRootClass( brushhm, ReadClass( ts ) );
	EndTokenStream( ts );
	HManagerRebuildHash( brushhm );	

	// mapnode class
	ts = BeginTokenStream( in_mapnode_name );
	if ( !ts )
	{
		debug_wire_only = true;
		printf( "can't open mapnode class.\n" );
	}
	else
	{
		mapnodehm = NewHManager();
		HManagerSetRootClass( mapnodehm, ReadClass( ts ) );
		EndTokenStream( ts );
		HManagerRebuildHash( mapnodehm );
	}

	lightdefhm = NewHManagerLoadClass( in_lightdef_name );
	if ( !lightdefhm )
	{
		debug_wire_only = true;
		printf( "can't open lightdef class.\n" );	
	}


	texdefhm = NewHManagerLoadClass( in_texdef_name );
	if ( !texdefhm )
	{
		debug_wire_only = true;
		printf( "can't open texdef class.\n" );
	}

	texturehm = NewHManagerLoadClass( in_texture_name );
	if ( !texturehm )
	{
		debug_wire_only = true;
		printf( "can't open texture class.\n" );
	}

	visleafhm = NewHManagerLoadClass( in_visleaf_name );
	if ( !visleafhm )
	{
		printf( "can't open visleaf class. no visleaf debug draw.\n" );
		debug_visleaf_allow = false;
	}
	else
	{
		debug_visleaf_allow = true;
	}

	printf( "ok\n" );

	printf( "compile classes ...\n" );


	if ( !debug_wire_only )
	{
		CompileTextureClass( texturehm );
		CompileTexdefClass( texdefhm, texturehm );
		
		CompileLightdefClass( lightdefhm );
		CompileVertexClass( vertexhm );
		CompileBrushClass( brushhm, vertexhm, planehm, lightdefhm, texdefhm );
		CompileMapnodeClass( mapnodehm, brushhm, vertexhm, planehm );
	}
	else
	{
		CompileVertexClass( vertexhm );
		CompileBrushClass_debug( brushhm, vertexhm, planehm );
	}

	if ( debug_visleaf_allow )
	{
		CompileVisleafClass( visleafhm, planehm );
	}
	else
	{
		Error( "missing visleaf class for rt_portal_pvs.\n" );
	}

//	SetupField( in_fplane_name, in_fnode_name );
//	SetupFieldMap( in_fmap_name );

	SetupVisleafFaces();
	SetupNodeBounds();
	SetupNodeLeafbits();
}


/*
  =============================================================================
  render

  =============================================================================
*/

/*
  ==================================================
  

  ==================================================
*/




/*
  ==================================================
  setup opengl & glut 

  ==================================================
*/


int	r_argc;
char	**r_argv;

fp_t		r_speed = 0;
// r_origin moved up for backface cull
fp_t	r_yaw = 0.0;
fp_t	r_roll = 0.0;
fp_t	r_pitch = 0.0;

vec3d_t	r_delta;

bool_t	debug_lightpage = false;
int	debug_lightpagenum = 0;
bool_t	debug_time_test = false;
fp_t		r_frame_msec;

bool_t	update_smoke_pos = false;

int	screen_dump_num = 0;
bool_t	do_screen_dump = false;

int	debug_lod = 3;
bool_t	debug_lod_update = false;

bool_t	do_trace_test = 0xffffffff;	// hack: can't use true (=1) cause inv is 0xfffffffe ( uargh! ) which is also true
bool_t	do_draw_sky = false;

int		glut_win;

bool_t	key_pressmap[256];


void GlutKeyPress( unsigned char key, int x, int y )
{
	key_pressmap[key] = true;
}

void GlutKeyRelease( unsigned char key, int x, int y )
{
	key_pressmap[key] = false;
}

void HandlePressedKeys(void )
{

//	c = cos( r_pitch * (M_PI/180.0) ) * 10.0;
//	s = sin( r_pitch * (M_PI/180.0) ) * 10.0;

	if ( key_pressmap[27] )
	{
		glutDestroyWindow( glut_win );	       
		exit(-1);
	}

	if ( key_pressmap['w'] )
	{
		r_speed+=5.0; //r_speed*2.5;
		if ( r_speed > 50.0 )
			r_speed = 50.0;
	}

	if ( key_pressmap['z'] )
	{
		r_speed-=5.0;
		if ( r_speed < -50.0 )
			r_speed = -50.0;
	}

	if ( key_pressmap['j'] )
	{
		r_speed=2.0; //r_speed*2.5;
	}

	if ( key_pressmap['n'] )
	{
		r_speed-=2.0;
	}


	if ( key_pressmap['e'] )
	{
		r_delta[1] = 5.0;
	}
	if ( key_pressmap['d'] )
	{
		r_delta[1] = -5.0;
	}
	if ( key_pressmap['x'] )
	{
		r_yaw -= 5.0;
		if ( r_yaw < 0.0 )
			r_yaw+=360.0;
	}
	if ( key_pressmap['c'] )
	{
		r_yaw += 5.0;
		if ( r_yaw > 360.0 )
			r_yaw-=360.0;
	}

	if ( key_pressmap['f'] )
	{
		r_pitch -= 5.0;
		if ( r_pitch < 0.0 )
			r_pitch+=360.0;
	}
	if ( key_pressmap['r'] )
	{
		r_pitch += 5.0;
		if ( r_pitch > 360.0 )
			r_pitch-=360.0;
	}

	if ( key_pressmap['a'] )
	{
		r_yaw -= 5.0;
		if ( r_yaw < 0.0 )
			r_yaw+=360.0;
	}
	if ( key_pressmap['s'] )
	{
		r_yaw += 5.0;
		if ( r_yaw > 360.0 )
			r_yaw-=360.0;
	}

	if ( key_pressmap['l'] )
	{
		debug_lightpage = ~debug_lightpage;
		_Usleep( 300000 );
	}
	if ( key_pressmap['('] )
	{
		debug_lightpagenum--;
		_Usleep( 300000 );
	}
	if ( key_pressmap[')'] )
	{
		debug_lightpagenum++;       
		_Usleep( 300000 );
	}

	if ( key_pressmap['o'] )
	{
		debug_disable_specular = ~debug_disable_specular;
		_Usleep( 300000 );
	}
	if ( key_pressmap['p'] )
	{
		debug_disable_diffuse = ~debug_disable_diffuse;
		_Usleep( 300000 );
	}
	if ( key_pressmap['t'] )
	{
		debug_enable_trans = ~debug_enable_trans;
		_Usleep( 300000 );
	}
	if ( key_pressmap['i'] )
	{
		debug_draw_visleaf = ~debug_draw_visleaf;
		_Usleep( 300000 );
	}
	if ( key_pressmap['h'] )
	{
		debug_draw_fieldcache = ~debug_draw_fieldcache;
		_Usleep( 300000 );
	}	

	if ( key_pressmap['m'] )
	{	
		debug_time_test = true;
		_Usleep( 300000 );
	}
	     

	if ( key_pressmap['k'] )
	{
		update_smoke_pos = true;
	}

	if ( key_pressmap['/'] )
	{
		do_screen_dump = true;
	}

	if ( key_pressmap['-'] )
	{
		debug_lod-=2;
		if ( debug_lod < 3 )
			debug_lod = 3;
		debug_lod_update = true;
		_Usleep( 300000 );
	}
	
	if ( key_pressmap['='] )
	{
		debug_lod+=2;
		if ( debug_lod > 17 )
			debug_lod = 17;
		debug_lod_update = true;
		_Usleep( 300000 );
	}

	if ( key_pressmap['\\'] )
	{
		do_trace_test = ~do_trace_test;
		_Usleep( 300000 );
	}

	if ( key_pressmap['1'] )
	{
		do_draw_sky = ~do_draw_sky;
		_Usleep( 300000 );
	}
}

void GlutResize( int width, int height )
{
	glutPostRedisplay();
}


int GetMilliSec( void )
{
#ifndef __WIN32__
	int		msec;
	struct timeval	tv;

	gettimeofday( &tv, NULL );
	msec = tv.tv_usec / 1000;
	msec+= tv.tv_sec * 1000;
	
	return msec;
#else
	return 0;
#endif
}

void GlutDisplay( void )
{
	int		i, j, k;
	brush_t		*b;
	fixpolygon_t	*p;
	vertex_t		*v;

	int		time1, time2;

	time1 = GetMilliSec();
	
	glClearDepth( 0.0 );
	glClearColor( 0.3, 0.3, 0.3, 0.0);
	glClearColor( 0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//	glPolygonMode( GL_FRONT, GL_LINE );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

	if ( do_draw_sky )
	{
		R_RenderSkyBox();
	}

	R_BeginFaceSetup();
	
	if ( !debug_wire_only )
	{

#if 0
		// setup vertex array
		glEnableClientState( GL_VERTEX_ARRAY );
		glVertexPointer( 4, GL_FLOAT, 0, r_cachedv );
#ifdef GL_LOCK_ARRAYS_EXT
		glLockArraysEXT( 0, r_vertexnum );
#endif
#endif


#if 0
		PVStest( r_origin );
#else
		if ( PVS_UpdateCache( r_origin ) )
		{

#if 0
			// setup vertices
			for ( i = 0; i < r_cachedvnum; i++ )
			{	      
				CalcVertex( r_cachedv[i], r_scls[r_cachedvref[i]] );
			}

			DrawCachedTextureMap();
			DrawCachedDiffuseLightmap();
			DrawCachedSpecularLightmap();
//			DrawCachedVisleafs();	
#endif
		}
		else
		{
//			DrawWire();
		}
#endif


#ifdef  GL_LOCK_ARRAYS_EXT
		glUnlockArraysEXT();
#endif
//		glDisableClientState( GL_VERTEX_ARRAY );
		

	}
	else
	{
//		DrawWire();
	}

	R_EndFaceSetup();
	R_DumpFaceSetup();

	if ( !debug_enable_trans )
	{
		glDisable(GL_BLEND);
		glColor4f( 1.0, 1.0, 1.0, 1.0 );
	}
	else
	{
		glEnable( GL_BLEND );
		glBlendFunc( GL_SRC_ALPHA,  GL_ONE_MINUS_SRC_ALPHA );
		glColor4f( 1.0, 1.0, 1.0, 0.5 );
	}


	//
	// faces
	//

	R_FS_RasterizeTMapsBfr_tmap0();
	R_FS_RasterizeTMapsBfr_lmap0();
	R_FS_RasterizeTMapsBfr_lmap1();
//	R_FS_RasterizeTMapsBfr_mt_tmap0_lmap0();

	//
	// curved surfaces
	//

	if ( debug_lod_update )
	{
		R_CSurfaceSetLOD( debug_lod );
		debug_lod_update = false;
	}
	for ( i = 0; i < r_csurfacedefnum; i++ )
	{
		R_CSurfaceSetup( &r_csurfacedefs[i] );	
	}
		   

	if ( debug_draw_fieldcache )
		DrawCachedFieldCells();


//	R_ModelTest( r_origin, update_smoke_pos );
//	RunParticle();
//	FieldTest( r_origin );

//	VolumeTest( r_origin );
	R_VolumeCacheTest( r_origin, update_smoke_pos );
	update_smoke_pos = 0;

	if ( debug_lightpage )
		LightPage_DebugDraw( debug_lightpagenum );

	glFlush();
	glutSwapBuffers();	

	time2 = GetMilliSec();
	r_frame_msec = time2-time1;
//	fprintf( stderr, "f/s: %f ", 1000.0/(time2-time1) );
	
	if ( do_screen_dump )
	{
		char		tt[256];

		sprintf( tt, "dump%d.tga", screen_dump_num++ );
		R_ScreenDump( tt );
		do_screen_dump = false;
	}

	r_frame_count++;
}


void RunTimeTest( void )
{
	int		i;
	fp_t		msec;

	msec = 0;
	r_tri_count = 0;
	for ( i = 0; i < 360; i++ )
	{
		r_yaw+=1.0;
		if ( r_yaw > 360.0 )
			r_yaw-=360.0;

		Matrix3SetupRotate( r_matrix, r_roll*DEG2RAD, r_pitch*DEG2RAD, r_yaw*DEG2RAD );
		Matrix3Transpose( r_invmatrix, r_matrix );
		
		R_InitBaseFrustum( &r_frustum, 64.0, 64.0, 32.0 );
		R_FrustumRotate( &r_frustum, r_invmatrix );
		R_FrustumTranslate( &r_frustum, r_origin );
		R_FrustumCalcPlanes( &r_frustum );
		
		GlutDisplay();
		msec+=r_frame_msec;
	}

	printf( "timetest: %f msecs, %f f/s, %d tris, %f tris/sec\n", msec, 1000.0/(msec/360.0), r_tri_count, (r_tri_count/(msec/1000.0) ) );
	debug_time_test = false;
}

void TraceTest( vec3d_t pos, vec3d_t trypos )
{
	int		i;
	g_trace_t	*tr;

	vec3d_t		min;
	vec3d_t		max;

	vec3d_t		trymin;
	vec3d_t		trymax;
	
	fp_t		dist;
	vec3d_t		norm;
	vec3d_t		p1, p2, vec;
    

	for ( i = 0; i < 3; i++ )
	{
		min[i] = pos[i] - 16.0;
		max[i] = pos[i] + 16.0;

		trymin[i] = trypos[i] - 16.0;
		trymax[i] = trypos[i] + 16.0;
	}

	tr = G_TraceBoundBox( a_map->blockmap, min, max, trymin, trymax );	
	
	if ( tr->plane )
	{
		Vec3dCopy( norm, tr->plane->norm );
		dist = tr->plane->dist;

		Vec3dProjectOnPlane( p1, norm, dist, min );
		Vec3dProjectOnPlane( p2, norm, dist, trymin );

		Vec3dSub( vec, p2, p1 );

		Vec3dAdd( trymin, vec, min );
		Vec3dAdd( trymax, vec, max );

		tr = G_TraceBoundBox( a_map->blockmap, min, max, trymin, trymax );
		if ( tr->plane )                                                
                {			
			printf( "bounce again\n" );
			return;
		}
	}

	Vec3dCopy( min, trymin );
	Vec3dCopy( max, trymax );

	Vec3dAdd( vec, min, max );
	Vec3dScale( pos, 0.5, vec );		
}

void VolumeTest( vec3d_t pos )
{

	ivec3d_t		cpos;
	ccluster_t	*cc;

	if ( !a_map->volume_ccmap3 || !a_map->field_ccmap3 )
		return;

	IVec3dRint( cpos, pos );
	G_Vec3dToCCMap3Units( a_map->volume_ccmap3, cpos, cpos );
	
	cc = G_CCMap3FindCCluster( a_map->volume_ccmap3, cpos );
	if ( cc )
	{
		printf( "found volumetric light data\n" );
		VolumetricDrawTest( cc, pos );
	}
}

matrix3_t	sky_matrix;

void GlutIdle( void )
{
	fp_t	c, s;
	vec3d_t		trypos;

	Vec3dInit( r_delta, 0, 0, 0 );

	HandlePressedKeys();

	c = cos( r_yaw * (M_PI/180.0) ) * r_speed;
	s = sin( r_yaw * (M_PI/180.0) ) * r_speed;
	trypos[2] = r_origin[2] + c;
	trypos[1] = r_origin[1] + r_delta[1];
	trypos[0] = r_origin[0] + s;	


	if ( do_trace_test )
	{
		TraceTest( r_origin, trypos );
	}
	else
	{
		Vec3dCopy( r_origin, trypos );
	}

	r_speed *= 0.9;
	if ( fabs(r_speed) < 0.1 )
		r_speed = 0.0;

	if ( debug_time_test )
	{
		RunTimeTest();
		goto end;
	}


//	Vec3dCopy( r_origin, trypos );
	

	if ( debug_enable_trans )
	{
		glDisable(GL_DEPTH_TEST);
	}
	else
	{
		glEnable(GL_DEPTH_TEST);
	}

#if 0
	glMatrixMode( GL_MODELVIEW );
       	glLoadIdentity();
	glRotatef( r_yaw, 0.0 ,0.0, 1.0 );
	glRotatef( r_roll, 1.0 ,0.0, 0.0 );
	glRotatef( r_pitch, 0.0 ,1.0, 0.0 );


	glTranslatef( -(r_origin[0]/16.0), (r_origin[1]/16.0), -(r_origin[2]/16.0) );
#endif

	Matrix3SetupRotate( r_matrix, r_roll*DEG2RAD, r_pitch*DEG2RAD, r_yaw*DEG2RAD );
	Matrix3Transpose( r_invmatrix, r_matrix );
	Matrix3SetupRotate( sky_matrix, r_roll*DEG2RAD, r_pitch*DEG2RAD, (r_yaw+180)*DEG2RAD );

	R_InitBaseFrustum( &r_frustum, 64.0, 64.0, 32.0 );
	R_FrustumRotate( &r_frustum, r_invmatrix );
	R_FrustumTranslate( &r_frustum, r_origin );
	R_FrustumCalcPlanes( &r_frustum );
  
	GlutDisplay();
end:
	_Usleep( 10000 );
}


void R_SetupGLUT( void )
{
	glutInit( &r_argc, r_argv );
	glutInitDisplayMode( GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH );
	glutInitWindowSize( SIZE_X, SIZE_Y );
	glut_win = glutCreateWindow( "render3" );


	R_InitGL();
	{
		int		i;
		// init flat colors
		for ( i = 0; i < 256; i++ )
			Vec3dInit( r_flatcolors[i], RND, RND, RND );
	}
	
	Vec3dInit( r_origin, 0.0, 0.0, 0.0 );
//	Vec3dInit( r_origin, 208, 64, -576 );
}

void R_RunGLUT( void )
{
	glutDisplayFunc( GlutDisplay );
	glutReshapeFunc( GlutResize );
	glutKeyboardFunc( GlutKeyPress );
	glutKeyboardUpFunc( GlutKeyRelease );
	glutIdleFunc( GlutIdle );
	
	glutMainLoop();				
}


void GlutMain( void )
{
	glutInit( &r_argc, r_argv );
	glutInitDisplayMode( GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH );
	glutInitWindowSize( SIZE_X, SIZE_Y );
	glut_win = glutCreateWindow( "render3" );

	R_InitGL();
//	LoadMap();
	{
		int		i;
		// init flat colors
		for ( i = 0; i < 256; i++ )
			Vec3dInit( r_flatcolors[i], RND, RND, RND );
	}
	
	Vec3dInit( r_origin, 0.0, 0.0, 0.0 );



	glutDisplayFunc( GlutDisplay );
	glutReshapeFunc( GlutResize );
	glutKeyboardFunc( GlutKeyPress );
	glutKeyboardUpFunc( GlutKeyRelease );
	glutIdleFunc( GlutIdle );
	
	glutMainLoop();		
}

  

int r_main( int argc, char *argv[] )
{
	printf( "===== render3 - pvs test engine =====\n" );
	r_argc = argc;
	r_argv = argv;

//	GlutMain();
}
