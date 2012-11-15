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



// bspnode.c

#include "type_bspnode.h"



/*
  ====================
  Write_Node

  ====================
*/
void Write_Node( FILE *h, bspnode_t *n )
{
	fprintf( h, "%d %d %d %d %d\n",
		 n->plane,
		 n->child[0],
		 n->child[1],
		 n->firstbrush,
		 n->brushnum );
}



/*
  ====================
  Write_NodeArray

  ====================
*/
void Write_NodeArray( bspnode_t *base, int num, char *name, char *creator )
{
	FILE		*h;
	int		i;

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_NodeArray: can't open file '%s'\n", name );

	fprintf( h, "# bspnode file\n" );
	fprintf( h, "# generated by %s !!! DON'T EDIT !!!\n", creator );
	fprintf( h, "# <bspnode> <node plane/leaf type> <child[0]> <child[1]> <firstbrush> <brushnum>\n" );
	
	for ( i = 0; i < num; i++ )
	{
		fprintf( h, "%d ", i );
		Write_Node( h, &base[i] );
	}

	fprintf( h, "end\n" );

	fclose( h );
}



/*
  ====================
  Read_Node

  read data into already allocated node
  ====================
*/
void Read_Node( tokenstream_t *ts, bspnode_t *n )
{
	// read plane
	GetToken( ts );
	n->plane = atoi( ts->token );

	// read child[0]
	GetToken( ts );
	n->child[0] = atoi( ts->token );

	// read child[1]
	GetToken( ts );
	n->child[1] = atoi( ts->token );

	// read firstbrush
	GetToken( ts );
	n->firstbrush = atoi( ts->token );

	// read brushnum
	GetToken( ts );
	n->brushnum = atoi( ts->token );
} 



/*
  ====================
  Read_NodeArray

  ====================
*/
void Read_NodeArray( bspnode_t *base, int *maxnum, char *name )
{
	tokenstream_t		*ts;
	int			i;

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_NodeArray: can't open file '%s'\n", name );

	for ( i = 0; i < *maxnum; i++ )
	{
		// get node or end
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		if ( i != atoi( ts->token ) )
			Error( "Read_NodeArray: nodes not sorted.\n" );
		
		Read_Node( ts, &base[i] );
	}

	if ( i == *maxnum )
		Error( "Read_NodeArray: reached maxnum (%d)\n", *maxnum );

	EndTokenStream( ts );
	*maxnum = i;
}
