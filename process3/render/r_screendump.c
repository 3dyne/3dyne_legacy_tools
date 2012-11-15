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



// r_screendump.c

#include <math.h>

#include "interfaces.h"
#include "defs.h"
#include "shared.h"

#include "render.h"

#include "tga.h"

/*
  ==============================
  R_ScreenDump

  ==============================
*/

void R_ScreenDump( char *name )
{
	int		j;
	unsigned char	lfb[SIZE_X*SIZE_Y*4];
	FILE		*h;
	tga_t		*tga;

	printf( "R_ScreenDump: write tga to file %s\n", name );

	glReadBuffer( GL_FRONT );
	glReadPixels( 0, 0, SIZE_X, SIZE_Y, GL_RGB, GL_UNSIGNED_BYTE, lfb );

	tga = TGA_Create( SIZE_X, SIZE_Y, TGA_TYPE_TRUECOLOR );
	for ( j = 0; j < SIZE_X*SIZE_Y; j++ )
	{
		tga->image.red[j] = lfb[j*3];
		tga->image.green[j] = lfb[j*3+1];
		tga->image.blue[j] = lfb[j*3+2];
	}

	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	TGA_Write( h, tga );
	fclose( h );
	TGA_Free( tga );
}
