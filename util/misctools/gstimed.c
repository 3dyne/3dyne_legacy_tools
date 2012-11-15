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
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#define MAGIC_QUESTION ( 1234 )
#define MAGIC_ANSWER   ( 4321 )

int main( int argc, char* argv[] )
{
	struct hostent		*hp;
	// server socket
	struct sockaddr_in      sadd;
	int			ssock;
	unsigned char           sip[4];

	// client socket
	struct sockaddr_in      cadd;
	int			csock;

	int			l;
	int			magic;
	
	char			*sname;
	char			*portname;

	int			port;

	sname = GetCmdOpt( "-s", argc, argv );
	if( !sname )
	{
		printf( "missing opt -s ( servername )\n" );
		exit( -1 );
	}
	
	hp = gethostbyname( sname );
	if( !hp )
	{
		printf( "gethostbyname: failed ( name: %s )\n", sname );
		error( -1 );
	}
}
	
