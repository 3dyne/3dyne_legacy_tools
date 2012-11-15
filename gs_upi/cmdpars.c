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



// cmdpars.c
#include <stdio.h>
#include <string.h>
#include "cmdpars.h"

int CheckCmdSwitch( const char* switch_name, int arg_argc, char* arg_argv[] )
{
	unsigned int	i;

	if( arg_argv[1] == NULL )
	{
		//printf( "NULL pointer\n" );
		return( 0 );
	}

	for( i = 1; i < arg_argc; i++ )
	{
		if( strlen( switch_name ) != strlen( arg_argv[i] ))
		{
			//printf( "wrong lenght\n" );
			continue;
		}
		if( ! strcmp( switch_name, arg_argv[i] ) )
		{
			return( i );
		}
	}
	return( 0 );
}

char*	GetCmdOpt( const char* switch_name, int arg_argc, char* arg_argv[] )
{
	unsigned int	i;

	if( arg_argv[1] == NULL )
	{
		//printf( "NULL pointer\n" );
		return( NULL );
	}

	for( i = 1; i < arg_argc; i++ )
	{
		if( strlen( switch_name ) != strlen( arg_argv[i] ))
		{
			//printf( "wrong lenght\n" );
			continue;
		}
		if( ! strcmp( switch_name, arg_argv[i] ) )
		{
			if( i + 1 == arg_argc )
			{
				return( NULL );
			}
			return( arg_argv[i + 1] );
		}
	}
	return( NULL );
}

//
// mcb stuff
//

static int	local_argc;
static char	**local_argv;

/*
  ====================
  SetCmdArgs

  ====================
*/
void	SetCmdArgs( int argargc, char **argargv )
{
	local_argc = argargc;
	local_argv = argargv;
}


/*
  ====================
  CheckCmdSwitch2

  ====================
*/
int	CheckCmdSwitch2( const char *name )
{
	return CheckCmdSwitch( name, local_argc, local_argv );
}


/*
  ====================
  GetCmdOpt2

  ====================
*/
char*	GetCmdOpt2( const char* name )
{
	return GetCmdOpt( name, local_argc, local_argv );
}

/*
int	main( int argc, char* argv[] )
{
	unsigned int	arg_num;
	char*	temp_buf;
	arg_num = CheckCmdSwitch( "-bla", argc, argv );
	//printf( "arg num: %i\n", arg_num );
	if( arg_num == 0 )
	{ 
		printf( "switch not set.\n" );
	}
	else
	{
		printf( "switch set as arg %i.\n", arg_num );
	}

	temp_buf = GetCmdOpt( "-file", argc, argv );
	if( temp_buf == NULL )
	{
		printf( "arg not found.\n" );
	}
	printf( "temp buf: %s\n", temp_buf );

}
*/
