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



// sface2shape.c

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
  SFace2Shape

  ==============================
*/
hobj_t * SFace2Shape( hobj_t *sface, hmanager_t *tdhm, hmanager_t *txhm, hobj_t *tmobj )
{
	hobj_t		*shape;
	hobj_t		*polygon;
	hobj_t		*texdef;
	hobj_t		*texture;
	hpair_t		*pair;
	char		str[256];

	shape = EasyNewClass( "shape" );

	InsertHPair( shape, NewHPair2( "string", "tess_name", "sface" ) );

	polygon = DeepCopyClass( sface );

	//
	// handle material, texdef
	//

	texdef = EasyLookupClsref( polygon, "texdef", tdhm );

	texture = EasyLookupClsref( texdef, "texture", txhm );

	pair = FindHPair( texture, "ident" );
	if ( !pair )
		Error( "missing key 'ident'\n" );

	//
	// remove sky sfaces
	//
	if ( strstr( pair->value, "sky" ) )
	{
		// fixme: destroy 'shape' and 'polygon'
		return NULL;
	}

	//
	// search material
	//
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

	{
		hobj_t		*texdef0;

		texdef0 = EasyNewClass( "proj_texdef0" );
		InsertClass( shape, texdef0 );

		EasyNewClsref( texdef0, "texdef", texdef );
	}

	RemoveAndDestroyAllHPairsOfKey( polygon, "texdef" );

	InsertClass( shape, polygon );
	
	return shape;
}


int main( int argc, char *argv[] )
{
	char	*in_sface_name;
	char	*out_shape_name;

	char	*in_texdef_name;
	char	*in_texture_name;
	char	*in_texture_material_name;

	int	remove_num;

	hmanager_t	*tdhm;
	hmanager_t	*txhm;
	hmanager_t	*tmhm;

	hmanager_t	*sfacehm;

	hobj_t		*shape_root;	
	hobj_search_iterator_t		iter;
	hobj_t		*sface;

	FILE		*h;

	puts( "===== sface2shape - init shape class from simple faces =====" );
	SetCmdArgs( argc, argv );

	in_sface_name = GetCmdOpt2( "-i" );
	out_shape_name = GetCmdOpt2( "-o" );

	in_texdef_name = GetCmdOpt2( "-td" );
	in_texture_name = GetCmdOpt2( "-tx" );
	in_texture_material_name = GetCmdOpt2( "-tm" );

	printf( "output shape class: %s\n", out_shape_name );

	if ( !in_sface_name )
		Error( "no input simple face class\n" );
	
	if ( !out_shape_name )
		Error( "no output shape class\n" );

	if ( !in_texdef_name )
		Error( "no input texdef class\n" );

	if ( !in_texture_name )
		Error( "no input texture class\n" );

	if ( !in_texture_material_name )
		Error( "no input textue material class\n" );

	printf( "input simple face class: %s\n", in_sface_name );
	if ( ! ( sfacehm = NewHManagerLoadClass( in_sface_name ) ) )
		Error( "load failed\n" );

	printf( "input texdef class: %s\n", in_texdef_name );
	if ( ! ( tdhm = NewHManagerLoadClass( in_texdef_name ) ) )
		Error( "load failed\n" );

	printf( "input texture class: %s\n", in_texture_name );
	if ( ! ( txhm = NewHManagerLoadClass( in_texture_name ) ) )
		Error( "load failed\n" );

	printf( "input texture material class: %s\n", in_texture_material_name );
	if ( ! ( tmhm = NewHManagerLoadClass( in_texture_material_name ) ) )
		Error( "load failed\n" );

	shape_root = NewClass( "shapes", "sfaces0" );

	remove_num = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( sfacehm ), "polygon" );
	for ( ; ( sface = SearchGetNextClass( &iter ) ) ; )
	{
		hobj_t		*shape;

		shape = SFace2Shape( sface, tdhm, txhm, HManagerGetRootClass( tmhm ) );

		if ( !shape )
		{
			remove_num++;
			continue;
		}

		InsertClass( shape_root, shape );
	}

	printf( " %d sfaces (sky ...) are ignored\n", remove_num );

	h = fopen( out_shape_name, "w" );
	if ( !h )
		Error( "can't open file\n" );

	WriteClass( shape_root, h );
	fclose( h );

	HManagerSaveID();	

	exit(0);
}
