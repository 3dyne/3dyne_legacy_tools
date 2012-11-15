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



// bsp2face.c

#include "bsp.h"

int		p_planenum;
plane_t		p_planes[MAX_PLANES];
int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_node_name;
	char		*out_brush_name;

	bspbrush_t	*in_list;
	bspbrush_t		*b;

	printf( "===== bsp2face - build faces from bsp brushes =====\n" );

	SetCmdArgs( argc, argv );

	in_brush_name = GetCmdOpt2( "-i" );
	out_face_name = GetCmdOpt2( "-o" );
	
	if ( !in_brush_name )
		Error( "no input brush file.\n" );
	printf( " input brush file: %s\n", in_brush_name );

	if ( !out_face_name )
		Error( "no output face file.\n" );
	printf( " output face file: %s\n", out_brush_name );

	p_planenum = MAX_PLANES;
	Read_Planes( "planes.asc", p_planes, &p_planenum );
	printf( " %d planes\n", p_planenum );

	in_list = Read_BrushList( in_brush_name );
	printf( " %d input brushes\n", BrushListLength( in_list ) );

	//
	// create polygons for in_list
	// 
	for ( b = in_list; b ; b=b->next )
		CreateBrushPolygons( b );

	// p_nodes[0] is headnode
	p_nodenum = 1;
	BSP_MakeTreeRecursive( 0, in_list, 0 );
	
	printf( " %d output brushes\n", p_brushnum );
	printf( " %d nodes: %d solid, %d empty\n", p_nodenum, stat_solidnum, stat_emptynum );
	printf( " %d leafs with %d brushes\n", stat_empty2num, stat_inempty2num );
	
	printf( " writing files ...\n" );
	Write_BrushArray( p_brushes, p_brushnum, out_brush_name, "bsp" );
	Write_NodeArray( p_nodes, p_nodenum, out_node_name, "bsp" );

}
