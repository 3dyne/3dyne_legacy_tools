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



// dllsyms.c

#include "stdio.h"
#include "stdlib.h"
#include "../gs_upi/lib_token.h"

int main( int argc, char *argv[] )
{
	tokenstream_t	*ts;
	int		i;
	ts = BeginTokenStream( argv[1] );

	for(;;)
	{
		if ( GetToken( ts ) == TOKEN_FAIL )
			break;

		if ( strcmp( "FX_CALL", ts->token ) )
			continue;

		GetToken( ts );
		for ( i = 0; i < strlen(ts->token); i++ )
		{
			if ( !isalpha( ts->token[i] ) )
				break;
			printf( "%c", ts->token[i] );
		}
		printf("\n" );
	}
}
