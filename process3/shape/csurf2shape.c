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



// csurf2shape.c

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
  CSurf2Shape

  ==============================
*/
hobj_t * CSurf2Shape( hobj_t *csurf, hobj_t *tmobj )
{
	hobj_t		*shape;
	hobj_t		*csurf_real;
	hpair_t		*pair;
	hobj_t		*texdef;

	shape = EasyNewClass( "shape" );

	InsertHPair( shape, NewHPair2( "string", "tess_name", "csurf" ) );

	//
	// create a copy of the csurf, and clean it up
	//
	csurf_real = DeepCopyClass( csurf );
	ClassSetType( csurf_real, "csurf" );
	InsertClass( shape, csurf_real );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "scale" );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "vec2" );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "vec1" );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "vec0" );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "shift" );
	RemoveAndDestroyAllHPairsOfKey( csurf_real, "ident" );

	//
	// create texdef class
	//
	texdef = EasyNewClass( "uv_texdef0" );
	InsertClass( shape, texdef );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "scale" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "vec2" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "vec1" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "vec0" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "shift" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( csurf, "ident" ) ) );

	//
	// create material pair
	//
	pair = FindHPair( csurf, "ident" );
	if ( !pair )
		Error( "missing key 'ident'\n" );
	pair = FindHPair( tmobj, pair->value );
	if ( !pair )
	{
		// no special material defined, use default
		InsertHPair( shape, NewHPair2( "ref", "material", "default" ) );
	}
	else
	{
		InsertHPair( shape, NewHPair2( "ref", "material", pair->value ) );
	}
	
	return shape;	
}


int main( int argc, char *argv[] )
{
	char	*in_csurf_name;
	char	*out_shape_name;

	char	*in_texture_material_name;

	hmanager_t	*csurfhm;
	hmanager_t	*tmhm;

	hobj_t		*shape_root;
	hobj_t		*csurf;
	hobj_search_iterator_t		iter;

	FILE	*h;

	puts( "===== csurf2shape - init shape class from curved surfaces =====" );
	SetCmdArgs( argc, argv );

	in_csurf_name = GetCmdOpt2( "-i" );
	out_shape_name = GetCmdOpt2( "-o" );
	
	in_texture_material_name = GetCmdOpt2( "-tm" );

	printf( "output shape class: %s\n", out_shape_name );

	if ( !in_csurf_name )
		Error( "no input curved surface class\n" );

	if ( !out_shape_name )
		Error( "no ouput shape class\n" );

	if ( !in_texture_material_name )
		Error( "no input texture material class\n" );
	
	printf( "input curved surface class: %s\n", in_csurf_name );
	if ( ! ( csurfhm = NewHManagerLoadClass( in_csurf_name ) ) )
		Error( "load failed\n" );

	printf( "input texture material class: %s\n", in_texture_material_name );
	if ( ! ( tmhm = NewHManagerLoadClass( in_texture_material_name ) ) )
		Error( "load failed\n" );

	shape_root = NewClass( "shapes", "csurfs0" );
	
	InitClassSearchIterator( &iter, HManagerGetRootClass( csurfhm ), "csurface" );
	for ( ; ( csurf = SearchGetNextClass( &iter ) ) ; )
	{
		hobj_t		*shape;

		shape = CSurf2Shape( csurf, HManagerGetRootClass( tmhm ) );
		InsertClass( shape_root, shape );
	}

	h = fopen( out_shape_name, "w" );
	if ( !h )
		Error( "can't open file\n" );

	WriteClass( shape_root, h );
	fclose( h );

	HManagerSaveID();		

	exit(0);
}
