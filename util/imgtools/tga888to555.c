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

#include "shock.h"
#include "tga.h"
#include "cmdpars.h"

int main( int argc, char *argv[] )
{
	char	*tname;
	char	*lname;
	tga_t	*tga;
	
	FILE	*thandle;
	int	i;
	
	unsigned int	r, g, b;
	unsigned short	sbuf;
	unsigned char	*buf555;
	
	unsigned int	width, height, bytes;

	tname = GetCmdOpt( "-t", argc, argv );
	if( !tname )
		__error( "missing argument -t\n" );

	thandle = fopen( tname, "rb" );
	if( !thandle )
		__error( "cannot open file\n" );

	tga = TGA_Read( thandle );

	__chkptr( tga );

	if( tga->image_type != TGA_TYPE_TRUECOLOR )
		__error( "tga not truecolor\n" );
	
	width = tga->image_width;
	height = tga->image_height;
	bytes = tga->image.bytes;

//	buf = ( unsigned char* ) malloc( width * height * 2 );

	fwrite( &width, 4, 1, stdout );
	fwrite( &height, 4, 1, stdout );
	fflush( stdout );
	for( i = 0; i < tga->image.pixels; i++ )
	{
		r = tga->image.red[i];
		r>>=3;
		g = tga->image.green[i];
		g>>=3;
		b = tga->image.blue[i];
		b>>=3;

		r<<=10;
		g<<=5;
		
		sbuf = r | g | b;
		
		fwrite( &sbuf, sizeof( unsigned short ), 1, stdout );
		fflush( stdout );
	}
	fflush( stdout );

	fclose( thandle );
}
