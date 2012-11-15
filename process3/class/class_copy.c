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



// class_copy.c

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


int main( int argc, char *argv[] )
{
	char	*in_dest_name;
	char	*out_dest_name;
	char	*in_src_name;

	char	*dest_class;
	char	*src_class;

	hmanager_t	*desthm;
	hobj_t		*destobj;

	hmanager_t		*srchm;
	hobj_t		*srcobj;
	hobj_t		*obj;

	FILE		*h;

	puts( " --- COPY CLASS --- " );

	SetCmdArgs( argc, argv );

	in_dest_name = GetCmdOpt2( "-idest" );
	out_dest_name = GetCmdOpt2( "-odest" );
	in_src_name = GetCmdOpt2( "-isrc" );
	dest_class = GetCmdOpt2( "-dest" );
	src_class = GetCmdOpt2( "-src" );

	if ( !in_dest_name )
		Error( "no input destination class file\n" );

	if ( !out_dest_name )
		Error( "no output destination class file\n" );

	if ( !in_src_name )
		Error( "no input source class file\n" );

	if ( !dest_class )
		Error( "no destination class name\n" );
	
	if ( !src_class )
	{
		printf( "copy source class with root !\n" );
	}

	printf( "input destination class file: %s\n", in_dest_name );
	if ( ! ( desthm = NewHManagerLoadClass( in_dest_name ) ) )
		Error( "laad failed\n" );

	printf( "input source class file: %s\n", in_src_name );
	if ( ! ( srchm = NewHManagerLoadClass( in_src_name ) ) )
		Error( "load failed\n" );

	destobj = HManagerSearchClassName( desthm, dest_class );
	if ( !destobj )
		Error( "can't find destination class '%s'\n", dest_class );

	if ( !src_class )
	{
		InsertClass( destobj, DeepCopyClass( HManagerGetRootClass( srchm ) ) );
	}
	else
	{
		hobj_search_iterator_t	iter;
		hobj_t	*obj;

		srcobj = HManagerSearchClassName( srchm, src_class );
		if ( !srcobj )
			Error( "can't find source class '%s'\n", src_class );
		
		// copy each object of the srcobj
		InitClassSearchIterator( &iter, srcobj, "*" );
		for ( ; ( obj = SearchGetNextClass( &iter ) ) ; )
		{
			InsertClass( destobj, DeepCopyClass( obj ) );
		}
	}

	h = fopen( out_dest_name, "w" );
	if ( !h )
		Error( "can't open file\n" );
	WriteClass( HManagerGetRootClass( desthm ), h );
	fclose( h );

	exit(0);
}
