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



// vsc2c.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_error.h"
#include "lib_token.h"
#include "lib_hobj.h"
#include "cmdpars.h"

#define MAX_NUM_VERTICES	( 1024 * 512 )
#define MAX_NUM_FACES		( 1024 * 256 )
#define MAX_NUM_VERTEXREFS	( 1024 * 1024 )

int		g_num_vertex;
vec3d_t		g_vertices[MAX_NUM_VERTICES];

int		g_num_face;
int		g_faces[MAX_NUM_FACES];

int		g_num_vertexref;
int		g_vertexrefs[MAX_NUM_VERTEXREFS];

int main( int argc, char *argv[] )
{
	int		i, j;

	char		*in_vsc_name;
	char		*out_vertex_name;
	char		*out_polygon_name;
	char		*out_face_name;

	tokenstream_t	*ts;
	FILE		*h;

	hobj_t		*root;

	g_num_vertex = 0;
	g_num_face = 0;
	g_num_vertexref = 0;

	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -ivsc filename: input videoscape file" );
		puts( " -ov filename: output vertex class" );
		puts( " -op filename: output polygon class" );
		puts( " -of filename: output face class" );
		exit(-1);
	}

	SetCmdArgs( argc, argv );

	in_vsc_name = GetCmdOpt2( "-ivsc" );
	out_vertex_name = GetCmdOpt2( "-ov" );
	out_polygon_name = GetCmdOpt2( "-op" );
	out_face_name = GetCmdOpt2( "-of" );

	if ( !in_vsc_name )
		Error( "no input vsc file\n" );
	
	if ( out_vertex_name )
		printf( "switch: build output vertex class\n" );

	if ( out_polygon_name )
		printf( "switch: build output polygon class\n" );
	
	if ( out_face_name )
		printf( "switch: build output face class\n" );

	ts = BeginTokenStream( in_vsc_name );
	if ( !ts )
		Error( "can't open vsc file\n" );

	// check header '3DG1'
	GetToken( ts );
	if ( strcmp( "3DG1", ts->token ) )
		Error( "missing vsc header\n" );

	// get num_vertex
	GetToken( ts );
	g_num_vertex = atoi( ts->token );
	if ( g_num_vertex > MAX_NUM_VERTICES )
		Error( "reached MAX_NUM_VERTICES\n" );

	// read num_vertex vertices
	for ( i = 0; i < g_num_vertex; i++ )
	{
		// get x
		GetToken( ts );
		g_vertices[i][0] = atof( ts->token );
		// get y
		GetToken( ts );
		g_vertices[i][1] = atof( ts->token );
		// get z
		GetToken( ts );
		g_vertices[i][2] = atof( ts->token );
	}
	
	// read face defs till end
	for (;;)
	{
		// get vertexref_num
		if ( GetToken( ts ) == TOKEN_FAIL )
		{
			break;
		}

		if ( g_num_face >= MAX_NUM_FACES )
			Error( "reached MAX_NUM_FACES\n" );

		g_faces[g_num_face] = atoi( ts->token );

		if ( g_faces[g_num_face] != 3 )
			Error( "face is no triangle\n" );

		// read vertexrefs
		for ( i = 0; i < g_faces[g_num_face]; i++ )
		{
			if ( g_num_vertexref >= MAX_NUM_VERTEXREFS )
				Error( "reached MAX_NUM_VERTEXREFS\n" );
			
			GetToken( ts );
			g_vertexrefs[g_num_vertexref] = atoi( ts->token );
			g_num_vertexref++;
		}
		g_num_face++;

		// skip color
		GetToken( ts );
	}
	
	EndTokenStream( ts );

	printf( "float vertices[%d][3]={\n", g_num_vertex );
	for ( i = 0; i < g_num_vertex; i++ )
	{
		printf( "\t{ %f, %f, %f }", g_vertices[i][0], g_vertices[i][1],  g_vertices[i][2] );
		if ( i < g_num_vertex-1 )
		{
			printf( "," );
		}
		printf( "\n" );
	}
	printf( "};\n" );

	printf( "int triangles[%d][3]={\n", g_num_face );
	j = 0;
	for ( i = 0; i < g_num_face; i++ )
	{
		printf( "\t{ %d, %d, %d }", g_vertexrefs[j++], g_vertexrefs[j++], g_vertexrefs[j++] );
		if ( i < g_num_face-1 )
		{
			printf( "," );
		}
		printf( "\n" );		
	}
	printf( "};\n" );
	
	exit(0);
}

