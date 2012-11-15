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



// flat.c

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

void MakeLocalClsRefsRecursive( hobj_t *obj, hobj_t *flat )
{
	hobj_t		*o;
	hpair_t		*pair;

	// make local class ref
	
	for ( o = obj->hobjs; o ; o=o->next )
	{
		pair = NewHPair2( "lclsref", o->type, o->name );
		InsertHPair( obj, pair );
		MakeLocalClsRefsRecursive( o, flat );
	}

	for ( ; ( o = obj->hobjs ) ; )
	{
		RemoveClass2( o );
		InsertClass( flat, o );
	}
}

int main( int argc, char *argv[] )
{
	char	*in_class_name;
	char	*out_flat_name;
	char	*in_entry_name;
	char	*in_e;

	hmanager_t	*classhm;
	hmanager_t	*entryhm;
	hobj_t		*flat;

	FILE		*h;

	printf( "===== flat - flatten a class hierarchy by creating local clsrefs =====\n" );

	SetCmdArgs( argc, argv );

	in_class_name = GetCmdOpt2( "-i" );
	out_flat_name = GetCmdOpt2( "-o" );
	in_entry_name = GetCmdOpt2( "-entry" );
	in_e = GetCmdOpt2( "-e" );

	if ( !in_class_name )
		Error( "no input class\n" );
	if ( !out_flat_name )
		Error( "no output flat\n" );

	if ( !in_entry_name )
	{
		printf( "don't generate entry\n" );
		entryhm = NULL;
	}
	else
	{
		printf( "generate entry\n" );
		entryhm = NewHManagerLoadClass( in_entry_name );
		if ( !entryhm )
		{
			hobj_t	*root;
			printf( "no input entry class, generate new ...\n" );
			entryhm = NewHManager();
			root = NewClass( "entries", "entries0" );
			HManagerSetRootClass( entryhm, root );
		}
	}
	

	printf( "loading class ...\n" );
	classhm = NewHManagerLoadClass( in_class_name );
	if ( !classhm )
		Error( " failed\n" );


	if ( entryhm )
	{
		hpair_t		*pair;
		hobj_t		*root;


		root = HManagerGetRootClass( classhm );

		if ( in_e )
		{
			pair = NewHPair2( "ref", in_e, root->name );
		}
		else
		{
			pair = NewHPair2( "ref", root->type, root->name );
		}
		InsertHPair( HManagerGetRootClass( entryhm ), pair );
		
		h = fopen( in_entry_name, "w" );
		WriteClass( HManagerGetRootClass( entryhm ), h );
		fclose( h );
	}

	flat = NewClass( "flat", "flat1" );
	MakeLocalClsRefsRecursive( HManagerGetRootClass( classhm ), flat );
	// add root class itself
	InsertClass( flat, HManagerGetRootClass( classhm ) );

	h = fopen( out_flat_name, "w" );
	if ( !h )
		Error( "can't open flat\n" );
//	WriteClassWithoutRoot( flat, h );
	WriteClass( flat, h );

	fclose( h );

}
