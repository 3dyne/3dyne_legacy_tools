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



// gl_client.c

#include "gl_client.h"

int	glc_ssock;
struct sockaddr_in	glc_sadd;

void GLC_ConnectServer( char* server )
{
	gls_cmd_t	cmd;

	glc_ssock = socket( AF_INET, SOCK_STREAM, 0 );
	if ( 0 > glc_ssock )
	{
		printf( "socket failed.\n" );
		exit(-1);
	}

	glc_sadd.sin_family = AF_INET;
	glc_sadd.sin_addr.s_addr = htonl( INADDR_LOOPBACK );
	glc_sadd.sin_port = htons( 26002 );

	if ( connect( glc_ssock, (struct sockaddr *) &glc_sadd, sizeof( glc_sadd ) ) < 0 )		
	{
		printf( "connect failed.\n" );
		close( glc_ssock );
		exit(-1);
	}

	cmd = GLS_CMD_CONNECT;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );
}

void GLC_DisconnectServer( void )
{
	gls_cmd_t	cmd;
	
	cmd = GLS_CMD_DISCONNECT;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );
	close( glc_ssock );
}

void GLC_BeginList( char *name, int enable )
{
	gls_cmd_t	cmd;
	char		tmp[16];

	memset( tmp, 0, 16 );
	Error( "untested strange code\n" );
	strncpy( tmp, name, 15 );

	cmd = GLS_CMD_BEGINLIST;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );
	send( glc_ssock, &enable, 4, 0 );
	send( glc_ssock, tmp, 16, 0 );	
}

void GLC_EndList( void )
{
	gls_cmd_t	cmd;

	cmd = GLS_CMD_ENDLIST;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );
}

void GLC_DrawPolygon( polygon_t *in )
{
	gls_cmd_t	cmd;
	int		i;

	if ( !in )
		return;

	cmd = GLS_CMD_POLYGON;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );

	send( glc_ssock, &in->pointnum, sizeof( int ), 0 );
	for ( i = 0; i < in->pointnum; i++ )
	{
		send( glc_ssock, &in->p[i][0], sizeof( fp_t ), 0 );
		send( glc_ssock, &in->p[i][1], sizeof( fp_t ), 0 );
		send( glc_ssock, &in->p[i][2], sizeof( fp_t ), 0 );
	}
}

void GLC_SetColor( int glc_color )
{
	gls_cmd_t	cmd;

	static int last_color = -1;

	if ( glc_color != GLC_COLOR_MAGIC_CLEAR_LISTS )
	{
		if ( glc_color == last_color )
			// drop traffic
			return;
		
		last_color = glc_color;
	}

	cmd = GLS_CMD_SETCOLOR;
	send( glc_ssock, &cmd, GLS_CMD_SIZE, 0 );
	send( glc_ssock, &glc_color, sizeof ( int ), 0 );
}

#if 0
int main()
{
	polygon_t	*poly;
	
	vec3d_t		v = { 0, 0, 1 };
	fp_t		d = 1.0;

	int		i;

	poly = BasePolygonForPlane( v, d );
	for ( i = 0; i < 4; i++ )
		Vec3dScale( poly->p[i], 0.5/8192.0, poly->p[i] );

	GLC_ConnectServer( "" );
	GLC_BeginList();
	GLC_DrawPolygon( poly );
	GLC_EndList();
	GLC_DisconnectServer();

	sleep( 1 );

	GLC_ConnectServer( "" );
	GLC_BeginList();
	GLC_DrawPolygon( poly );
	GLC_EndList();
	GLC_DisconnectServer();
}
#endif
