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



// lib_unique.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_unique.h"

static unique_t		unique_number = 1;

void InitUniqueNumber( void )
{
	unique_number = 1;
}

unique_t GetUniqueNumber( void )
{
     	if ( unique_number == UNIQUE_CANT_GET )
		return UNIQUE_CANT_GET;

	unique_number++;

	return unique_number-1;
}

void TestUniqueNumber( unique_t test )
{	
	if ( test >= unique_number )
		unique_number = test + 1;    
}

unique_t StringToUnique( char *text )
{
	if ( text[0] != '#' )
		return UNIQUE_INVALIDE;

	return atoi( &text[1] );
}


int CompareUniques( const void *key1, const void *key2 )
{
	unique_t	u1, u2;
	
	u1 = *((unique_t*)(key1));
	u2 = *((unique_t*)(key2));

	if ( u1 > u2 )
		return 1;
	else if ( u1 < u2 )
		return -1;
	else 
		return 0;
			
}
