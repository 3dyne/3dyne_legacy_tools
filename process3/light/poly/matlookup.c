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



// matlookup.c

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
  LookupMaterial

  ==============================
*/
void LookupMaterial( hmanager_t *inhm, hmanager_t *tdhm, hmanager_t *txhm, hobj_t *tmobj )
{
	hobj_search_iterator_t	iter;
	hobj_t		*obj;

	InitClassSearchIterator( &iter, HManagerGetRootClass( inhm ), "*" );
	for ( ; ( obj = SearchGetNextClass( &iter ) ) ; )
	{
		hpair_t		*ident;

		// is there a pair with key 'ident' ?
		ident = FindHPair( obj, "ident" );
		if ( ident )
		{
			hpair_t		*clsref;
			// yes
			
			clsref = FindHPair( tmobj, ident->value );
			if ( !clsref )
			{
				// no special material for ident, use 'default'
				InsertHPair( obj, NewHPair2( "ref", "material", "default" ) );
			}
			else
			{
				InsertHPair( obj, NewHPair2( "ref", "material", clsref->value ) );
			}
		}

		if ( tdhm && txhm )
		{
			hpair_t		*pair;

			// is there a pair with key 'texdef' ?	       
			pair = FindHPair( obj, "texdef" );
			if ( pair )
			{
				hobj_t		*texdef;
				hobj_t		*texture;
				hpair_t		*texmat;
			       
				texdef = HManagerSearchClassName( tdhm, pair->value );
				if ( !texdef )
				{
					Error( "can't find texdef '%s'\n", pair->value );
				}

				pair = FindHPair( texdef, "texture" );
				
				texture = HManagerSearchClassName( txhm, pair->value );
				if ( !texture )
				{
					Error( "can't find texture '%s'\n", pair->value );
				}

				ident = FindHPair( texture, "ident" );
				
				if ( !ident )
				{
					Error( "can't find key 'ident' in texdef '%s'\n", texdef->name );
				}
								
				texmat = FindHPair( tmobj, ident->value );
				if ( !texmat )
				{
					// no special material for ident, use 'default'
					InsertHPair( obj, NewHPair2( "ref", "material", "default" ) );
				}
				else
				{
					InsertHPair( obj, NewHPair2( "ref", "material", texmat->value ) );
				}	
			}			
		}		
	}
}



int main( int argc, char *argv[] )
{
	char	*in_name;
	char	*out_name;

	char	*in_texdef_name;
	char	*in_texture_name;
	char	*in_texture_material_name;

	hmanager_t	*inhm;
	hmanager_t	*texdefhm;
	hmanager_t	*texturehm;
	hmanager_t	*texturematerialhm;
	
	FILE	*h;
	
	puts( "===== matlookup - search for all 'ident' or 'texdef' the material" );
	SetCmdArgs( argc, argv );

	in_name = GetCmdOpt2( "-i" );
	out_name = GetCmdOpt2( "-o" );
	
	in_texdef_name = GetCmdOpt2( "-td" );
	in_texture_name = GetCmdOpt2( "-tx" );
	in_texture_material_name = GetCmdOpt2( "-tm" );

	if ( !in_name )
	{
		Error( "no input class name\n" );
	}
	else
	{
		inhm = NewHManagerLoadClass( in_name );
	}

	if ( !out_name )
		Error( "no output class name\n" );

	if ( !in_texdef_name )
	{
		printf( "Warning: no texdef class, ignore all 'texdef' clsrefs !\n" );
		texdefhm = NULL;
	}
	else
	{
		texdefhm = NewHManagerLoadClass( in_texdef_name );
	}
	
	if ( !in_texture_name )
	{
		printf( "Warning: no texture class, ignore all 'texdef' clsrefs !\n" );
		texturehm = NULL;
	}
	else
	{
		texturehm = NewHManagerLoadClass( in_texture_name );
	}

	if ( !in_texture_material_name )
	{
		Error( "no texture material class name\n" );
	}
	else
	{
		texturematerialhm = NewHManagerLoadClass( in_texture_material_name );
	}

	LookupMaterial( inhm, texdefhm, texturehm, HManagerGetRootClass( texturematerialhm ) );

	h = fopen( out_name, "w" );
	if ( !h )
		Error( "can't open file\n" );

	WriteClass( HManagerGetRootClass( inhm ), h );
	fclose( h );
}
