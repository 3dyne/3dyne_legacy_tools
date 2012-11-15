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



// lib_trimesh.h

#ifndef __lib_trimesh
#define __lib_trimesh

#include "lib_unique.h"
#include "lib_math.h"
#include "lib_container.h"
#include "lib_hobj.h"

typedef struct trimesh_vertex_s
{
	unique_t		id;
	int			index;		// set by TriMesh_EnumVertices
	vec3d_t			vec;
	
	// calc by TriMesh_CalcAllKnotNormals
	vec3d_t			norm;

	// calc by TriMesh_CalcAllKnotValues
	fp_t			value;

} trimesh_vertex_t;

typedef struct trimesh_knot_s
{
	unique_t		id;		// id of releated vertex
	u_list_t		tri_list;

	trimesh_vertex_t	*vertex_ref;
} trimesh_knot_t;

typedef struct trimesh_tri_s
{
	unique_t		id;
	int			index;		// set by TriMesh_EnumTris
	int			flood;		// 0: not flooded, >=1: flooded
	trimesh_knot_t		*knot_refs[3];	// 0: v0, 1: v1, 2: v2
	struct trimesh_tri_s	*other_refs[3];	// 0: v0-v1, 1: v1-v2, 2: v2-v0

	// calc by TriMesh_CalcAllTriNormals
	vec3d_t			norm;
} trimesh_tri_t;

typedef struct trimesh_s
{
	u_map_t		knot_map;
	u_map_t		tri_map;
	u_list_t	vertex_list;
} trimesh_t;

// trimesh main

void TriMesh_Init( trimesh_t *mesh );
unique_t TriMesh_AddFreeVertex( trimesh_t *mesh, vec3d_t v, fp_t merge_dist );
trimesh_tri_t * TriMesh_AddTri( trimesh_t *mesh, unique_t tri_id, unique_t knot_ids[3] );
void TriMesh_RemoveTri( trimesh_t *mesh, trimesh_tri_t *tri );

void TriMesh_DumpTri( trimesh_tri_t *tri );
void TriMesh_Dump( trimesh_t *mesh );

hobj_t * TriMesh_BuildClassFromVertices( trimesh_t *mesh );
hobj_t * TriMesh_BuildClassFromTris( trimesh_t *mesh );

void TriMesh_EnumTris( trimesh_t *mesh, int *count );
void TriMesh_EnumVertices( trimesh_t *mesh, int *count );

void TriMesh_ClearFlood( trimesh_t *mesh, int flood );	// flood = 0: clear all, flood >=1: clear special flood
trimesh_tri_t * TriMesh_GetUnFloodedTri( trimesh_t *mesh );
void TriMesh_FloodTriStrip( trimesh_tri_t *start, int start_edge, int flood, u_list_t *knot_list );

hobj_t * TriMesh_BuildGLMesh( trimesh_t *mesh, unsigned char *base, int *ofs );

// simplify mesh

void TriMesh_SetupAllKnotVertexRefs( trimesh_t *mesh );
void TriMesh_CalcAllTriNormals( trimesh_t *mesh );
void TriMesh_CalcAllKnotNormals( trimesh_t *mesh );
void TriMesh_CalcAllKnotValues( trimesh_t *mesh );

bool_t TriMesh_VisitKnotTrisInOrder( trimesh_knot_t *knot, void (*tri_func)(trimesh_tri_t *tri, trimesh_knot_t *knot ) );

void TriMesh_RemoveKnotAndFix( trimesh_t *mesh, trimesh_knot_t *knot );

trimesh_t * ReadTriMeshFiles( char *vertex_name, char *tri_name );

void TriMesh_GLDBG( trimesh_t *mesh );

#endif
