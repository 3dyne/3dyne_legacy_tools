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



#include <stdio.h>

#include "arr.h"
#include "shock.h"
#include "cmdpars.h"

void main( int argc, char* argv[] )
{
	arr_t*	inarr;
	arr_t*	outarr;

	FILE*	inhandle;
	FILE*	outhandle;
	
	char*	outname;
	char*	data;
	char*	ident;

	int	firstarg;
	int	i;
	int	width, height, size;
	int	steps;
	int	flags;

	width = height = size = 0;

	outname = GetCmdOpt( "-o", argc, argv );
	__chkptr( outname );
//	printf( "creating animation %s\n", outname );
	
	ident = GetCmdOpt( "-i", argc, argv );

	firstarg = 3;
	if( ident != NULL )
		firstarg+=2;

	steps = argc - firstarg;
	
	for( i = firstarg; i < argc; i++ )
	{
		inhandle = fopen( argv[i], "rb" );
		__chkptr( inhandle );
//		printf( "%s\n", argv[i] );
		inarr = ARR_Read( inhandle );
		if( size == 0 )
		{
			size = inarr -> bytes;
			width = inarr -> size_x;
			height = inarr -> size_y;
			flags = inarr->flags;
			data = ( char* )malloc( size * steps );
			__chkptr( data );
		}
		memcpy( data+size*(i-firstarg), inarr->data, size );
	}
	outarr = ARR_Create( width, height, 4, steps, ident, flags );
	memcpy( outarr->data, data, size * steps );
	
	outhandle = fopen( outname, "wb" );
	ARR_Write( outhandle, outarr );
}
