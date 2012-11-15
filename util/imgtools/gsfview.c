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
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <unistd.h>
#include "ui_service.h"
#include "cdb_service.h"
#include "shock.h"
#include "cmdpars.h"
#include "arr.h"

#define	TYPE_UNKNOWN	( 0 )
#define TYPE_ARR	( 1 )
#define TYPE_WAL	( 2 )

typedef struct {
	char	name_junk[32];
	u_int32_t	size_x;
	u_int32_t	size_y;
	u_int32_t	mstart0;
	u_int32_t	mstart1;
	u_int32_t	mstart2;
	u_int32_t	mstart3;
	char		mipmap_junk[44];
}	wal_header_t;

void SecureShutDown( int sig ) {
	
	printf("\nSecureShutDown: shutting down services ...\n");
	if ( sig == SIGSEGV ) {
		printf(" Cause: SIGSEGV.\n");
		printf(" Press <RETURN>\n");
//		X_Flush();
		getchar();
	}
	KEYB_ShutDown();
	VID_ShutDown();
	UI_ShutDown();

	exit(0);
}

void MyShockHandler()
{
	__named_message( "\n" );
	SecureShutDown( 0 );
}

void PrintUsage()
{
	printf( "usage: gsfview -f wal | arr [ -t wal | arr ( force type )] [ -p pal ( force pal )]\n" );
}

void main( int argc, char* argv[] )
{
	char*	pal_name;
	char*	file_name;
	char*	file_suf;

	int	file_type = TYPE_UNKNOWN;
	int	i, i2;
	int	start_x;
	int	data_start;

	vid_device_t*	vid_dev;
	char*		data;
	arr_t*		arr;
	char*		wal_data[4];
	wal_header_t	wal_header;
	FILE*		pal_handle;
	FILE*		in_handle;

	/* Signal handler setzen */
	
	signal( SIGSEGV, (void(*) ()) SecureShutDown );
	signal( SIGABRT, (void(*) ()) SecureShutDown );
	signal( SIGTERM, (void(*) ()) SecureShutDown );
	signal( SIGQUIT, (void(*) ()) SecureShutDown );
	signal( SIGINT,  (void(*) ()) SecureShutDown );

	SOS_SetShockHandler( MyShockHandler );
	file_name = GetCmdOpt( "-f", argc, argv );
	if( file_name == NULL )
	{
		__message( "no filename given.\n" );
		PrintUsage();
		exit( 0 );
	}

	file_suf = strrchr( file_name, '.' );
	if( ( file_suf == NULL ) && ( !CheckCmdSwitch( "-t", argc, argv)  ))
	{
		printf( "filename doesn't seem to have a sufix.\n" );
		PrintUsage();
		exit( 0 );
	}

	file_suf++;
	if( CheckCmdSwitch( "-t", argc, argv ) )
	{
		file_suf = GetCmdOpt( "-t", argc, argv );
	}
	
	printf( "suf: %s\n", file_suf );

	if( ! (memcmp( file_suf, "arr", 3 )))
	{
		file_type = TYPE_ARR;
	} else if( ! (memcmp( file_suf, "wal", 3 )))
	{
		file_type = TYPE_WAL;
	}
	printf( "type: %d\n", file_type );

	CDB_StartUp( 0 );
	UI_StartUp( "ui_x11.so" );
	vid_dev = VID_StartUp();
	VID_SetModeByName( 640, 480, 8 );
	KEYB_StartUp();
	
	pal_name = GetCmdOpt( "-p", argc, argv );
	if( pal_name == NULL )
	{
		pal_name = CDB_GetString( "misc/default_pal" );
		if( pal_name == NULL )
		{
			__error( "couldn't find any palname.\n" );
		}
	}

	printf( "\t\tpal: %s\n", pal_name );
	pal_handle = fopen( pal_name, "rb" );
	VID_LoadPal( pal_handle );
	fclose( pal_handle );

	data = ( char* )VID_GetVirtualPage() -> data;
	in_handle = fopen( file_name, "rb" );
	if( in_handle == NULL )
		__error( "file not found.\n" );

	if( file_type == TYPE_ARR )
	{
		arr = ARR_Read( in_handle ); 
		ARR_Dump( arr );
		if( CheckCmdSwitch( "-d", argc, argv ))
		{
			ARR_Dump( arr );
		}
		start_x = 0;
		data_start = 0;
		for( i = 0; i < arr->mipmap_num; i++ )
		{
//			printf(" i: %d, x: %d, y %d\n", i, arr->size_x, arr->size_y );
			for( i2 = 0; i2 < arr->size_y; i2++ )
			{
				memcpy( &data[i2*640+start_x], &arr->data[i2*arr->size_x+data_start], arr->size_x);
			}
//			printf("\n");
			start_x+=arr->size_x+1;
			data_start += arr->size_x * arr->size_y;
			arr->size_x/=2;
			arr->size_y/=2;
		}
	}
	if( file_type == TYPE_WAL ) // warning: quick and dirty code! todo: make it better.
	{
		fread( &wal_header, sizeof( wal_header_t ), 1, in_handle );
		//wal_tmp = wal_header->size_x*wal_header->size_y;
		//wal_data_size = wal_tmp + wal_tmp/4 + wal_tmp/8 + wal_tmp/16;
		start_x = 0;
		for( i = 0; i < 4; i++ )
		{
			wal_data[i] = ( char* )malloc( wal_header.size_x * wal_header.size_y );
			fread( wal_data[i], wal_header.size_x * wal_header.size_y, 1, in_handle );

			for( i2 = 0; i2 < wal_header.size_y; i2++ )
			{
				memcpy( &data[i2*640+start_x], &wal_data[i][i2*wal_header.size_x], wal_header.size_x );
			}
			
			start_x += wal_header.size_x+1;
			wal_header.size_x/=2;
			wal_header.size_y/=2;
		
		}
	}	

	if( file_type == TYPE_UNKNOWN )
	{
		data[6500] = 127;
	}
	VID_Refresh();
	for(;;)
	{
		usleep( 100000 );
		KEYB_Update();
		if( KEYB_KeyPressed( KEYID_ENTER ))
			break;
	}
	KEYB_ShutDown();
	VID_ShutDown();
	UI_ShutDown();
}
