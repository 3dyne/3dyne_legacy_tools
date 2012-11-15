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

// log:
// 1.11.98 : big change from P8 to RGB565

#include <stdio.h>
#include <sys/types.h>

#include "arr.h"
#include "tga.h"
#include "pal.h"
#include "shock.h"
#include "cmdpars.h"
#include "cdb_service.h"

void PrintUsage()
{
	printf( "usage: tga242arr -t tga -a arr -p pal [ -i ident ] [ --rgb565 ]\n" );
}

unsigned short RGB888ToRGB565( rgb_t *rgb )
{
	unsigned short		c,r,g,b;
	r = rgb->red;
	r>>=3;
	g = rgb->green;
	g>>=2;
	b = rgb->blue;
	b>>=3;

	r<<=11;
	g<<=5;

	c = r | b | g;

	return c;
}

void main( int argc, char* argv[] )
{
	u_int32_t	i, i2, arrcount;
	int	x, y, xs, ys, xh, yh;
	char	dumpheaders = 0;
	char*	tganame;
	char*   arrname;
	char*	palname;
	char*	arrident;
	char*	arrnext;
	char*	arrmode;

	int	reducemode;

	int		mm;
	int		ts, td;
	int		pixels;
	int		rsum, gsum, bsum;
	int		x2, y2;

	int		blur;

	FILE*	tgahandle;
	FILE*	arrhandle;
	FILE*	palhandle;
	pal_t*	pal = NULL;	
	tga_t*	tga[2];
	arr_t*	arr;
	rgb_t	rgb;
	
	dumpheaders = CheckCmdSwitch( "-d", argc, argv );

	palname = GetCmdOpt( "-p", argc, argv );

	tganame = GetCmdOpt( "-t", argc, argv );
	if( tganame == NULL )
	{
		printf( "no tga given.\n" );
		PrintUsage();
		exit( 0 );
	}


	if ( CheckCmdSwitch( "--rgb565", argc, argv ) )
	{
		reducemode = ARR_F_RGB565;
	}
	else
	{
		reducemode = ARR_F_P8;
	}

	arrident = GetCmdOpt( "-i", argc, argv );
	if( arrident == NULL )
		__warning( "no arr ident given.\n" );
	else if( strlen( arrident ) >= 32 )
		arrident[31] = '\0';

	printf( "tga: %s\n", tganame );
	tgahandle = fopen( tganame, "rb" );
	CHKPTR( tgahandle );
	tga[0] = TGA_Read( tgahandle );
       	CHKPTR( tga[0] );
//	TGA_Dump( tga[0] );

	fclose( tgahandle );
	if( dumpheaders )
		TGA_Dump( tga[0] );

	tga[1] = TGA_Create( tga[0]->image_width/*/2*/, tga[0]->image_height/*/2*/, TGA_TYPE_TRUECOLOR );
	__chkptr( tga[1] );
	
	arrname = GetCmdOpt( "-a", argc, argv );
	if( arrname == NULL )
	{
		printf( "no arr given.\n" );
		PrintUsage();
		exit( 0 );
	}
	printf( "arr: %s\n", arrname );
	
	switch( tga[0]->image_type )
	{
	case TGA_TYPE_TRUECOLOR:
		
		printf( "tga is 24bit.\n" );
		if( palname == NULL )
		{
			CDB_StartUp( 0 );
			palname = CDB_GetString( "misc/default_pal" );
			if( palname == NULL )
			{
				PrintUsage();
				__error( "no pal found.\n" );
			}
		}
		printf( "pal: %s\n", palname );
		palhandle = fopen( palname, "rb" );
		CHKPTR( palhandle );
		pal = PAL_Read( palhandle );
		CHKPTR( pal );
		fclose( palhandle );

		arr = ARR_Create( tga[0]->image_width, tga[0]->image_height, 4, 1, arrident, reducemode );

		CHKPTR( arr );
		printf( "reducing color. " );

		xs = tga[0]->image_width;
		ys = tga[0]->image_height;
		arrcount = 0;

		for ( mm = 0; mm < 4; mm++ ) {
			ts = mm & 1;
			td = (mm+1) & 1;
//			printf("%d->%d\n",ts,td);
			// reduce
//			printf(" i: %d, x: %d, y: %d\n", mm, xs, ys );
			pixels = xs * ys;
			for( i = 0; i < pixels; i++ )
			{
				rgb.red = tga[ts]->image.red[i];
				rgb.green = tga[ts]->image.green[i];
				rgb.blue = tga[ts]->image.blue[i];
				
				if ( reducemode == ARR_F_P8 )
					arr->data[arrcount++] = PAL_ReduceColor( pal, &rgb );
				else
				{
					*((unsigned short*)(&arr->data[arrcount])) = RGB888ToRGB565( &rgb );
					arrcount+=2;
				}

				if( (i & 0xff) == 0 )
				{
					printf( "." );
					fflush( stdout );
				}
			}			

			if ( mm == 3 )
				break;
			
			// mipmap 
			xh = xs / 2;
			yh = ys / 2;

			if ( xh < 4 || yh < 4 )
				blur = 0;
			else
				blur = 1;

			for ( y = 0; y < yh; y++ ) {
				for ( x = 0; x < xh; x++ ) {
					x2 = x*2;
					y2 = y*2;

					if ( blur )
					{
						rsum = tga[ts]->image.red[ y2*xs + x2 ];
						gsum = tga[ts]->image.green[ y2*xs + x2 ];
						bsum = tga[ts]->image.blue[ y2*xs + x2 ];
						
						rsum += tga[ts]->image.red[ y2*xs + (x2+1) ];
						gsum += tga[ts]->image.green[ y2*xs + (x2+1) ];
						bsum += tga[ts]->image.blue[ y2*xs + (x2+1) ];
						
						rsum += tga[ts]->image.red[ (y2+1)*xs + x2 ];
						gsum += tga[ts]->image.green[ (y2+1)*xs + x2 ];
						bsum += tga[ts]->image.blue[ (y2+1)*xs + x2 ];
						
						rsum += tga[ts]->image.red[ (y2+1)*xs + (x2+1) ];
						gsum += tga[ts]->image.green[ (y2+1)*xs + (x2+1) ];
						bsum += tga[ts]->image.blue[ (y2+1)*xs + (x2+1) ];
						
						tga[td]->image.red[ y*xh + x ] = rsum / 4;
						tga[td]->image.green[ y*xh + x ] = gsum / 4;
						tga[td]->image.blue[ y*xh + x ] = bsum / 4;
					}
					else
					{

						rsum = tga[ts]->image.red[ y2*xs + x2 ];
						gsum = tga[ts]->image.green[ y2*xs + x2 ];
						bsum = tga[ts]->image.blue[ y2*xs + x2 ];				
						tga[td]->image.red[ y*xh + x ] = 255; //rsum;
						tga[td]->image.green[ y*xh + x ] = 0;//gsum;
						tga[td]->image.blue[ y*xh + x ] = 0; //bsum;		
					}
				}
			}
			xs = xh;
			ys = yh;
		}
		

		printf( "\n" );
		
		break;
		/*
	case TGA_TYPE_INDEXED:
		printf( "tga is indexed./n" );
		arr = ARR_Create( tga->image_width, tga->image_height, 1, arr_ident, NULL );
		CHKPTR( arr );
		printf( "copying ...\n" );
		memcpy( arr->data, tga->image_indexed.data, tga->image_indexed.bytes );
		break;
		*/
	default:
		__error( "we need an uncompressed 24(/8)bit tga.\n" );
		ARR_Free( arr );
		break;
	}
	arrhandle = fopen( arrname, "wb" );
	CHKPTR( arrhandle );
	ARR_Write( arrhandle, arr );
	fclose( arrhandle );
}

