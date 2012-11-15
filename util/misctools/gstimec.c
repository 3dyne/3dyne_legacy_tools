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



// gstimec.c
/*
  ============================================================================

                                                                   gstimec.c

			   SHAMELESS PIECE OF WORK '98 by sim, garagesoft	

  ============================================================================
*/

// client for the gstimes server.
// i wrote this one, because wether 'timed' is completely crap or
// i was to thumb to configure it. 
// this program sends a request to ssock ( ip of server '-s' on port '-p' ( defaults to 
// 26002 )). the request consists of the 'magic' number 1234 my sockadd an ip-address
// then it waites for the number 4321 and the server date

// ***********************************************************************
//      *************************************************************
//       this is a real GS Lo-Fi application. don't use it at home!! 
//      *************************************************************
// ***********************************************************************

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "cmdpars.h"

#define MAGIC_QUESTION ( 1234 )
#define MAGIC_ANSWER   ( 4321 )



static struct hostent		*hp;
// server socket
static struct sockaddr_in      sadd;
static int			ssock;
static unsigned char           sip[4];

// client socket
static struct sockaddr_in      cadd;
static int			csock;
static unsigned char		mip[4];

static int			l;
static int			magic;

static char			*sname;
static char			*portname;

static int			sport;
static int		sstime;  // seems that there exists a var 'stime' in time.h 

static char    hostname[MAXHOSTNAMELEN];

void Request();

int main( int argc, char* argv[] )
{
	char	*ptr;
	int	del;
	



	sname = GetCmdOpt( "-s", argc, argv );
	if( !sname )
	{
		printf( "missing opt -s ( servername )\n" );
		exit( -1 );
	}
	
	hp = gethostbyname( sname );
	if( !hp->h_addr )
	{
		printf( "gethostbyname: failed ( name: %s )\n", sname );
		exit( -1 );
	}
	memcpy( sip, hp->h_addr, 4 );
	printf( "server ip:\t%i.%i.%i.%i\n",( unsigned char ) sip[0],( unsigned char ) sip[1],( unsigned char ) sip[2], ( unsigned char ) sip[3]);
	

	gethostname( hostname, sizeof( hostname ) );
	hp = gethostbyname( hostname );
	if( !hp->h_addr )
	{
		printf( "gethostbyname: failed ( name: %s )\n", hostname );
		exit( -1 );
	}
	memcpy( mip, hp->h_addr, 4 );

	printf( "client ip:\t%i.%i.%i.%i\n",( unsigned char ) mip[0],( unsigned char ) mip[1],( unsigned char ) mip[2], ( unsigned char ) mip[3]);

	sport = 26002;
	portname = GetCmdOpt( "-p", argc, argv );
	if( portname )
	{
		sport = atoi( portname );
	}
	printf( "server port:\t%d\n", sport );
		
	ssock = socket( AF_INET, SOCK_STREAM, 0 );
	memcpy( ( char* )&sadd.sin_addr, ( char* )sip, 4 );
	sadd.sin_family = AF_INET; 
        sadd.sin_port = htons(sport);
	
	

	del = 60000000; // detting delay to 60 s

	ptr = GetCmdOpt( "-d", argc, argv );
	if( ptr )
		del = atoi( ptr )*1000000;
	
	printf( "delay:\t%d\n", del );
	
	for( ;; )
	{
		Request();
		usleep( del );
	}

}
	
void Request()
{
	int	ltime;
	
	magic = MAGIC_QUESTION;

	
	if( connect( ssock, ( struct sockaddr* ) &sadd, sizeof( sadd )) < 0 )
	{
		printf( "connect: failed\n" );
		return;
	}
/*
	if( sendto( ssock, &magic, sizeof( int ), 0, ( struct sockaddr* ) &sadd, sizeof( sadd ) ) < 0 )
	{
		printf( "client -> server question failed\n" );
		return;
		exit( -1 );
	}
*/
	

	// create your socket                                                   
        csock = socket( AF_INET, SOCK_DGRAM, 0 );
        cadd.sin_addr.s_addr = INADDR_ANY;
        cadd.sin_family = AF_INET;
        cadd.sin_port = 0;

	bind( csock, ( struct sockaddr* ) &cadd, sizeof( cadd ));

	l = sizeof( cadd );
        getsockname( csock, (struct sockaddr* ) &cadd, &l );

//	printf( "my port: %i\n", ntohs(cadd.sin_port) );
	if( sendto( ssock, &cadd, sizeof( cadd ), 0, ( struct sockaddr* ) &sadd, sizeof( sadd )) < 0 )
	{
		printf( "send: failed\n" );
//		exit( -1 );
		return;
	}
	if( sendto( ssock, mip, 4, 0, ( struct sockaddr* ) &sadd, sizeof( sadd ) ) < 0 )
	{
		printf( "send: failed\n" );
		return;
		exit( -1 );
	}

//	printf( "waiting for answer\n" );
	
	if( ( read( csock, &magic, sizeof( int ) )) < 0)
	{
		printf( "server -> client magic failed\n" );
		return;
		exit( -1 );
		
	}
	//printf( "magic: %d\n", magic );
	if( magic != MAGIC_ANSWER )
	{
		printf( "NOT gstimes wating at port %d on %s\n", sport, sname );
		return;
	}

	ltime = time( NULL );

	if( ( read( csock, &sstime, sizeof( int ) )) < 0)
	{
		printf( "server -> client servertime failed\n" );
		return;
		exit( -1 );
		
	}
	printf( "servertime: %d\n", sstime );
/*
	if( (( unsigned int )( sstime - ltime )) > 43200 ) // time hole greater than 12h?
	{
		printf( "very big timehole of x secs detected.\nleaving time alone\n" );
		exit( -1 );
	}
	*/
	if( (( unsigned int )( sstime - ltime )) < 2 )
		return;

	stime( ( time_t* )&sstime ); 
	close( csock );
}
