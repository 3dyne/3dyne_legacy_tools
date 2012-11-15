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



// class_alter.c

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
	char		*in_class_name;
	char		*out_class_name;

	char		*class_type;
	char		*compare_key;
	char		*value_from;
	char		*value_to;


	hmanager_t	*in_hm;
	hmanager_type_iterator_t	iter;
	hobj_t		*obj;

	hpair_t		*pair, *pair2;

	int		num_no_key;
	int		num_change;
	int		num_test;

	FILE		*h;


	puts( " --- ALTER CLASS ---" );

	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -ic	: input class name" );
		puts( " -oc	: output class name" );
		puts( " -t	: in all this class types ..." );
		puts( " -k	: look for this key ..." );
		puts( " -vf	: and change this value (from) ..." );
		puts( " -vt	: to this value (to) ..." );		

		exit(-1);
	}

	SetCmdArgs( argc, argv );	

	in_class_name = GetCmdOpt2( "-ic" );
	out_class_name = GetCmdOpt2( "-oc" );

	class_type = GetCmdOpt2( "-t" );
	compare_key = GetCmdOpt2( "-k" );
	value_from = GetCmdOpt2( "-vf" );
	value_to = GetCmdOpt2( "-vt" );

	if ( !in_class_name )
		Error( "no input class\n" );
	if ( !out_class_name )
		Error( "no output class\n" );
	if ( !class_type )
		Error( "no class type\n" );
	if ( !compare_key )
		Error( "no compare key\n" );
	if ( !value_from )
		Error( "no 'from' value\n" );
	if ( !value_to )
		Error( "no 'to' value\n" );
	
	in_hm = NewHManagerLoadClass( in_class_name );
	if ( !in_hm )
		Error( "class load failed\n" );
	
	num_no_key = 0;
	num_change = 0;
	num_test = 0;

	HManagerInitTypeSearchIterator( &iter, in_hm, class_type );
	for ( ; ( obj = HManagerGetNextClass( &iter ) ) ; )
	{
		// get compare key

		num_test++;

		pair = FindHPair( obj, compare_key );
		if ( !pair )
		{
			num_no_key++;
			continue;
		}

		// compare for equal
		if ( strcmp( pair->value, value_from ) )
		{			
			continue;
		}

		pair2 = NewHPair2( pair->type, pair->key, value_to );
		RemoveHPair( obj, pair );
		FreeHPair( pair );
		InsertHPair( obj, pair2 );
		
		num_change++;
	}

	printf( " %d classes teseted\n", num_test );
	printf( " %d pairs are changed\n", num_change );
	printf( " %d classes ain't got the key\n", num_no_key );

	h = fopen( out_class_name, "w" );
	if ( !h )
		Error( "can't open output false class\n" );
	WriteClass( HManagerGetRootClass( in_hm ), h );
	fclose( h );
	
	exit(0);
}
