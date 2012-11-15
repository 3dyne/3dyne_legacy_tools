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



// vsc2class.c

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
hobj_t		*g_vertexobjs[MAX_NUM_VERTICES];

int		g_num_face;
int		g_faces[MAX_NUM_FACES];

int		g_num_vertexref;
int		g_vertexrefs[MAX_NUM_VERTEXREFS];

int main( int argc, char *argv[] )
{
	int		i;

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

	puts( "===== vsc2class =====" );

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
	printf( " %d vertices, %d faces, %d vertexrefs\n", g_num_vertex, g_num_face, g_num_vertexref );

	if ( out_vertex_name )
	{
		//
		// build vertex class
		//
		root = NewClass( "vertices", "vertices0" );
		for ( i = 0; i < g_num_vertex; i++ )
		{
			hobj_t	*vertex;
			
			vertex = EasyNewClass( "vertex" );
			g_vertexobjs[i] = vertex;
			InsertClass( root, vertex );
			EasyNewVec3d( vertex, "vec", g_vertices[i] );
		}
		h = fopen( out_vertex_name, "w" );
		if ( !h )
			Error( "can't open vertex class\n" );
		WriteClass( root, h );
		fclose( h );
	}

	if ( out_polygon_name )
	{
		int		k;
		//
		// build polygon class
		//
		k = 0;
		root = NewClass( "polygons", "polygons0" );
		for ( i = 0; i < g_num_face; i++ )
		{
			int		j;
			hobj_t		*polygon;

			polygon = EasyNewClass( "polygon" );
			InsertClass( root, polygon );
			
			EasyNewInt( polygon, "pointnum", g_faces[i] );
			for ( j = 0; j < g_faces[i]; j++ )
			{
				char		tt[256];
				sprintf( tt, "%d", j );
				EasyNewVec3d( polygon, tt, g_vertices[g_vertexrefs[k]] );
				k++;
			}
		}
		h = fopen( out_polygon_name, "w" );
		if ( !h )
			Error( "can't open polygon class\n" );
		WriteClass( root, h );
		fclose( h );		
	}

	if ( out_face_name )
	{
		int		k;
		//
		// build face class
		//
		k = 0;
		root = NewClass( "faces", "faces0" );
		for ( i = 0; i < g_num_face; i++ )
		{
			int		j;
			hobj_t		*face;

			face = EasyNewClass( "face" );
			InsertClass( root, face );

			EasyNewInt( face, "pointnum", g_faces[i] );
			for ( j = 0; j < g_faces[i]; j++ )
			{
				char		tt[256];
				sprintf( tt, "%d", j );
				EasyNewClsref( face, tt, g_vertexobjs[g_vertexrefs[k]] );
				k++;
			}
		}
		h = fopen( out_face_name, "w" );
		if ( !h )
			Error( "can't open face class\n" );
		WriteClass( root, h );
		fclose( h );			
	}

	HManagerSaveID();
	exit(0);
}

