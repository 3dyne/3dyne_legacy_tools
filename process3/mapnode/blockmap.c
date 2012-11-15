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



// blockmap.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "cmdpars.h"                                                            
#include "wire.h"                                                               
#include "lib_token.h"                                                          
#include "lib_error.h"                                                          
#include "lib_math.h"                                                           
#include "lib_poly.h"                                                           
#include "lib_unique.h"                                                         
#include "lib_hobj.h"                                                           

/*
  ==============================
  CalcBrushBoundBox

  ==============================
*/

#define MAX_BRUSH_SURFACES		( 256 )

void CalcBrushBoundBox( hmanager_t *brushhm, hmanager_t *plhm, hmanager_t *tdhm, hmanager_t *txhm )
{
	hobj_search_iterator_t  brushiter;
	hobj_search_iterator_t  surfiter;
	hobj_search_iterator_t  polyiter;

	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*plane;
	hobj_t		*texdef;
	hobj_t		*texture;
	hobj_t		*poly;	

	hpair_t		*pair;

	vec3d_t		min, max;
	bool_t		has_polys;

	InitClassSearchIterator( &brushiter, HManagerGetRootClass( brushhm ), "bspbrush" );
	
	for ( ; ( brush = SearchGetNextClass( &brushiter ) ) ; )
	{
		int		i, j;
		int		num_plane;
		hobj_t		       *surfs[MAX_BRUSH_SURFACES];
		vec3d_t			norms[MAX_BRUSH_SURFACES];
		fp_t			dists[MAX_BRUSH_SURFACES];
		polygon_t	       *polys[MAX_BRUSH_SURFACES];

		//
		// build all polygons of the brush
		//

		num_plane = 0;
		InitClassSearchIterator( &surfiter, brush, "surface" );	       		
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ) ; )
		{	
			plane = EasyLookupClsref( surface, "plane", plhm );

			if ( num_plane >= MAX_BRUSH_SURFACES )
			{
				Error( "reached MAX_BRUSH_SURFACES\n" );
			}

			surfs[num_plane] = surface;

			EasyFindVec3d( norms[num_plane], plane, "norm" );
			EasyFindFloat( &dists[num_plane], plane, "dist" );

			polys[num_plane] = BasePolygonForPlane( norms[num_plane], dists[num_plane] );
			num_plane++;
		}


		//
		// clip polys by all other planes
		//

		Vec3dInitBB( min, max, 999999.9 );		

		// clip polys[i] by all planes[j]
		for ( i = 0; i < num_plane; i++ )
		{
			for ( j = 0; j < num_plane; j++ )
			{
				if ( i == j )
				{
					continue;
				}
				
				if ( polys[i] )
				{
					ClipPolygonInPlace( &polys[i], norms[j], dists[j] );	
				}
			}

			if ( !polys[i] )
			{
				printf( "WARNING: clipped polygon of surface '%s' of brush '%s' away\n", surfs[i]->name, brush->name );
			}
			else
			{
//				PolygonAddToBB( min, max, polys[i] );

// 2012-10-28 mcb: where has PolygonAddToBB gone? Can not find it anywhere in the old source code.
				int k;
				for ( k = 0; k < polys[i]->pointnum; k++ ) {
					Vec3dAddToBB( min, max, polys[i]->p[k] );
				}

			}
		}


		InitClassSearchIterator( &surfiter, brush, "surface" );	       		
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ) ; )
		{
#if 0
			InitClassSearchIterator( &polyiter, surface, "polygon" );
		
			has_polys = false;
			for ( ; ( poly = SearchGetNextClass( &polyiter ) ) ; )
			{
				int		i;
				int		pointnum;
				vec3d_t		p;
				char		tt[256];

				has_polys = true;
				EasyFindInt( &pointnum, poly, "num" );
				
				for ( i = 0; i < pointnum; i++ )
				{
					sprintf( tt, "%d", i );
					EasyFindVec3d( p, poly, tt );
					Vec3dAddToBB( min, max, p );
				}				
			} 

			

			if ( !has_polys )
			{
				EasyNewInt( surface, "remove", 1 );
			}
#endif
			
			//
			// check for special surfaces ( e.g. sky )
			//
			if ( FindHPair( surface, "texdef" ) )
			{
				texdef = EasyLookupClsref( surface, "texdef", tdhm ); 

				if ( FindHPair( texdef, "texture" ) )
				{
					texture = EasyLookupClsref( texdef, "texture", txhm ); 
					
					pair = FindHPair( texture, "ident" );
					if ( !pair )
					{						
						Error( "missing key 'ident'\n" );
					}
					
					if ( strstr( pair->value, "sky" ) )
					{
						EasyNewInt( surface, "surf_sky", 1 );
					}
				}
			}
			
			HManagerRemoveAndDestroyAllClassesOfType( brushhm, surface, "polygon" );
			RemoveAndDestroyAllHPairsOfKey( surface, "content" );
			RemoveAndDestroyAllHPairsOfKey( surface, "texdef" );
		}

	restart_list:
		InitClassSearchIterator( &surfiter, brush, "surface" );	       		
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ) ; )
		{
			hpair_t		*pair;
			
			pair = FindHPair( surface, "remove" );
			if ( pair )
			{
				RemoveClass2( surface );
				goto restart_list;
			}
		}

		EasyNewVec3d( brush, "min", min );
		EasyNewVec3d( brush, "max", max );

		RemoveAndDestroyAllHPairsOfKey( brush, "original" );
		RemoveAndDestroyAllHPairsOfKey( brush, "content" );
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

	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	return hm;
}


int main( int argc, char *argv[] )
{
	char	*in_brush_name;
	char	*out_brush_name;

	char	*in_plane_name;
	char	*in_texdef_name;
	char	*in_texture_name;

	hmanager_t	*brushhm;
	hmanager_t	*plhm;
	hmanager_t	*tdhm;
	hmanager_t	*txhm;
	
	FILE		*h;

	printf( "===== blockmap - prepare bspbrush class for use as blockmap input =====\n" );

	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-i" );
	out_brush_name = GetCmdOpt2( "-o" );

	in_plane_name = GetCmdOpt2( "-pl" );
	in_texdef_name = GetCmdOpt2( "-td" ); 
	in_texture_name = GetCmdOpt2( "-tx" );

	if ( !in_brush_name )
	{
		Error( "no input brush class\n" );
	}
	
	if ( !out_brush_name )
	{
		Error( "no output brush class\n" );
	}

	if ( !in_plane_name )
	{
		Error( "no input plane class\n" );
	}

	if ( !in_texdef_name )
	{
		Error( "no input texdef class\n" );
	}

	if ( !in_texture_name )
	{
		Error( "no input texture class\n" );
	}
	
	plhm = ReadPlaneClass( in_plane_name );

	tdhm = NewHManagerLoadClass( in_texdef_name );
	txhm = NewHManagerLoadClass( in_texture_name );	

	brushhm = NewHManagerLoadClass( in_brush_name );
	if ( !brushhm )
		Error( "class load faield\n" );

	CalcBrushBoundBox( brushhm, plhm, tdhm, txhm );

	printf( "write output brush class ...\n" );
	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "failed\n" );
	WriteClass( HManagerGetRootClass( brushhm ), h );
	fclose( h );
	
}
