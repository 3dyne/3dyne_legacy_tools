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



// class_split.c

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
	char		*out_true_class_name;
	char		*out_false_class_name;

	char		*compare_key;
	char		*compare_value;
	char		*compare_func;

	hobj_t		*in_root;
	hobj_t		*out_true_root;
	hobj_t		*out_false_root;
	hobj_t		*obj;
	hobj_search_iterator_t	iter;
	hpair_t		*pair;

	int		num_no_key;
	int		num_true;
	int		num_false;
	int		num_test;

	tokenstream_t	*ts;
	FILE		*h;


	puts( " --- SPLIT CLASS ---" );

	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -ic	: input class name" );
		puts( " -otc	: output true class name" );
		puts( " -ofc	: output false class name" );
		puts( " -k	: compare key" );
		puts( " -v	: compare value" );
		puts( " -cmp	: compare func 'eq'" );

		exit(-1);
	}

	SetCmdArgs( argc, argv );	

	in_class_name = GetCmdOpt2( "-ic" );
	out_true_class_name = GetCmdOpt2( "-otc" );
	out_false_class_name = GetCmdOpt2( "-ofc" );

	compare_key = GetCmdOpt2( "-k" );
	compare_value = GetCmdOpt2( "-v" );

	compare_func = GetCmdOpt2( "-cmp" );

	if ( !in_class_name )
		Error( "no input class\n" );
	if ( !out_true_class_name )
		Error( "no output true class\n" );
	if ( !out_false_class_name )
		Error( "no output false class\n" );
	if ( !compare_key )
		Error( "no compare key\n" );
	if ( !compare_value )
		Error( "no compare value\n" );
	if ( !compare_func )
		Error( "no compare func\n" );
	
	ts = BeginTokenStream( in_class_name );
	if ( !ts )
		Error( "can't open input class\n" );
	in_root = ReadClass( ts );
	EndTokenStream( ts );

	out_false_root = NewClass( in_root->type, in_root->name );
	out_true_root = NewClass( in_root->type, in_root->name );

	num_no_key = 0;
	num_false = 0;
	num_true = 0;
	num_test = 0;

	InitClassSearchIterator( &iter, in_root, "*" );
	for ( ; ( obj = SearchGetNextClass( &iter ) ) ; )
	{
		// get compare key

		num_test++;

		pair = FindHPair( obj, compare_key );
		if ( !pair )
		{
			// pair not found, compare is 'false'
			InsertClass( out_false_root, DeepCopyClass( obj ) );
			num_no_key++;
			num_false++;
			continue;
		}
		
		if ( !strcmp( compare_func, "eq" ) )
		{
			// compare for equal
			if ( !strcmp( pair->value, compare_value ) )
			{
				// equal
				InsertClass( out_true_root, DeepCopyClass( obj ) );
				num_true++;
			}
			else
			{
				// not equal
				InsertClass( out_false_root, DeepCopyClass( obj ) );			
				num_false++;
			}
		}
	}

	printf( " %d classes teseted\n", num_test );
	printf( " %d classes are false ( %d of them ain't got the key )\n", num_false, num_no_key );
	printf( " %d classes are true\n", num_true );

	h = fopen( out_false_class_name, "w" );
	if ( !h )
		Error( "can't open output false class\n" );
	WriteClass( out_false_root, h );
	fclose( h );

	h = fopen( out_true_class_name, "w" );
	if ( !h )
		Error( "can't open output true class\n" );
	WriteClass( out_true_root, h );
	fclose( h );

	exit(0);
}
