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



// arr.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "arr.h"
#include "lib_error.h"
#include "lib_packed.h"
//#include "filelumps.h"

arr_t *ARR_Create( unsigned short size_x, unsigned short size_y, unsigned short mipmap_num, unsigned short steps, const char* ident, unsigned int flags ) 
{
	int		i, size;
	arr_t		*arr;
	
	arr = ( arr_t* ) malloc( sizeof( arr_t ) );
	
	arr -> size_x = size_x;
	arr -> size_y = size_y;
	arr -> mipmap_num = mipmap_num;
	arr -> steps = steps;
	arr -> flags = flags;

	if( steps == 0 )
		printf( "creating arr with null steps./n" );

	if( ident != NULL )
	{
//		if( strlen( ident ) >= 32 )
//		{
//			ident[31] = '\0';
//		}
		strncpy( arr -> ident, ident, 31 );
		arr -> ident[31] = '\0';
	} else
	{
		memset( arr -> ident, 0, 32 );
	}
	

	
	size = 0;
	for( i = 0; i < mipmap_num; i++ )
	{
		size+= size_x * size_y;
		size_x/=2;
		size_y/=2;
	}
	if( arr->flags == ARR_F_RGB565 )
		size*=2;
		
	arr -> bytes = size;
	arr -> data = ( unsigned char* ) malloc( size * steps );

	return arr;
}


arr_t *ARR_Read( FILE* in_handle ) 
{
// ====================================
// testing lib_packed !
// ====================================
	int		i, size;
	int		size_x, size_y;
	arr_t		*arr;
	char		buf[ARR_HEADERSIZE];
	char		id[4];
	short		tmp;


	arr = ( arr_t* ) malloc( sizeof( arr_t ) );
	printf( "ARR_Read\n" );

	printf( "handle = %p\n", in_handle );
		
	fread( buf, ARR_HEADERSIZE, 1, in_handle );

	BeginUnpack( 0, buf, ARR_HEADERSIZE );

	UnpackString( id, 4 );
	printf( "id = %s\n", id );

	if( memcmp( id, ARR_ID, 4 ) ) {
		printf( "wrong id in arr.\n" );
		exit( -1 );
	}		
	
	// I hate 16bit!
	UnpackString( arr -> ident, 32 );

	printf( "ident = %s\n", arr->ident );

	Unpacks16( &tmp );
	arr->mipmap_num = ( int )tmp;

	printf( "mm = %d\n", arr->mipmap_num );

	Unpacks16( &tmp );
	arr->size_x = ( int )tmp;
	printf( "sizex = %d\n", arr->size_x );

	Unpacks16( &tmp );
	arr->size_y = ( int )tmp;
	printf( "sizey = %d\n", arr->size_y );

	Unpacks16( &tmp );
	printf( "steps = %d\n", ( int ) tmp );
	arr->steps = ( int )tmp;

	Unpacks32( (int*)&(arr->flags) );
	printf( "flags = %d\n", arr->flags );
	
	size = 0;
	size_x = arr -> size_x;
	size_y = arr -> size_y;
	if( arr -> steps == 0 )
	{
		free( arr );
		Error( "steps == 0\n" );
		exit( -1 );
	}

	for( i = 0; i < (int) arr->mipmap_num; i++ )
	{
		size+= size_x * size_y;
		size_x/=2;
		size_y/=2;
	}

	if( arr->flags == ARR_F_RGB565 )
		size*=2;

	arr -> data = ( unsigned char* ) malloc( size * arr -> steps );
	// fix me: use __chkptr!
	if ( arr -> data == NULL ) {
		Error(" can't alloc data.\n");
		exit(0);
	}
	arr -> bytes = size;

	fread( arr -> data, size * arr -> steps, 1, in_handle );
	
	printf( "ARR_Read: ptr: %p\n", arr );

	return arr;	
}

#ifdef __arr_ib_ext

arr_t *ARR_ReadIB( ib_file_t* in_handle ) {

	int		i, size;
	int		size_x, size_y;
	arr_t		*arr;
	arr_header_l	header;

	arr = ( arr_t* ) malloc( sizeof( arr_t ) );
	
	IB_Read( &header, sizeof( arr_header_l ), 1, in_handle );
	if( memcmp( header.id, ARR_ID, 4 ) ) {
		printf( "wrong id in arr.\n" );
		exit( -1 );
	}		
	
	memcpy( arr -> ident, header.ident, 32 );
	arr -> mipmap_num =	header.mipmap_num;
	arr -> size_x =		header.size_x;
	arr -> size_y =		header.size_y;
	if( arr -> steps == 0 )
	{
		free( arr );
		return NULL;
	}

	size = 0;
	size_x = arr -> size_x;
	size_y = arr -> size_y;
	for( i = 0; i < arr->mipmap_num; i++ )
	{
		size+= size_x * size_y;
		size_x/=2;
		size_y/=2;
	}

	arr -> data = ( unsigned char* ) malloc( size * arr -> steps );
	// fix me: use __chkptr!
	if ( arr -> data == NULL ) {
		printf(" can't alloc data.\n");
		exit(0);
	}
	arr -> bytes = size;

	IB_Read( arr -> data, size, 1, in_handle );

	return arr;	
}

#endif

void ARR_Write( FILE* out_handle, arr_t *arr ) {
	
	arr_header_l	header;
	memcpy( header.id, ARR_ID, 4 );
	memcpy( header.ident, arr -> ident, 32 );
	header.size_x = arr -> size_x;
	header.size_y = arr -> size_y;
	header.mipmap_num = arr -> mipmap_num;
	header.steps = arr -> steps;
	header.flags = arr->flags;
     
	fwrite( &header, sizeof( arr_header_l ),1 , out_handle );

	fwrite( arr -> data, arr -> bytes * arr -> steps, 1, out_handle );
}

void ARR_Free( arr_t *arr ) {

	free( arr -> data );
	free( arr );

	arr = NULL;
}

void ARR_Dump( arr_t *arr ) {

	printf("ARR_Dump:\n");
	printf(" width:      %d\n", arr -> size_x );
	printf(" height:     %d\n", arr -> size_y );
	printf(" mipmap_num: %d\n", arr -> mipmap_num );
	printf(" steps:      %d\n", arr -> steps ); 
	printf(" ident:   %s\n", arr -> ident );
	printf(" flags: %s\n", arr->flags == ARR_F_RGB565 ? "rgb565" : "p8" );
	printf(" data at:    %p\n", arr -> data );
}

//#define WITH_ARR_MAIN

#ifdef WITH_ARR_MAIN

int main() {
	FILE*	in_handle;
	FILE*	out_handle;
	arr_t	*arr;
	
	in_handle = fopen( "wm_gates.arr", "r" );
	arr = ARR_Read( in_handle );
	ARR_Dump( arr );
	ARR_Free( arr );
	fclose( in_handle );
	out_handle = fopen( "testout.arr", "w" );
	ARR_Write( out_handle, arr );
	fclose( out_handle );	
}

#endif
