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



// epoly.c

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
                                                                                

hobj_t * BuildPolygons( hmanager_t *brushhm )
{
	hobj_search_iterator_t  brushiter;
	hobj_search_iterator_t  surfiter;
	hobj_search_iterator_t  polyiter;

	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*poly;

	hobj_t		*poly_root;
	hobj_t		*poly_cpy;

	hpair_t		*pair;
	char		tt[256];

	poly_root = NewClass( "polygons", "polygons0" );
	
	InitClassSearchIterator( &brushiter, HManagerGetRootClass( brushhm ), "bspbrush" );
	
	for ( ; ( brush = SearchGetNextClass( &brushiter ) ) ; )
	{
		InitClassSearchIterator( &surfiter, brush, "surface" );
		
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ) ; )
		{
			InitClassSearchIterator( &polyiter, surface, "polygon" );

			for ( ; ( poly = SearchGetNextClass( &polyiter ) ) ; )
			{
				poly_cpy = DeepCopyClass( poly );

				// add surface clsref
				pair = NewHPair2( "ref", "surface", surface->name );
				InsertHPair( poly_cpy, pair );

				// add plane clsref
				pair = FindHPair( surface, "plane" );
				if ( !pair )
					Error( "missing 'plane'\n" );
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( poly_cpy, pair );

				// add texdef clsref
				pair = FindHPair( surface, "texdef" );
				if ( !pair )
				{
					printf( "missing 'texdef' in polygon '%s', ignore\n", poly->name );
					continue;
				}
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( poly_cpy, pair );				
					
				InsertClass( poly_root, poly_cpy );
			}
		}
	}

	return poly_root;
}

int main( int argc, char *argv[] )
{
	char	*in_brush_name;
	char	*out_poly_name;

	hmanager_t	*brushhm;
	FILE		*h;
	hobj_t		*poly_root;

	printf( "===== epoly - keep only the polygons of a brush class =====\n" );

	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-i" );
	out_poly_name = GetCmdOpt2( "-o" );

	if ( !in_brush_name )
	{
		Error( "no input brush class\n" );
	}

	if ( !out_poly_name )
	{
		Error( "no output poly class\n" );
	}

	brushhm = NewHManagerLoadClass( in_brush_name );
	if ( !brushhm )
		Error( "class load faield\n" );

	poly_root = BuildPolygons( brushhm );
	
	h = fopen( out_poly_name, "w" );
	WriteClass( poly_root, h );
	fclose( h );
}
