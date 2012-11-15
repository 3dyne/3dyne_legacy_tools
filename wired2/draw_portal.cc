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



// draw_portal.c

#include "draw_portal.h"

void Draw_Hit( void );

void Draw_Portals( char *prodir )
{
	int		i, pointnum;
	vec3d_t		p[32];

	tokenstream_t	*ts;
	char		tmp[256];



	sprintf( tmp, "%s/portals", prodir );

	ts = BeginTokenStream( tmp );

	if ( !ts )
	{
		printf( "Draw_Portal: can't open portal file.\n" );
		return;
	}

	XZStartDraw();
	YStartDraw();
	CameraStartDraw();

	XZColor( colorgreen_i );
	YColor( colorgreen_i );	

	for(;;)
	{
		GetToken( ts );
		if ( !strcmp( ts->token, "end" ) )
			break;

		pointnum = atoi( ts->token );

		if ( pointnum > 32 )
		{
			printf( "pointnum > 32" );
			goto portal_error;
		}

		GetToken( ts );
		if ( ts->token[0] != ':' )
			goto portal_error;

		for( i = 0; i < pointnum; i++ )
		{

			// '('
			GetToken( ts );
			if ( ts->token[0] != '(' )
				goto portal_error;

			GetToken( ts );
			p[i][0] = atof( ts->token );
			GetToken( ts );
			p[i][1] = atof( ts->token );
			GetToken( ts );
			p[i][2] = atof( ts->token );
			
			GetToken( ts );
			if ( ts->token[0] != ')' )
				goto portal_error;
			
		}

		for ( i = 0; i < pointnum; i++ )
		{
			XZDrawLine( p[i], p[(i+1)%pointnum] );
			YDrawLine( p[i], p[(i+1)%pointnum] );
		}

	}

	CameraEndDraw();
	XZEndDraw();
	YEndDraw();

	EndTokenStream( ts );

	return;

portal_error:

	printf("parse error in portal file.\n" );
	return;
}

#define		MAX_POINTS	( 10000 )
void Draw_Trace( char *prodir )
{
	int		i, pointnum;
	tokenstream_t	*ts;
	vec3d_t		p[MAX_POINTS];
	char		tmp[256];

	Draw_Portals( prodir );

	sprintf( tmp, "%s/trace", prodir );

	ts = BeginTokenStream( tmp );
	if ( !ts )
	{
		printf( "Draw_Portal: can't open trace file.\n" );
		return;
	}


	for( i = 0; i < MAX_POINTS; i++ )
	{
		GetToken( ts );
		if ( !strcmp( ts->token, "end" ) )
			break;

		if ( ts->token[0] != '(' )
			goto portal_error;

		GetToken( ts );
		p[i][0] = atof( ts->token );
		GetToken( ts );
		p[i][1] = atof( ts->token );
		GetToken( ts );
		p[i][2] = atof( ts->token );
			
		GetToken( ts );
		if ( ts->token[0] != ')' )
			goto portal_error;
	}
		

	XZStartDraw();
	YStartDraw();
	CameraStartDraw();
		
	XZColor( colorred_i );
	YColor( colorred_i );	
		
//	CameraDrawPolygon( p, pointnum );

	if ( i == MAX_POINTS )
	{
		printf( "Draw_Trace: maybe more than MAX_POINTS points.\n" );
	}

	pointnum = i;		
	for ( i = 0; i < pointnum-1; i++ )
	{
			XZDrawLine( p[i], p[i+1] );
			YDrawLine( p[i], p[i+1] );
			CameraDrawLine( p[i], p[i+1] );
	}

	CameraEndDraw();
	XZEndDraw();
	YEndDraw();
		

#if 0
	CameraEndDraw();
	XZEndDraw();
	YEndDraw();
#endif

	EndTokenStream( ts );


	return;

portal_error:

	printf("parse error in trace file.\n" );
	return;
}


void Draw_Hit( void )
{
	int i;
	tokenstream_t	*ts;

	vec3d_t		p[2];

	ts = BeginTokenStream( "hit" );

	if ( !ts )
	{
		printf( "Draw_Hit: can't open hit file.\n" );
		return;
	}

	XZStartDraw();
	YStartDraw();
	
	XZColor( colorred_i );
	YColor( colorred_i );	
		
	for(;;)
	{
		GetToken( ts );
		if ( !strcmp( ts->token, "end" ) )
			break;

		if ( ts->token[0] != ':' )
			goto hit_error;

		for( i = 0; i < 2; i++ )
		{

			// '('
			GetToken( ts );
			if ( ts->token[0] != '(' )
				goto hit_error;
			
			GetToken( ts );
			p[i][0] = atof( ts->token );
			GetToken( ts );
			p[i][1] = atof( ts->token );
			GetToken( ts );
			p[i][2] = atof( ts->token );
			
			GetToken( ts );
			if ( ts->token[0] != ')' )
				goto hit_error;
			
		}
		
		for ( i = 0; i < 2; i++ )
		{
			XZDrawLine( p[0], p[1] );
			YDrawLine( p[0], p[1] );
		}

	}

#if 1
	XZEndDraw();
	YEndDraw();
#endif

	EndTokenStream( ts );



	return;

hit_error:

	printf("parse error in hit file.\n" );
	return;




}
