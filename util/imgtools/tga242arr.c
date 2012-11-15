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



// tga242arr.c
#include <stdio.h>
#include <sys/types.h>

#include "arr.h"
#include "tga.h"
#include "pal.h"
#include "shock.h"
#include "cmdpars.h"

void PrintUsage()
{
	printf( "usage: tga242arr -t tga -a arr -p pal [ -i ident ]\n" );
}

void main( int argc, char* argv[] )
{
	u_int32_t	i;
	char	dump_headers = 0;
	char*	tga_name;
	char*   arr_name;
	char*	pal_name;
	char*	arr_ident;

	FILE*	tga_handle;
	FILE*	arr_handle;
	FILE*	pal_handle;
	pal_t*	pal;	
	tga_t*	tga;
	arr_t*	arr;
	rgb_t	rgb;
	
	dump_headers = CheckCmdSwitch( "-d", argc, argv );

	pal_name = GetCmdOpt( "-p", argc, argv );
	if( pal_name == NULL )
	{
		printf( "no pal given.\n" );
		PrintUsage();
		exit( 0 );
	}
	printf( "pal: %s\n", pal_name );
	pal_handle = fopen( pal_name, "rb" );
	CHKPTR( pal_handle );
	pal = PAL_Read( pal_handle );
	CHKPTR( pal );
	fclose( pal_handle );
	if( dump_headers )
		PAL_Dump( pal );

	tga_name = GetCmdOpt( "-t", argc, argv );
	if( tga_name == NULL )
	{
		printf( "no tga given.\n" );
		PrintUsage();
		exit( 0 );
	}
	printf( "tga: %s\n", tga_name );
	tga_handle = fopen( tga_name, "rb" );
	CHKPTR( tga_handle );
	tga = TGA_Read( tga_handle );
	CHKPTR( tga );
	fclose( tga_handle );
	if( dump_headers )
		TGA_Dump( tga );
	
	arr_name = GetCmdOpt( "-a", argc, argv );
	if( arr_name == NULL )
	{
		printf( "no arr given.\n" );
		PrintUsage();
		exit( 0 );
	}
	printf( "arr: %s\n", arr_name );
	if( tga->image_type == TGA_TYPE_TRUECOLOR )
	{
		
		arr = ARR_Create( tga->image_width, tga->image_height, 1, NULL, NULL );
		CHKPTR( arr );
		printf( "reducing color. " );
		for( i = 0; i < tga->image.pixels; i++ )
		{
			rgb.red = tga->image.red[i];
			rgb.green = tga->image.green[i];
			rgb.blue = tga->image.blue[i];
			
			arr->data[i] = PAL_ReduceColor( pal, &rgb );
			if( (i & 0xff) == 0 )
			{
				printf( "." );
				fflush( stdout );
			}
		}
		printf( "\n" );
		arr_handle = fopen( arr_name, "wb" );
		CHKPTR( arr_handle );
		ARR_Write( arr_handle, arr );
		fclose( arr_handle );
	}

}

