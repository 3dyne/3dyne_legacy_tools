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



// face_link.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_mem.h"
#include "lib_hobj.h"
#include "lib_container.h"
#include "lib_unique.h"
#include "cmdpars.h"

typedef struct vertex_s
{
	unique_t	self_id;
	
	u_list_t	face_list;	// list of faces shareing the vertex
} vertex_t;


typedef struct face_s
{
	int		pointnum;
	int		p[8];		// variable
} face_t;

void * GetVertexPrimaryKey( const void *obj )
{
	return &(((vertex_t*)(obj))->self_id);
}

int main( int argc, char *argv[] )
{
	char	*in_face_name;
	hobj_search_iterator_t	hobj_iter;
	hobj_t		*face_root;
	hobj_t		*face;
	u_map_t		vertex_map;
	u_list_t	face_list;
	int		i;

	int		num_face;
	int		num_vertex;
	int		num_faceref;

	puts( "===== face_link =====" );

	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -if filename: input face class" );
		exit(-1);
	}

	SetCmdArgs( argc, argv );

	in_face_name = GetCmdOpt2( "-if" );
	
	if ( !in_face_name )
		Error( "no input face class\n" );

	face_root = ReadClassFile( in_face_name );
	if ( !face_root )
		Error( "can't read of face class\n" );

	U_InitMap( &vertex_map, map_default, CompareUniques, GetVertexPrimaryKey );
	U_InitList( &face_list );

	num_face = 0;
	num_vertex = 0;
	num_faceref = 0;

	InitClassSearchIterator( &hobj_iter, face_root, "face" );
	for ( ; ( face = SearchGetNextClass( &hobj_iter ) ) ; )
	{
		face_t		*f;
		size_t		size;
		int		pointnum;

		EasyFindInt( &pointnum, face, "pointnum" );
		size = (size_t)&(((face_t *)0)->p[pointnum]);
		f = (face_t *) NEWBYTES( size );
		f->pointnum = pointnum;

		for ( i = 0; i < pointnum; i++ )
		{
			char	tt[256];
			vertex_t		*vert;

			sprintf( tt, "%d", i );
			EasyFindInt( &f->p[i], face, tt );
			
			vert = U_MapSearch( &vertex_map, &f->p[i] );
			if ( !vert )
			{
				vert = NEWTYPE( vertex_t );
				vert->self_id = f->p[i];
				U_InitList( &vert->face_list );
				U_MapInsert( &vertex_map, vert );
				num_vertex++;
			}
			U_ListInsertAtHead( &vert->face_list, f );
			num_faceref++;
		}		
		U_ListInsertAtHead( &face_list, f );
		num_face++;
	}
	
	printf( " %d faces, %d vertices with %d facerefs\n", num_face, num_vertex, num_faceref );

	exit(0);
}
