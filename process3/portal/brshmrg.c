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



// brshmrg.c

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
	char	*in_brush1_name;
	char	*in_brush2_name;
	char	*out_brush_name;
 
	hmanager_t	*brush1hm;
	hmanager_t	*brush2hm;

	hobj_search_iterator_t	iter;		
	hobj_t		*brush1;
	int		insert_num;

	FILE		*h;

	printf( "===== brshmrg - combine the polygons of two brush classe =====\n" );
	SetCmdArgs( argc, argv );

	in_brush1_name = GetCmdOpt2( "-b1" );
	in_brush2_name = GetCmdOpt2( "-b2" );
	out_brush_name = GetCmdOpt2( "-o" );

	if ( !in_brush1_name )
	{
		in_brush1_name = "_gather1_bspbrush.hobj";
		printf( " default input brush1 class: %s\n", in_brush1_name );
	}
	else
	{
		printf( " input brush1 class: %s\n", in_brush1_name );		
	}


	if ( !in_brush2_name )
	{
		in_brush2_name = "_gather2_bspbrush.hobj";
		printf( " default input brush2 class: %s\n", in_brush2_name );
	}
	else
	{
		printf( " input brush2 class: %s\n", in_brush2_name );		
	}
	

	if ( !out_brush_name )
	{
		out_brush_name = "_brshmrg_bspbrush.hobj";
		printf( " default output brush class: %s\n", out_brush_name );
	}
	else
	{
		printf( " output brush name: %s\n", out_brush_name );
	}
	
	printf( "load ...\n" );

	brush1hm = NewHManagerLoadClass( in_brush1_name );
	brush2hm = NewHManagerLoadClass( in_brush2_name );

	insert_num = 0;
	
	printf( "copy polygons from class two to class one ...\n" );

	InitClassSearchIterator( &iter, HManagerGetRootClass( brush1hm ), "bspbrush" );
	for ( ; (brush1 = SearchGetNextClass( &iter ) ); )
	{
		hobj_search_iterator_t	surfiter;		
		hobj_t		*brush2;
		hobj_t		*surface1;
		//
		// search brush1 in brush2hm
		//
		
		brush2 = HManagerSearchClassName( brush2hm, brush1->name );
		
		if ( !brush2 )
			Error( "can't find brush '%s' in second class.\n", brush1->name );

		InitClassSearchIterator( &surfiter, brush1, "surface" );
		for ( ; ( surface1 = SearchGetNextClass( &surfiter ) ) ; )
		{
			hobj_search_iterator_t	polyiter;
			hobj_t		*surface2;
			hobj_t		*poly;

			surface2 = FindClass( brush2, surface1->name );
			if ( !surface2 )
				Error( "can't find surface '%s' in second class.\n", surface1->name );

			// copy polygons from surface2 to surface1
#if 0
			InitClassSearchIterator( &polyiter, surface2, "polygon" );
			for ( ; ( poly = SearchGetNextClass( &polyiter ) ); )
			{
				RemoveClass2( poly );
				InsertClass( surface1, poly );
				insert_num++;
			}
#endif		
		restart_list:	// awful hack, cause RemoveClass2 would trashes the iterator
			for ( ; ( poly = FindClassType( surface2, "polygon" ) ) ; )
			{
				RemoveClass2( poly );
				InsertClass( surface1, poly );
				insert_num++;	
				goto restart_list;
			}
		}
	}

	printf( " %d polygons copied from brush2 class to brush1 class\n", insert_num );

	h = fopen( out_brush_name, "w" );
	if ( !h )
		Error( "can't output open file.\n" );
	WriteClass( HManagerGetRootClass( brush1hm ), h );
	fclose( h );
	
}
