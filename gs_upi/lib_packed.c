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



// u_packed.c

#include <stdio.h>
#include <string.h>
#include "lib_endian.h"
#include "lib_packed.h"
#include "lib_error.h"

static char	*cur_start;
static char	*cur_pos;
static int	cur_maxsize;
static int	cur_size;
static int	cur_mode;

/*
  ====================
  BeginPack

  ====================
*/
void BeginPack( int mode, void *ptr, int maxsize )
{
	cur_start = ( char * ) ptr;
	cur_pos = cur_start;
	cur_maxsize = maxsize;
	cur_size = 0;
	cur_mode = mode;
}

/*
  ====================
  EndPack

  ====================
*/
int EndPack( void )
{
	return cur_size;
}

/*
  ====================
  Packs8

  ====================
*/
void Packs8( char s8 )
{
	if ( cur_size + 1 >= cur_maxsize )
	{

	}
	*cur_pos = s8;
	cur_pos++;
	cur_size++;
}

/*
  ====================
  GetPackPtr

  ====================
*/
void * GetPackPtr( void )
{
	return cur_pos;
}

/*
  ====================
  GetPackSize

  ====================
*/
int GetPackSize( void )
{
	return cur_size;
}

/*
  ====================
  Packs16

  ====================
*/
void Packs16( short s16 )
{
	char	tmp[2];

	if ( cur_size + 2 >= cur_maxsize )
	{

	}

	if ( cur_mode != PACKED_MACHINE_BIN )
		s16 = SHORT( s16 );

	memcpy( tmp, &s16, 2 );

	*cur_pos = tmp[0];
	cur_pos++;
	*cur_pos = tmp[1];
	cur_pos++;
	cur_size += 2;
}

/*
  ====================
  Packs32

  ====================
*/
void Packs32( int s32 )
{
	char	tmp[4];

	if ( cur_size + 4 >= cur_maxsize )
	{

	}

	if ( cur_mode != PACKED_MACHINE_BIN )
		s32 = INT( s32 );

	memcpy( tmp, &s32, 4 );

	*cur_pos = tmp[0];
	cur_pos++;
	*cur_pos = tmp[1];
	cur_pos++;
	*cur_pos = tmp[2];
	cur_pos++;
	*cur_pos = tmp[3];
	cur_pos++;

	cur_size += 4;
}


/*
  ====================
  Packfp32

  ====================
*/
void Packfp32( float fp32 )
{
	char	tmp[4];

	if ( cur_size + 4 >= cur_maxsize )
	{

	}

	memcpy( tmp, &fp32, 4 );

	*cur_pos = tmp[0];
	cur_pos++;
	*cur_pos = tmp[1];
	cur_pos++;
	*cur_pos = tmp[2];
	cur_pos++;
	*cur_pos = tmp[3];
	cur_pos++;

	cur_size += 4;	
}


/*
  ====================
  Packntstring

  ====================
*/
void PackntString( char *ntstring )
{
	for ( ;; )
	{
		if ( cur_size == cur_maxsize )
		{
			
		}
		*cur_pos = *ntstring;
		cur_pos++;
		ntstring++;
		cur_size++;

		if ( *(cur_pos-1) == 0 )
			break;
		
	}
}

/*
  ====================
  PackString

  ====================
*/
void PackString( char *string, int size )
{
	if( cur_size + size >=  cur_maxsize )
	{
	}

	memcpy( cur_pos, string, size );
}

/*
  ====================
  Pack

  ====================
*/

/*
  ====================
  BeginUnPack

  ====================
*/
void BeginUnpack( int mode, void *ptr, int size )
{
	cur_start = ( char * ) ptr;
	cur_pos = cur_start;

	cur_maxsize = size;
	cur_size = 0;
	cur_mode = mode;
}


int EndUnpack()
{
	return cur_size;
}


/*
  ====================
  UnPacks8

  ====================
*/

void Unpacks8( char *s8 )
{

	*s8 = *cur_pos;


	cur_pos++;
	cur_size++;
}

void Unpacks16( short *s16 )
{
	memcpy( s16, cur_pos, 2 );

	if ( cur_mode != PACKED_MACHINE_BIN )
		*s16 = SHORT( *s16 );
	cur_pos+=2;
	cur_size+=2;

}

void Unpacks32( int *s32 )
{
	memcpy( s32, cur_pos, 4 );

	if ( cur_mode != PACKED_MACHINE_BIN )
		*s32 = INT( *s32 );
	cur_pos+=4;
	cur_size+=4;

}


void Unpackfp32( float *fp32 )
{
	memcpy( fp32, cur_pos, 4 );

	cur_pos+=4;
	cur_size+=4;
}


void UnpackString( char *dest, int size )
{
	memcpy( dest, cur_pos, size );
	
	cur_pos += size;
	cur_size += size;
}

void UnpackntString( char *dest, int maxsize )
{
	for ( ;; )
	{
		if ( cur_size == cur_maxsize )
		{
			
		}
		*dest = *cur_pos;
		cur_pos++;
		dest++;
		cur_size++;

		if ( *(cur_pos-1) == 0 )
			break;
		
	}
}

void UnpackSkip( int space )
{
	if( cur_size + space >= cur_maxsize )
	{
		Error( "running out of input buffer\n" );
	}

	cur_pos += space;
	cur_size += space;
}
