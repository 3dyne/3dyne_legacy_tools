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



//tga.c

#include <stdio.h>
#include <stdlib.h>

#define __TGA_C

#include "tga.h"

tga_t *TGA_Create( unsigned short width, unsigned short height, unsigned char image_type ) {

	tga_t	*tga;

	tga = ( tga_t* ) malloc( sizeof( tga_t ) );

	tga -> ident_len	= 0;
	tga -> cmap_type	= 0;
	tga -> image_type	= image_type;
	tga -> cmap_origin	= 0;
	tga -> cmap_len		= 0;
	tga -> centry_size	= 0;
	tga -> image_xorg	= 0;
	tga -> image_yorg	= 0;
	tga -> image_width	= width;
	tga -> image_height	= height;
	tga -> pixel_size	= 24;
	tga -> image_discr	= 32;

	if ( image_type == TGA_TYPE_TRUECOLOR ) {
		tga -> image.pixels	= (tga -> image_width) * (tga -> image_height);
		tga -> image.bytes	= tga -> image.pixels * ((tga -> pixel_size) / 8);
		
		tga -> image.red	= (unsigned char*) malloc( tga -> image.pixels );
		tga -> image.green	= (unsigned char*) malloc( tga -> image.pixels );
		tga -> image.blue	= (unsigned char*) malloc( tga -> image.pixels );
		
		if ( tga -> image.red == NULL || 
		     tga -> image.green == NULL ||
		     tga -> image.blue == NULL ) {
			
			printf("Can't alloc image.\n");
			exit(0);
		}       
	}

	if ( image_type == TGA_TYPE_INDEXED ) {
		tga -> image_indexed.pixels	= (tga -> image_width) * (tga -> image_height);
		tga -> image_indexed.bytes	= tga -> image.pixels * ((tga -> pixel_size)/ 8);
		tga -> image_indexed.pal	= PAL_Create( tga -> cmap_len );
		tga -> image_indexed.data	= (unsigned char*) malloc( tga -> image_indexed.pixels );
		
	}

	return tga;

}

tga_t *TGA_Read( FILE *tga_handle ) {

	tga_t		*tga;
	int		i;

	tga = ( tga_t* ) malloc( sizeof( tga_t ) );
	
	tga -> ident_len       	= fgetc( tga_handle );
	tga -> cmap_type       	= fgetc( tga_handle );
	tga -> image_type	= fgetc( tga_handle );
	tga -> cmap_origin	= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> cmap_len		= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> centry_size	= fgetc( tga_handle );
	tga -> image_xorg	= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> image_yorg	= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> image_width	= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> image_height	= fgetc( tga_handle ) + (fgetc( tga_handle )<<8);
	tga -> pixel_size	= fgetc( tga_handle );
	tga -> image_discr	= fgetc( tga_handle );

	// ignore ident
	
	if ( tga -> ident_len > 0 ) {
		fseek( tga_handle, tga -> ident_len, SEEK_CUR );
	}

	if ( tga -> image_type == TGA_TYPE_TRUECOLOR ) {
		
		// ignore cmap

		if ( tga -> cmap_len > 0 ) {
			fseek( tga_handle, 
			       (tga -> cmap_len) * ((tga -> centry_size) / 8), SEEK_CUR );
		}

		tga -> image.pixels	= (tga -> image_width) * (tga -> image_height);
		tga -> image.bytes	= tga -> image.pixels * ((tga -> pixel_size) / 8);

		tga -> image.red	= (unsigned char*) malloc( tga -> image.pixels );
		tga -> image.green	= (unsigned char*) malloc( tga -> image.pixels );
		tga -> image.blue	= (unsigned char*) malloc( tga -> image.pixels );

		if ( tga->pixel_size == 32 )
			tga->image.alpha = (unsigned char*) malloc( tga -> image.pixels );
		else
			tga->image.alpha = NULL;
		
		if ( tga -> image.red == NULL || 
		     tga -> image.green == NULL ||
		     tga -> image.blue == NULL ) {
			
			printf("Can't alloc image.\n");
			exit(0);
		}       
		
		for ( i = 0; i < tga -> image.pixels; i++ ) {
			
			tga -> image.blue[i]	= fgetc( tga_handle );
			tga -> image.green[i]	= fgetc( tga_handle );
			tga -> image.red[i]	= fgetc( tga_handle );
			
			if ( tga->pixel_size == 32 )
				tga->image.alpha[i] = fgetc( tga_handle );
		}	
		
	}

	if ( tga -> image_type == TGA_TYPE_INDEXED ) {
		tga -> image_indexed.pal = PAL_Create( tga -> cmap_len );
		for ( i = 0; i < tga -> cmap_len; i++ ) {	
			tga -> image_indexed.pal -> rgb_set[i].red	= fgetc( tga_handle );
			tga -> image_indexed.pal -> rgb_set[i].green	= fgetc( tga_handle );
			tga -> image_indexed.pal -> rgb_set[i].blue    	= fgetc( tga_handle );
		}

		tga -> image_indexed.pixels	= (tga -> image_width) * (tga -> image_height);
		tga -> image_indexed.bytes	= tga -> image.pixels * ((tga -> pixel_size)/ 8);
		tga -> image_indexed.data	= (unsigned char*) malloc( tga -> image_indexed.pixels );
		
		
		for ( i = 0; i < tga -> image_indexed.pixels; i++ ) {
			tga -> image_indexed.data[i]	= fgetc( tga_handle );	
		}	
		
	}
	
	return tga;
		
}

void TGA_Write( FILE* tga_handle, tga_t *tga ) {

	int		i;

	fputc( tga -> ident_len, tga_handle );
	fputc( tga -> cmap_type, tga_handle );
	fputc( tga -> image_type, tga_handle );
	fwrite( &tga -> cmap_origin, 2, 1, tga_handle );
	fwrite( &tga -> cmap_len, 2, 1, tga_handle );
	fputc( tga -> centry_size, tga_handle );
	fwrite( &tga -> image_xorg, 2, 1, tga_handle );
	fwrite( &tga -> image_yorg, 2, 1, tga_handle );
	fwrite( &tga -> image_width, 2, 1, tga_handle );
	fwrite( &tga -> image_height, 2, 1, tga_handle );
	fputc( tga -> pixel_size, tga_handle );
	fputc( tga -> image_discr, tga_handle );

	if ( tga -> image_type == TGA_TYPE_TRUECOLOR ) {

		for ( i = 0; i < tga -> image.pixels; i++ ) {
			fputc( tga -> image.blue[i], tga_handle );
			fputc( tga -> image.green[i], tga_handle );
			fputc( tga -> image.red[i], tga_handle );
		}	
	}

	if ( tga -> image_type == TGA_TYPE_INDEXED ) {
		
		for ( i = 0; i < tga -> cmap_len; i++ ) {
			fputc( tga -> image_indexed.pal -> rgb_set[i].red, tga_handle );
			fputc( tga -> image_indexed.pal -> rgb_set[i].green, tga_handle );
			fputc( tga -> image_indexed.pal -> rgb_set[i].blue, tga_handle );
		}
		
		for ( i = 0; i < tga -> image_indexed.pixels; i++ ) {
			fputc( tga -> image_indexed.data[i], tga_handle );
		}

	}
}

void TGA_Dump( tga_t *tga ) {

	printf("TGA_DumpHeader:\n");

	printf(" ident_len   : %d\n", tga -> ident_len);
	printf(" cmap_type   : %d\n", tga -> cmap_type);
	printf(" image_type  : %d\n", tga -> image_type);
	printf(" cmap_origin : %d\n", tga -> cmap_origin);
	printf(" cmap_len    : %d\n", tga -> cmap_len);
	printf(" centry_size : %d\n", tga -> centry_size);
       	printf(" image_xorg  : %d\n", tga -> image_xorg);
	printf(" image_yorg  : %d\n", tga -> image_yorg);
	printf(" image_width : %d\n", tga -> image_width);
	printf(" image_height: %d\n", tga -> image_height);
	
	printf(" pixel_size  : %d\n", tga -> pixel_size);
	printf(" image_discr : %d\n", tga -> image_discr);

	printf("      -Extended-\n");

	printf(" pixels      : %d\n", tga -> image.pixels);
	printf(" bytes       : %d\n", tga -> image.bytes);
	printf(" red at      : %p\n", tga -> image.red);
	printf(" green at    : %p\n", tga -> image.green);
	printf(" blue at     : %p\n", tga -> image.blue);	
}

void TGA_Free( tga_t *tga ) {

	if ( tga -> image_type == TGA_TYPE_TRUECOLOR ) {
		free( (void*) tga -> image.red );
		free( (void*) tga -> image.green );
		free( (void*) tga -> image.blue );
		if ( tga->pixel_size == 32 )
			free( (void*) tga -> image.alpha );
	}
	if ( tga -> image_type == TGA_TYPE_INDEXED ) {
		PAL_Free( tga -> image_indexed.pal );
		free( (void*) tga -> image_indexed.data );
	}
	free( (void*) tga );
}

// #define WITH_TGA_MAIN

#ifdef WITH_TGA_MAIN

int main() {

	tga_t	*tga_r;
	tga_t	*tga_c;
	tga_t	*tga_w;

	FILE	*in_handle;
	FILE	*out_handle;

	in_handle = fopen("w_mwb.tga", "r");

	tga_r = TGA_Read( in_handle );
	TGA_Dump( tga_r );

	fclose( in_handle );
	getchar();

//	TGA_CreateHeader( &tga_c, 640, 480);
//	TGA_DumpHeader( &tga_c );


	out_handle = fopen("testout.tga", "w");
	TGA_Write( out_handle, tga_r );
	fclose( out_handle );
	getchar();       	
}

#endif
