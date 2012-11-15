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
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "cmdpars.h"

#define MAGIC_QUESTION ( 1234 )
#define MAGIC_ANSWER   ( 4321 )

struct hostent*         hp;
// control socket
struct sockaddr_in      sadd;
int			ssock;
int			l;
unsigned int		magic;

unsigned char		cip;
struct sockaddr_in      cadd;
int			csock;


void Listen();

int main( int argc, char* argv[] )
{
	int	sport;
	char	*ptr;

	sport = 26002;
	
	ptr = GetCmdOpt( "-p", argc, argv );
	if( ptr )
	{
		sport = atoi( ptr );
	}

	// create listening socket
	ssock = socket( AF_INET, SOCK_DGRAM, 0 );
	sadd.sin_family = AF_INET;
	sadd.sin_addr.s_addr = INADDR_ANY;
	sadd.sin_port = htons( sport );

	// bind socket to address
	if( bind( ssock, ( struct sockaddr* ) &sadd, sizeof( sadd )) < 0)
	{
		printf( "bind: cannot bind control socket\n" );
		exit( -1 );
	}
	
	l = sizeof( ssock );
	if( getsockname( ssock, ( struct sockaddr* ) &sadd, &l) < 0 )	
	{
		printf( "getsockname: failed\n" );
		exit( -1 );
	}
	printf( "ssock port num %i\n", ntohs( sadd.sin_port ) );

	for( ;; )
	{
		Listen();
	}

}

void Listen()
{
	int		stime;

	if( ( read( ssock, &magic, sizeof( int ))) < 0)
	{
		printf( "read: failed\n" );
		return;
		exit( -1 );	
		
	}
//	printf( "magic: %d\n", magic );
	if( magic != MAGIC_QUESTION )
	{
		printf( "request without right magic number.\nbumming\n" );
		
		return;
	}

	if( ( read( ssock, &cadd, sizeof( int ))) < 0)
	{
		printf( "read: failed\n" );
		return;
		exit( -1 );
	}

	if( ( read( ssock, &cadd.sin_addr, sizeof( int ))) < 0)
	{
		printf( "read: failed\n" );
		return;
		exit( -1 );
	}
	

	csock = socket( AF_INET, SOCK_DGRAM, 0 ); 

	if( connect( csock, ( struct sockaddr* ) &cadd, sizeof( cadd )) < 0 )
	{
		printf( "connect: failed\n" );
		return;
		exit( -1 );
	}


	magic = MAGIC_ANSWER;
	if( send( csock, &magic, sizeof( int ), 0) < 0 )
	{
		printf( "send: failed\n" );
//		close( csock );
		return;
		exit( -1 );
	}

	stime = time( NULL);
//	printf( "servetime: %d\n", stime );
	if( send( csock, &stime, sizeof( int ), 0) < 0 )
	{
		printf( "send: failed\n" );
		//	close( csock );
		return;
		exit( -1 );
	}
	
	

	close( csock );

}
