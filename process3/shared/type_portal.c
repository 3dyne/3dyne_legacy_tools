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



// type_portal.c

#include "type_portal.h"

int_array_t* NewIntArray( int intnum )
{
    	int		size;
	int_array_t	*ia;

	size = (int)((int_array_t *)0)->ints[intnum];
	ia = (int_array_t *) malloc( size );
	memset( ia, 0, size );

	return ia;
}

void FreeIntArray( int_array_t *ia )
{
	free( ia );
}



/*
  ====================
  Read_IntArray

  ====================
*/
int_array_t* Read_IntArray( tokenstream_t *ts )
{
	int_array_t	*ia;
	int		i;

	// get intnum
	GetToken( ts );
	i = atoi( ts->token );
	ia = NewIntArray( i );
	ia->intnum = i;

	for ( i = 0; i < ia->intnum; i++ )
	{
		GetToken( ts );
		ia->ints[i] = atoi( ts->token );
	}

	return ia;
}



/*
  ====================
  Read_IAArray

  ====================
*/
void Read_IAArray( int_array_t **base, int *maxnum, char *name )
{
	tokenstream_t		*ts;
	int			i;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_IAArray: can't open file '%s'\n", name );

	for ( i = 0; i < *maxnum; i++ )
	{
		// get int_array or end
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		if ( i != atoi( ts->token ) )
			Error( "Read_IAArray: int_arrays not sorted.\n" );

		base[i] = Read_IntArray( ts );
	}
	
	if ( i == *maxnum )
		Error( "Read_NodeArray: reached maxnum (%d)\n", *maxnum );

	EndTokenStream( ts );

	*maxnum = i;
}



/*
  ====================
  Read_Portal

  read data into an already alloced portal
  ====================
*/
void Read_Portal( tokenstream_t *ts, portal_t *p )
{
	// get plane norm
	Read_Vec3d( ts, p->norm );

	// get plane dist
//	Read_fp( ts, &p->dist );
	GetToken( ts );
	p->dist = atof( ts->token );
	
	// get nodes
	GetToken( ts );
	p->nodes[0] = atoi( ts->token );
	GetToken( ts );
	p->nodes[1] = atoi( ts->token );

	// get polygon
	p->p = Read_Polygon( ts );
	
}



/*
  ====================
  Read_PortalArray

  ====================
*/
void Read_PortalArray( portal_t *base, int *maxnum, char *name )
{
	tokenstream_t		*ts;
	int		i;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_PortalArray: can't open file '%s'\n", name );

	for ( i = 0; i < *maxnum; i++ )
	{
		// get portal or end
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		if ( i != atoi( ts->token ) )
			Error( "Read_PortalArray: portals not sorted.\n" );

		Read_Portal( ts, &base[i] );
	}

	if ( i == *maxnum )
		Error( "Read_PortalArray: reached maxnum (%d)\n", *maxnum );

	EndTokenStream( ts );
	*maxnum = i;
}
