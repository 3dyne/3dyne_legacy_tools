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
#include <signal.h>
#include <unistd.h>

#include "shock.h"
#include "cmdpars.h"
#include "cdb_service.h"
#include "ui_service.h"

#include "arr.h"
#include "pal.h"

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

void main( int argc, char* argv[] )
{
	char*	arrname;
	char*	palname;
	char*	sourcename;
	char*	data[100];
	

	int	asteps;
	int	i, i2;

	int	width, height;

	FILE*	arrhandle;
	FILE*		palhandle;

	arr_t*	arr;

	vid_device_t*	viddev;
	char*		viddata;

	signal( SIGSEGV, (void(*) ()) SecureShutDown );
	signal( SIGABRT, (void(*) ()) SecureShutDown );
	signal( SIGTERM, (void(*) ()) SecureShutDown );
	signal( SIGQUIT, (void(*) ()) SecureShutDown );
	signal( SIGINT,  (void(*) ()) SecureShutDown );

	UI_StartUp( "ui_x11.so" );
//	UI_StartUp( "ui_svgalib.so" );
	viddev = VID_StartUp();
	VID_SetModeByName( 640, 480, 8 );
	KEYB_StartUp();
	
	

	SOS_SetShockHandler( MyShockHandler );

	arrname = GetCmdOpt( "-a", argc, argv );
	__chkptr( arrname );

	//sourcename = GetCmdOpt( "-s", argc, argv );
	//__chkptr( sourcename );

	CDB_StartUp( 0 );
	palname = GetCmdOpt( "-p", argc, argv );
	if( palname == NULL )
	{
		palname = CDB_GetString( "misc/default_pal" );
		if( palname == NULL )
		{
			__error( "couldn't find pal.\n" );
		}
	}
	
	

	palhandle = fopen( palname, "rb" );
	__chkptr( palhandle );

//	asteps = atoi( GetCmdOpt( "-st", argc, argv ));
	arrhandle = fopen( arrname, "rb" );
	arr = ARR_Read( arrhandle );
	asteps = arr -> steps;
	width = arr -> size_x;
	height = arr -> size_y;
	
	for( i = 0; i < asteps; i++ )
	{
		data[i] = ( char* ) malloc( arr -> bytes );
		memcpy( data[i], arr->data+arr->bytes*i, arr->bytes );
	}
	

	VID_LoadPal( palhandle );
	fclose( palhandle );

	viddata = ( char* )VID_GetVirtualPage() -> data;

	for(;;)
	{
		for( i = 0; i < asteps ; i++ )
		{
			/*
			  if( i == 7 )
		  i = 0;
			*/
			
			for( i2 = 0; i2 < height; i2++ )
			{
				memcpy( &viddata[i2*640], &data[i][i2*height], width);
				memcpy( &viddata[i2*640+width], &data[i][i2*height], width);
				memcpy( &viddata[i2*640+640*width], &data[i][i2*height], width);
				memcpy( &viddata[i2*640+640*width+width], &data[i][i2*height], width);
			}
			
			VID_Refresh();
			usleep( 50000 );
			KEYB_Update();
			if( KEYB_KeyPressed( KEYID_ESC ))
				goto the_end;
			
			
		}
		/*
		  for( i = asteps - 2; i >= 1 ; i-- )
		  {
		  
		//	  if( i == 7 )
		//	  i = 0;
		
			
			for( i2 = 0; i2 < height; i2++ )
			{
				memcpy( &viddata[i2*640], &data[i][i2*height], width);
			}
			VID_Refresh();
			usleep( 200000 );
			KEYB_Update();
			if( KEYB_KeyPressed( KEYID_ESC ))
				goto the_end;
				}
		*/
	}

the_end:
	KEYB_ShutDown();
	VID_ShutDown();
	UI_ShutDown();
}
