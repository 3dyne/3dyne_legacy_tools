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



// cface2shape.c

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
  BuildPolygonFromCFace

  ==============================
*/
hobj_t * BuildPolygonFromCFace( hobj_t *cpoly, hmanager_t *planehm )
{
	vec3d_t		norm;
	fp_t		dist;
	polygon_t	*p;
	hpair_t		*pair;
	hobj_t		*plane;
	hobj_t		*poly;
	vec3d_t		min, max;
	int		i;
	char		str[256];

	poly = EasyNewClass( "polygon" );

	//
	// baseplane
	//

	pair = FindHPair( cpoly, "plane" );
	if ( !pair )
		Error( "missing key 'plane' in cpoly '%s'\n", cpoly->name );

	InsertHPair( poly, CopyHPair( pair ) );

	plane = HManagerSearchClassName( planehm, pair->value );
	if ( !plane )
		Error( "can't find plane '%s'\n", pair->value );

	EasyFindVec3d( norm, plane, "norm" );
	EasyFindFloat( &dist, plane, "dist" );

	p = BasePolygonForPlane( norm, dist );

	
	//
	// clip by bound box
	//

	EasyFindVec3d( min, cpoly, "min" );
	EasyFindVec3d( max, cpoly, "max" );
	
	for ( i = 0; i < 3; i++ )
	{
		if ( !p )
		{
			return NULL;
//			Error( "clipped polygon away\n" );
		}

		Vec3dInit( norm, 0, 0, 0 );
		norm[i] = 1.0;
		dist = ( max[i] );
		ClipPolygonInPlace( &p, norm, dist );

		if ( !p )
		{
			return NULL;
//			Error( "clipped polygon away\n" );
		}

		norm[i] = -1.0;
		dist = -( min[i] );
		ClipPolygonInPlace( &p, norm, dist );
	}

	if ( !p )
	{
		return NULL;
//		Error( "clipped polygon away\n" );
        }

	EasyNewInt( poly, "num", p->pointnum );

	for ( i = 0; i < p->pointnum; i++ )
	{
		sprintf( str, "%d", i );
		EasyNewVec3d( poly, str, p->p[i] );
	}

	return poly;	
}

/*
  ==============================
  CFace2Shape

  ==============================
*/
hobj_t * CFace2Shape( hobj_t *cface, hobj_t *tmobj, hmanager_t *plhm )
{
	hobj_t		*shape;
	hobj_t		*polygon;
	hobj_t		*cface_real;
	hobj_t		*texdef;
	hpair_t		*pair;


	//
	// create light-able polygon from bound box
	//

	polygon = BuildPolygonFromCFace( cface, plhm );
	if ( !polygon )
	{
		return NULL;
	}

        shape = EasyNewClass( "shape" );
        InsertHPair( shape, NewHPair2( "string", "tess_name", "cface" ) );
	InsertClass( shape, polygon );

	//
	// create a copy of the cface, and clean it up
	//
	cface_real = DeepCopyClass( cface );
	ClassSetType( cface_real, "cface" );
	InsertClass( shape, cface_real );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "rotate" );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "scale" );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "shift" );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "ident" );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "dist" );
	RemoveAndDestroyAllHPairsOfKey( cface_real, "norm" );

	//
	// create texdef class
	//
	texdef = EasyNewClass( "proj_texdef0" );
	InsertClass( shape, texdef );
	InsertHPair( texdef, CopyHPair( FindHPair( cface, "rotate" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( cface, "scale" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( cface, "shift" ) ) );
	InsertHPair( texdef, CopyHPair( FindHPair( cface, "ident" ) ) );
	

	//
	// create material pair
	//
	pair = FindHPair( cface, "ident" );
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
	char	*in_cface_name;
	char	*out_shape_name;
	char	*in_plane_name;

	char	*in_texture_material_name;

	hmanager_t	*cfacehm;
	hmanager_t		*tmhm;
	hmanager_t		*plhm;

	hobj_t		*shape_root;
	hobj_t		*cface;
	hobj_search_iterator_t		iter;

	FILE	*h;

	puts( "===== cface2shape - init shape class from curved faces =====" );
	SetCmdArgs( argc, argv );

	in_cface_name = GetCmdOpt2( "-i" );
	out_shape_name = GetCmdOpt2( "-o" );
	in_plane_name = GetCmdOpt2( "-pl" );

	in_texture_material_name = GetCmdOpt2( "-tm" );

	printf( "output shape class: %s\n", out_shape_name );

	if ( !in_cface_name )
		Error( "no input curved face class\n" );

	if ( !out_shape_name )
		Error( "no ouput shape class\n" );

	if ( !in_texture_material_name )
		Error( "no input texture material class\n" );

	if ( !in_plane_name )
		Error( "no input plane class\n" );

	printf( "input curved face class: %s\n", in_cface_name );
	if ( ! ( cfacehm = NewHManagerLoadClass( in_cface_name ) ) )
		Error( "load failed\n" );

	printf( "input texture material class: %s\n", in_texture_material_name );
	if ( ! ( tmhm = NewHManagerLoadClass( in_texture_material_name ) ) )
		Error( "load failed\n" );

	printf( "input plane class: %s\n", in_plane_name );
	if ( ! ( plhm = NewHManagerLoadClass( in_plane_name ) ) )
		Error( "load failed\n" );

	shape_root = NewClass( "shapes", "cfaces0" );
	
	InitClassSearchIterator( &iter, HManagerGetRootClass( cfacehm ), "cpoly" );
	for ( ; ( cface = SearchGetNextClass( &iter ) ) ; )
	{
		hobj_t		*shape;

		shape = CFace2Shape( cface, HManagerGetRootClass( tmhm ), plhm );
		if ( !shape )
		{
			printf( "CFace2Shape failed for '%s'\n", cface->name );
			continue;
		}

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
	
