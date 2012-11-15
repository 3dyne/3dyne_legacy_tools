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



// iosanity.c

// test sanity of io-subsystem

#include "lib_error.h"

int WriteASCIITest( char *filename, int num )
{
//	int	c;
	int	i, ii;
	FILE	*h;

	printf( "writing test file %s ( %d * 128 )\n", filename, num );

	h = fopen( filename, "wb" );
	
	if( !h )
		return 1;

	for( i = 0; i < num; i++ )
	{
		for( ii = 0; ii < 128; ii++ )
			fputc( ii, h );
	}

	fclose( h );

	printf( "done.\n" );
	return 0;
}

int CheckASCIITest( char *filename, int num )
{
	int	i, ii, c;

	FILE	*h;

	printf( "checking ...\n" );

	h = fopen( filename, "rb" );

	if( !h )
		return	1;

	c = 0;

	for( i = 0; i < num; i++ )
	{
		for( ii = 0; ii < 128; ii++ )
		{
			//c = fgetc( h );

			fread( &c, 1, 1, h );
			if( c != ii )
			{
				printf( "incosistency: at position 0x%x\n", 127 * num + ii );
				printf( "expected: %d, got: %d\n", ii, c );
				return 1;
			}
		}
	}
	fclose( h );
	printf( "sane.\n" );
	return 0;
}


int main()
{
	int	i;

	for( i = 0; i < 256; i++ )
	{
		if( WriteASCIITest( "io.test", 256 * 1024 ) )
			Error( "exiting\n" );

		if( CheckASCIITest( "io.test", 256 * 1024 ) )
			Error( "input error\n" );
	}
	return 0;
}
		
