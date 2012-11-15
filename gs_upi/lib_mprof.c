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



// lib_mprof.c
#include <string.h>
#include <stdlib.h>

#include "lib_mprof.h"

#define MPROF_MAX_REGISTERED_FUNCS	( 1024 )

static int	mprof_func_num = 0;
func_profile_t	mprof_funcs[MPROF_MAX_REGISTERED_FUNCS];

void MicroProfile_begin( void )
{
	mprof_func_num = 0;
}

void MicroProfile_end( void )
{
	int		i;

	for ( i = 0; i < mprof_func_num; i++ )
	{
		printf( "%s : %d total calls, %llu total ticks, %.2f ticks per call\n", 
			mprof_funcs[i].name,
			mprof_funcs[i].totalcallnum,
			mprof_funcs[i].totalcountnum,
			(double)mprof_funcs[i].totalcountnum/(double)(mprof_funcs[i].totalcallnum) );
	}
}

func_profile_t * MicroProfile_RegisterFunc( char *name )
{
	if ( mprof_func_num == MPROF_MAX_REGISTERED_FUNCS )
	{
		printf( "MicroProfile_RegisterFunc: reached MPROF_MAX_REGISTERED_FUNCS\n" );
		exit(-1);
	}

	memset( &mprof_funcs[mprof_func_num], 0, sizeof( func_profile_t ) );
	strcpy( mprof_funcs[mprof_func_num].name, name );
	mprof_func_num++;

	return &mprof_funcs[mprof_func_num-1];
}
