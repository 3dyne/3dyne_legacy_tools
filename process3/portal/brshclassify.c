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



// brshclassify.c

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

#include "defs.h"


int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_brush_name;

	hobj_t		*brushcls;
	hobj_search_iterator_t	iter;
	hobj_t		*brush;

	tokenstream_t	*ts;
	FILE		*h;

	int		surf_num = 0;
	int		ignore_num = 0;
	int		brush_num = 0;
	int		remove_num = 0;

	printf( "===== brshclassify - sets bsp_ignore if no polygons in a surface =====\n" );
	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-b" );
	out_brush_name = GetCmdOpt2( "-o" );

	if ( !in_brush_name )
	{
		in_brush_name = "_brshmrg_bspbrush.hobj";
		printf( " default input brush class: %s\n", in_brush_name );
	}
	else
	{
		printf( " input brush class: %s\n", in_brush_name );		
	}

	if ( !out_brush_name )
	{
		out_brush_name = "_brshclassify_bspbrush.hobj";
		printf( " default output brush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output brush name: %s\n", out_brush_name );
	}
		
	ts = BeginTokenStream( in_brush_name );
	if ( !ts )
		Error( "can't open input class.\n" );
	brushcls = ReadClass( ts );
	EndTokenStream( ts );

	InitClassSearchIterator( &iter, brushcls, "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &iter ) ) ; )
	{
		hobj_search_iterator_t	surfiter;
		hobj_t		*surface;
		int		surface_num = 0;

		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ) ; )
		{
			if ( FindClassType( surface, "polygon" ) )
			{
				// ok, at least on polygon
				surface_num++;
			}
			else
			{
				hpair_t		*pair;
				pair = NewHPair2( "int", "bsp_ignore", "1" );
				InsertHPair( surface, pair );
				ignore_num++;
			}
			surf_num++;
		}
		if ( surface_num == 0 )
		{
			hpair_t		*pair;

//			printf( "remove brush '%s', contents '%s'\n", brush->name, FindHPair(brush,"content")->value );
			pair = NewHPair2( "int", "remove", "1" );
			InsertHPair( brush, pair );
		}
		brush_num++;
	}
	
	// remove brush for real

	printf( "remove ...\n" );
	
	// hack, cause RemoveClass trashes the iterator ...
restart_list:
	InitClassSearchIterator( &iter, brushcls, "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &iter ) ) ; )
	{
		hpair_t		*pair;
		pair = FindHPair( brush, "remove" );
		if ( !pair )
			continue;
		RemoveClass2( brush );
		remove_num++;
		goto restart_list;
	}

	printf( " %d brushes removed of %d\n", remove_num, brush_num );
	printf( " %d surface of %d are marked as 'bsp_ignore'\n", ignore_num, surf_num );
      

	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "can't open output class.\n" );

	WriteClass( brushcls, h );
	fclose( h );

	exit(0);
}
