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



// pal.c

// the tool version

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "filelumps.h"
#include "pal.h"

// local globals

// fix me: pow_tab is limited to 256 colors

static double pow_tab[256];
static pow_tab_init = 0;

// local functions
static void InitPowTab() {

	int		i;

	printf("InitPowTab.\n");

	for ( i=0; i<256; i++) {
		pow_tab[i] = pow(10.0, ((double) i)/(256.0/5.0) ); 
	}	
	pow_tab_init = 1;
}

// global functions

pal_t *PAL_Create( unsigned int rgb_num ) {
	pal_t		*pal;	

	pal = ( pal_t* ) malloc( sizeof( pal_t ) );
	pal -> rgb_num = rgb_num;
	pal -> rgb_set = ( rgb_t* ) malloc( sizeof( rgb_t ) * pal -> rgb_num );

	return pal;
}

pal_t *PAL_Read( FILE *handle ) {

	int		i;
	pal_t		*pal;
	pal_header_l	header;
	rgb_l	       	rgb;

	pal = ( pal_t* ) malloc( sizeof( pal_t ) );

       	fread( &header, sizeof( pal_header_l ), 1, handle );
	if( memcmp( header.id, PAL_ID, 4 )) {
		printf( "wrong id in pal.\n" );
		exit( -1 );
	}
		
	pal -> rgb_num = header.rgb_num;
	pal -> rgb_set = ( rgb_t* ) malloc( sizeof( rgb_t ) * pal -> rgb_num );

	for ( i=0; i<pal -> rgb_num; i++) {
		fread( &rgb, sizeof( rgb_l ), 1, handle );
		pal -> rgb_set[i].red =		rgb.red;
		pal -> rgb_set[i].green =	rgb.green;
		pal -> rgb_set[i].blue =	rgb.blue;
	}      

	return pal;
}

void PAL_Free( pal_t *pal ) {

	free( pal -> rgb_set );
	free( pal );
	pal = NULL;
}

void PAL_Dump( pal_t *pal ) {

	printf("PAL_Dump:\n");
	printf(" rgb entries: %d\n", pal -> rgb_num );
	printf(" rgb set at : %p\n", pal -> rgb_set );

}

unsigned int PAL_ReduceColor( pal_t *pal, rgb_t *rgb ) {

//	double		akt;
//	double		best;
	int		rdiff, gdiff, bdiff;
	int		akt, best;

	unsigned int	best_i;
	unsigned int	i;

	if ( pow_tab_init == 0 ) {
		InitPowTab();
	}

	best = 0x7fffffff;
	best_i = 0;

	for ( i = 0; i < pal -> rgb_num; i++ ) {

#if 0		
		akt = ( pow_tab[ abs( rgb -> red   - pal -> rgb_set[i].red   )] +
			pow_tab[ abs( rgb -> green - pal -> rgb_set[i].green )] +
			pow_tab[ abs( rgb -> blue  - pal -> rgb_set[i].blue  )] );
#else
		rdiff = (int)rgb -> red   - (int)pal -> rgb_set[i].red;
		gdiff = (int)rgb -> green - (int)pal -> rgb_set[i].green;
		bdiff = (int)rgb -> blue  - (int)pal -> rgb_set[i].blue;

		akt = rdiff*rdiff + gdiff*gdiff + bdiff*bdiff;
#endif

		if( akt < best )
		{
			best = akt;
			best_i = i;
		}		
	}

	return best_i;
} 

//#define USE_PAL_MAIN

#ifdef USE_PAL_MAIN

int main() {

	pal_t	*pal;
	FILE*	in_handle;

	in_handle = fopen( "doom01.pal", "r" );

	pal = PAL_Load( in_handle );
	PAL_Dump( pal );
	PAL_Free( pal );
	getchar();
	
}

#endif
