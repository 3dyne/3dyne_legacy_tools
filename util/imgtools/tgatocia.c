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



// tgatocia.c

#include "shock.h"
#include "tga.h"
#include "pal.h"

int main()
{
	tga_t		*color;
	tga_t		*alpha;
	tga_t		*index;

	unsigned char	mask_red;
	unsigned char	mask_green;
	unsigned char	mask_blue;
	unsigned char	alpha_true;
	unsigned char	alpha_false;
	
	FILE		*h;

	int		i;
	int		width, height;
	int		pixelnum;

	mask_red = 0;
	mask_green = 0;
	mask_blue = 255;

	alpha_false = 0;
	alpha_true = 127;

	fprintf( stderr, "read colors ...\n" );
	h = fopen( "rgb.tga", "rb" );
	color = TGA_Read( h );
	fclose( h );

	if ( color->image_type != TGA_TYPE_TRUECOLOR )
		__error( " no true color tga.\n" );

	fprintf( stderr, "read indexed colors ...\n" );
	h = fopen( "index.tga", "rb" );
	index = TGA_Read( h );
	fclose( h );

	if ( index->image_type != TGA_TYPE_INDEXED )
		__error( " no indexed color tga.\n" );

	fprintf( stderr, "read alpha mask ...\n" );
	h = fopen( "mask.tga", "rb" );
	alpha = TGA_Read( h );
	fclose( h );

	if ( alpha->image_type != TGA_TYPE_TRUECOLOR )
		__error( " no true color tga.\n" );


	width = color->image_width;
	height = color->image_height;
	pixelnum = width * height;

	if ( width != index->image_width || width != color->image_width ||
	     height != index->image_height || height != color->image_height )
		__error( " diffrent image sizes.\n" );

	fprintf( stderr, "size: %d x %d\n", width, height );


	//
	// write cia-header
	//

	fwrite( "CIA1", 4, 1, stdout );
	fwrite( &width, 4, 1, stdout );
	fwrite( &height, 4, 1, stdout );


	//
	// write palette from indexed
	//

	fwrite( index->image_indexed.pal->rgb_set, 256*3, 1, stdout );


	//
	// write indexed 
	//

	fwrite( index->image_indexed.data, pixelnum, 1, stdout );


	//
	// write alpha
	//

	for ( i = 0; i < pixelnum; i++ )
	{
		if ( alpha->image.red[i] == mask_red &&
		     alpha->image.green[i] == mask_green &&
		     alpha->image.blue[i] == mask_blue )
		{
			fputc( alpha_true, stdout );
		}
		else
		{
			fputc( alpha_false, stdout );
		}
	}

	
	//
	// write rgb
	//

	for ( i = 0; i < pixelnum; i++ )
	{
		fputc( color->image.red[i], stdout );
		fputc( color->image.green[i], stdout );
		fputc( color->image.blue[i], stdout );
		
	}
	
}
