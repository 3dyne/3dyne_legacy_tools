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



// class_new.c

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
	char	*out_class_name;
	char	*class_name;
	char	*class_type;
	hobj_t	*obj;
	FILE		*h;

	puts( " --- NEW CLASS --- " );

	SetCmdArgs( argc, argv );	

	out_class_name = GetCmdOpt2( "-o" );
	class_name = GetCmdOpt2( "-name" );
	class_type = GetCmdOpt2( "-type" );

	if ( !out_class_name )
		Error( "no output class name (-o)\n" );

	if ( !class_type )
		Error( "no class type (-type)\n" );

	printf( "class type: %s\n", class_type );

	if ( !class_name )
	{
		printf( "no class name: generate unique id\n" );
		obj = EasyNewClass( class_type );
	}
	else
	{
		obj = NewClass( class_type, class_name );
	}
	printf( "class name: %s\n", ClassGetName( obj ) );

	h = fopen( out_class_name, "w" );
	if ( !h )
		Error( "can't open file\n" );
	WriteClass( obj, h );
	fclose( h );

	HManagerSaveID();

	exit(0);
}
