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



// lib_token.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lib_error.h"
#include "lib_token.h"

#define		CHAR_TAB	( 0x9 )
#define		CHAR_NEWLINE	( 0xa )
#define		CHAR_CR		( 0xd )
#define		CHAR_SPACE	( 0x20 )
#define		CHAR_QUOTA	( '\"' )

//static FILE	*t_handle = NULL;
//char		token[MAX_TOKEN_LEN];


tokenstream_t* BeginTokenStream( const char * name )
{
	tokenstream_t	*s;

	s = (tokenstream_t *) malloc( sizeof( tokenstream_t ) );
	memset( s, 0, sizeof( tokenstream_t ) );

	s->handle = fopen( name, "r" );

	if ( !s->handle )
	{
		free( s );
//		Error( "BeginTokenStream: can't open %s.\n", name );
		return NULL;
	}

	s->keep = 0;
	return s;
}

void EndTokenStream( tokenstream_t *stream )
{
	fclose ( stream->handle );
	free( stream );
}

int SkipLine( tokenstream_t *stream )
{
	int		c;

	for(;;)
	{
		c = fgetc( stream->handle );
		if ( c == EOF )
			return TOKEN_FAIL;
		if ( c == '\n' )
			return TOKEN_OK;
	}
}

int GetToken( tokenstream_t *stream ) {

	int		c;
	int		pos;
	int		quota;

	if ( !stream->handle )
		Error( "ParseToken: no open stream.\n" );

	if ( stream->keep )
	{
		stream->keep = 0;
		return TOKEN_OK;
	}

	pos = 0;
	quota = 0;

// skipping whitespaces       
	for(;;) {

		c = fgetc( stream->handle );

		if ( c == EOF )
			return TOKEN_FAIL;

		// skipping comment lines
		if ( c == '#' )
		{
			for(;;)
			{
				c = fgetc( stream->handle );
				if ( c == EOF )
					return TOKEN_FAIL;
				if ( c == '\n' )
					break;
			}
			continue;
		}

		if ( c == CHAR_QUOTA ) {
			quota = 1;	
			c = fgetc( stream->handle );
			break;
		}
		
		if ( c != CHAR_TAB && c != CHAR_NEWLINE && c != CHAR_SPACE && c != CHAR_CR )
			break;
	}

// copy till whitespace
	for(;;) {
		
		if ( c == EOF )
			break;
		
		if ( c == CHAR_QUOTA && quota == 1 )
			break;
		
		if ( ( c == CHAR_SPACE || c == CHAR_TAB || c == CHAR_NEWLINE || c == CHAR_CR ) && quota == 0 )
			break;	       
		
		stream->token[pos] = (char) c;
		pos++;
		
		c = fgetc( stream->handle );
		
	}
	
	stream->token[pos] = 0;

//	printf( "%s$\n", stream->token );

	return TOKEN_OK;
}

void KeepToken( tokenstream_t *stream )
{
	stream->keep = 1;
}
