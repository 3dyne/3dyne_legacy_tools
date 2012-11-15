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



// lib_trimesh.c

#include "s_mem.h"
#include "lib_trimesh.h"
#include "lib_gldbg.h"

static gld_session_t	*g_gldbg;

static void * KnotGetPrimaryKey( const void *obj )
{
	return (void*)&(((trimesh_knot_t*)(obj))->id);
}

static void * TriGetPrimaryKey( const void *obj )
{
	return (void*)&(((trimesh_tri_t*)(obj))->id);
}

/*
  ==============================
  TriMesh_Init

  ==============================
*/
void TriMesh_Init( trimesh_t *mesh )
{
	INITTYPE( mesh, trimesh_t );
	
	U_InitMap( &mesh->knot_map, map_default, CompareUniques, KnotGetPrimaryKey );
	U_InitMap( &mesh->tri_map, map_default, CompareUniques, TriGetPrimaryKey );
	U_InitList( &mesh->vertex_list );
}

/*
  ==============================
  TriMesh_Dump

  ==============================
*/
static int dump_num_tri;
static int dump_num_edge;
static int dump_num_link;
static int dump_num_knot;
static int dump_num_triref;
static int dump_num_vertexref;

static void TriDumpFunc( void *obj )
{
	trimesh_tri_t	*tri = obj;
	int			i;
	
	dump_num_tri++;
	
	for ( i = 0; i < 3; i++ )
	{
		if ( tri->other_refs[i] )
			dump_num_link++;
		dump_num_edge++;
	}
}

static void KnotDumpFunc( void *obj )
{
	trimesh_knot_t	*knot = obj;

	dump_num_knot++;
	dump_num_triref += U_ListLength( &knot->tri_list );

	if ( knot->vertex_ref )
		dump_num_vertexref++;
}

void TriMesh_Dump( trimesh_t *mesh )
{
	printf( "TriMesh_Dump:\n" );

	dump_num_tri = 0;
	dump_num_edge = 0;
	dump_num_link = 0;

	dump_num_knot = 0;
	dump_num_triref = 0;
	dump_num_vertexref = 0;

	U_MapForEach( &mesh->tri_map, TriDumpFunc );
	U_MapForEach( &mesh->knot_map, KnotDumpFunc );

	printf( " %d tris with %d edges and %d links\n", dump_num_tri, dump_num_edge, dump_num_link );
	printf( " %d knots with %d trirefs and %d vertexrefs\n", dump_num_knot, dump_num_triref, dump_num_vertexref );

}

void TriMesh_DumpTri( trimesh_tri_t *tri )
{
	printf( "TriMesh_DumpTri: id #%u\n", tri->id );       
	printf( " k0: #%u\n", tri->knot_refs[0]->id );
	printf( " k1: #%u\n", tri->knot_refs[1]->id );
	printf( " k2: #%u\n", tri->knot_refs[2]->id );
	printf( " other0: #%d\n", tri->other_refs[0]==NULL?-1:tri->other_refs[0]->id );
	printf( " other1: #%d\n", tri->other_refs[1]==NULL?-1:tri->other_refs[1]->id );
	printf( " other2: #%d\n", tri->other_refs[2]==NULL?-1:tri->other_refs[2]->id );
}

/*
  ==============================
  TriMesh_AddFreeVertex

  ==============================
*/
unique_t TriMesh_AddFreeVertex( trimesh_t *mesh, vec3d_t v, fp_t merge_dist )
{
	u_list_iter_t		iter;
	fp_t			len, best_len;
	trimesh_vertex_t	*vertex, *best_vertex;

	vec3d_t		delta;

	best_len = 9999999.9;
	best_vertex = NULL;

	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( vertex = U_ListIterNext( &iter ) ) ; )
	{
		Vec3dSub( delta, v, vertex->vec );
		len = Vec3dLen( delta );

		if ( len <= merge_dist )
		{
			if ( len < best_len )
			{
				best_len = len;
				best_vertex = vertex;
			}
		}
	}

	if ( !best_vertex )
	{
		// create a new vertex
		
		best_vertex = NEWTYPE( trimesh_vertex_t );
		best_vertex->id = HManagerGetFreeID();
		Vec3dCopy( best_vertex->vec, v );
		
		U_ListInsertAtHead( &mesh->vertex_list, best_vertex );
	}
	
	return best_vertex->id;
}

/*
  ==============================
  TriMesh_CheckLink

  ==============================
*/
int TriMesh_IsKnotRefInTri( trimesh_tri_t *tri, trimesh_knot_t *knot )
{
	if ( tri->knot_refs[0] == knot )
		return 0;
	if ( tri->knot_refs[1] == knot )
		return 1;
	if ( tri->knot_refs[2] == knot )
		return 2;

	return -1;
}

int TriMesh_IsKnotEdgeInTri( trimesh_tri_t *tri, trimesh_knot_t *knot1, trimesh_knot_t *knot2 )
{
	if ( tri->knot_refs[0] == knot1 &&
	     tri->knot_refs[1] == knot2 )
		return 0;
	if ( tri->knot_refs[1] == knot1 &&
	     tri->knot_refs[2] == knot2 )
		return 1;
	if ( tri->knot_refs[2] == knot1 &&
	     tri->knot_refs[0] == knot2 )
		return 2;
	return -1;
}

void TriMesh_LinkTris( trimesh_tri_t *tri1, trimesh_tri_t *tri2, trimesh_knot_t *knot1, trimesh_knot_t *knot2 )
{
	int		k1, k2;
	int		p, n;
	// link tri1 edge -> tri2
	
	// find edge in tri1
	k1 = TriMesh_IsKnotRefInTri( tri1, knot1 );
	k2 = TriMesh_IsKnotRefInTri( tri1, knot2 );

	p = (k1-1)<0 ? 2 : k1-1;
	n = (k1+1)>2 ? 0 : k1+1;

	if ( k2 == p )
	{
		// k2 is first, k1 is second
		tri1->other_refs[k2] = tri2;
	}
	else if ( k2 == n )
	{
		// k1 is first, k2 is second
		tri1->other_refs[k1] = tri2;
	}
	else
	{
		Error( "can't get edge\n" );
	}


	// link tri2 edge -> tri1

	// find edge in tri2
	k1 = TriMesh_IsKnotRefInTri( tri2, knot1 );
	k2 = TriMesh_IsKnotRefInTri( tri2, knot2 );

	p = (k1-1)<0 ? 2 : k1-1;
	n = (k1+1)>2 ? 0 : k1+1;

	if ( k2 == p )
	{
		// k2 is first, k1 is second
		tri2->other_refs[k2] = tri1;
	}
	else if ( k2 == n )
	{
		// k1 is first, k2 is second
		tri2->other_refs[k1] = tri1;
	}
	else
	{
		Error( "can't get edge\n" );
	}	
}


bool_t TriMesh_DoEdgeExist( trimesh_knot_t *knot1, trimesh_knot_t *knot2 )
{
	trimesh_tri_t	*t;
	u_list_iter_t	iter;
	
	U_ListIterInit( &iter, &knot1->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		if ( TriMesh_IsKnotEdgeInTri( t, knot1, knot2 ) != -1 )
			return true;
	}
	return false;
}

void TriMesh_LinkToOtherTri( trimesh_tri_t *tri, trimesh_knot_t *knot1, trimesh_knot_t *knot2 )
{
	trimesh_tri_t	*t;
	u_list_iter_t	iter;

	int		edge, other_edge;

	// serach knot1's tri_list for tris that have a knot1=>knot2 edge
	// if found it's illegal to add an other tri with this edge

	edge = TriMesh_IsKnotEdgeInTri( tri, knot1, knot2 );
	if ( edge == -1 )
		Error( "can't get edge\n" );

	U_ListIterInit( &iter, &knot1->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		if ( t == tri )
			continue;

		if ( TriMesh_IsKnotEdgeInTri( t, knot1, knot2 ) != -1 )
		{
			printf( "WARNING: got already a tri #%u with edge %d-%d\n", t->id, knot1->id, knot2->id );
			return;
		}

	}

	// search knot2's tri_list for tris that have a knot2=>knot1 edge
	// if found setup link

	U_ListIterInit( &iter, &knot2->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		if ( t == tri )
			continue;

		other_edge = TriMesh_IsKnotEdgeInTri( t, knot2, knot1 );
		
		if ( other_edge != -1 )
		{
			t->other_refs[other_edge] = tri;
			tri->other_refs[edge] = t;
		}
	}
}

/*
  ==============================
  TriMesh_AddTri

  tri def: tri_id + 3*knot_ids
  ==============================
*/
trimesh_tri_t * TriMesh_AddTri( trimesh_t *mesh, unique_t tri_id, unique_t knot_ids[3] )
{
	int			i;
	trimesh_knot_t		*knot;
	trimesh_tri_t		*tri;

	if ( tri_id == UNIQUE_INVALIDE )
	{
		// generated a unique id for this tri
		tri_id = HManagerGetFreeID();
	}

	if ( U_MapSearch( &mesh->tri_map, &tri_id ) )
		Error( "already a tri with id '#%u' in mesh\n", tri_id );

	tri = NEWTYPE( trimesh_tri_t );
	tri->id = tri_id;
	U_MapInsert( &mesh->tri_map, tri );
	
	for ( i = 0; i < 3; i++ )
	{
		knot = U_MapSearch( &mesh->knot_map, &knot_ids[i] );
		if ( !knot )
		{
			knot = NEWTYPE( trimesh_knot_t );
			knot->id = knot_ids[i];
			U_InitList( &knot->tri_list );
			U_MapInsert( &mesh->knot_map, knot );

//			printf( "new knot: %d\n", knot->id );
		}
		else
		{
//			printf( "found knot: %d\n", knot->id );
		}

		// knot knows tri
		U_ListInsertAtHead( &knot->tri_list, tri );

		// tri knows knot
		tri->knot_refs[i] = knot;
	}

	TriMesh_LinkToOtherTri( tri, tri->knot_refs[0], tri->knot_refs[1] );
	TriMesh_LinkToOtherTri( tri, tri->knot_refs[1], tri->knot_refs[2] );
	TriMesh_LinkToOtherTri( tri, tri->knot_refs[2], tri->knot_refs[0] );

	return tri;
}

/*
  ==============================
  TriMesh_RemoveTri
  
  ==============================
*/
void TriMesh_RemoveTri( trimesh_t *mesh, trimesh_tri_t *tri )
{
	int		i, j;
	u_list_iter_t	iter;
	trimesh_tri_t	*t;

	// remove from linked tris
	for ( i = 0; i < 3; i++ )
	{
		if ( tri->other_refs[i] != NULL )
		{
			for ( j = 0; j < 3; j++ )
			{
				if ( tri->other_refs[i]->other_refs[j] == tri )
				{
					tri->other_refs[i]->other_refs[j] = NULL;
					break;
				}
			}
			if ( j == 3 )
			{
				Error( "can't find tri in other\n" );
			}
		}
	}

	// remove from vertex tri_list

	for ( i = 0; i < 3; i++ )
	{
		U_ListIterInit( &iter, &tri->knot_refs[i]->tri_list );
		for ( ; ( t = U_ListIterNext( &iter ) ) ; )
		{
			if ( t == tri )
			{
				U_ListIterRemoveGoPrev( &iter );
				break;
			}
		}
		if ( !t )
		{
			Error( "can't find tri in tri_list\n" );
		}

#if 1
		if ( U_ListLength( &tri->knot_refs[i]->tri_list ) == 0 )
		{
			// knot is no more used
			U_MapRemove( &mesh->knot_map, &tri->knot_refs[i]->id );
			U_CleanUpList( &tri->knot_refs[i]->tri_list, NULL );
			FREE( tri->knot_refs[i] );
		}
#endif
	}

	// remove from map
	U_MapRemove( &mesh->tri_map, &tri->id );
	FREE( tri );		
}


// =============================================================================

/*
  ==============================
  TriMesh_BuildClassFromVertices

  ==============================
*/
hobj_t * TriMesh_BuildClassFromVertices( trimesh_t *mesh )
{
	hobj_t			*vertex_root;
	u_list_iter_t		iter;
	trimesh_vertex_t		*v;
	char			tt[256];

	vertex_root = EasyNewClass( "vertices" );	

	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( v = U_ListIterNext( &iter ) ) ; )
	{
		hobj_t		*vertex;

		sprintf( tt, "#%u", v->id );
		vertex = NewClass( "vertex", tt );
		EasyNewVec3d( vertex, "vec", v->vec );
		InsertClass( vertex_root, vertex );
	}

	return vertex_root;
}

/*
  ==============================
  TriMesh_BuildClassFromTris

  ==============================
*/
static 	hobj_t			*g_tri_root;

static void BuildClassFromTri_CB( void *obj )
{
	trimesh_tri_t	*t = obj;
	hobj_t		*tri;
	hpair_t		*pair;
	char		tt[256];

	sprintf( tt, "#%u", t->id );
	tri = NewClass( "tri", tt );
	InsertClass( g_tri_root, tri );

	sprintf( tt, "#%u", t->knot_refs[0]->id );
	pair = NewHPair2( "ref", "0", tt );
	InsertHPair( tri, pair );
	sprintf( tt, "#%u", t->knot_refs[1]->id );
	pair = NewHPair2( "ref", "1", tt );
	InsertHPair( tri, pair );
	sprintf( tt, "#%u", t->knot_refs[2]->id );
	pair = NewHPair2( "ref", "2", tt );
	InsertHPair( tri, pair );
}

hobj_t * TriMesh_BuildClassFromTris( trimesh_t *mesh )
{
	g_tri_root = EasyNewClass( "tris" );

	U_MapForEach( &mesh->tri_map, BuildClassFromTri_CB );

	return g_tri_root;
}

/*
  ==============================
  TriMesh_EnumTris

  ==============================
*/
void TriMesh_EnumTris( trimesh_t *mesh, int *count )
{
	Error( "TriMesh_EnumTris: not impl\n" );
}

/*
  ==============================
  TriMesh_EnumVertices

  ==============================
*/
void TriMesh_EnumVertices( trimesh_t *mesh, int *count )
{
	u_list_iter_t	iter;
	trimesh_vertex_t	*v;

	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( v = U_ListIterNext( &iter ) ) ; )
	{
		v->index = (*count)++;
	}
}

/*
  ==============================
  TriMesh_ClearFlood

  ==============================
*/
void TriMesh_ClearFlood( trimesh_t *mesh, int flood )
{
	u_list_t	list;
	u_list_iter_t	iter;
	trimesh_tri_t	*t;

	U_InitList( &list );
	U_MapGetObjList( &mesh->tri_map, &list );

	U_ListIterInit( &iter, &list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		if ( flood == 0 )
		{
			t->flood = 0;
		}
		else if ( t->flood == flood )
		{
			t->flood = 0;
		}
	}
	U_CleanUpList( &list, NULL );
}

/*
  ==============================
  TriMesh_GetUnFloodedTri

  ==============================
*/
trimesh_tri_t * TriMesh_GetUnFloodedTri( trimesh_t *mesh )
{
	u_list_t	list;
	u_list_iter_t	iter;
	trimesh_tri_t	*t;

	U_InitList( &list );
	U_MapGetObjList( &mesh->tri_map, &list );

	U_ListIterInit( &iter, &list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		if ( t->flood == 0 )
		{
			U_CleanUpList( &list, NULL );
			return t;
		}
	}
	
	return NULL;
}

/*
  ==============================
  TriMesh_FloodTriStrip

  ==============================
*/
static void FloodTriStripRecursive( trimesh_tri_t *tri, trimesh_knot_t *knot, int dir, int flood, u_list_t *knot_list )
{
	int		k, n;	// knot, next knot
	int		e;	// edge

	if ( !tri )
	{
		return;
	}
	if ( tri->flood != 0 )
	{
		return;
	}

	tri->flood = flood;

	e = n = 0;

	// index of current knot in this tri
	k = TriMesh_IsKnotRefInTri( tri, knot );

	if ( dir == -1 )
	{
		// next knot for tri strip is prev knot of current knot in current tri
		n = (k-1)<0?2:k-1;

		// leave through edge
		e = n;
	}
	else if ( dir == 1 )
	{
		// next knot for tri strip is next knot of current knot in current tri
		n = (k+1)==3?0:k+1;
		
		// leave through edge
		e = k;
	}
	else
		Error( "invalid direction\n" );

	U_ListInsertAtTail( knot_list, tri->knot_refs[n] );

	FloodTriStripRecursive( tri->other_refs[e], tri->knot_refs[n], -dir, flood, knot_list );
}

void TriMesh_FloodTriStrip( trimesh_tri_t *start_tri, int start_edge, int flood, u_list_t *knot_list )
{
	int		k;
	trimesh_knot_t	*knot;

	if ( start_tri->flood != 0 )
		Error( "TriMesh_FloodTriStrip: start_tri is already flooded\n" );

	k = start_edge;
	knot = start_tri->knot_refs[k];
	U_ListInsertAtTail( knot_list, knot );

	k = (k+1)==3?0:(k+1);
	knot = start_tri->knot_refs[k];
	U_ListInsertAtTail( knot_list, knot );
	
	FloodTriStripRecursive( start_tri, knot, 1, flood, knot_list );
}

/*
  ==============================
  TriMesh_BuildGLMesh

  ==============================
*/

hobj_t * TriMesh_BuildGLMesh( trimesh_t *mesh, unsigned char *base, int *ofs )
{
	int		i;

	int		m_num_vertex;
	vec3d_t		*m_vertices;
	int		m_num_vref;
	int		*m_vrefs;
	int		m_num_cmd;
	int		*m_cmds;
	
	int		count;

	trimesh_vertex_t	*v;

	u_list_iter_t		iter;
	u_list_iter_t		knot_iter;


	hobj_t		*glmesh;

	u_list_t	strip_list;
	u_list_t	*knot_list;
	trimesh_knot_t	*knot;
//	int		

	glmesh = EasyNewClass( "glmesh" );

	count = 0;
	TriMesh_EnumVertices( mesh, &count );
	TriMesh_SetupAllKnotVertexRefs( mesh );

	U_InitList( &strip_list );

	TriMesh_ClearFlood( mesh, 0 );
	{
		trimesh_tri_t	*start_tri;
		int		flood;

		// insert all knot_lists into the strip_list;
		m_num_vref = 0;
		m_num_cmd = 0;
		
		for ( flood = 1; (start_tri = TriMesh_GetUnFloodedTri( mesh ) ) ; flood++ )
		{

			knot_list = NEWTYPE( u_list_t );
			U_InitList( knot_list );
			TriMesh_FloodTriStrip( start_tri, 0, flood, knot_list );
			
			m_num_vref += U_ListLength( knot_list );
			m_num_cmd += 2;
			
			printf( " %d vrefs in strip\n", U_ListLength( knot_list ) );
			
			U_ListInsertAtTail( &strip_list, knot_list );
		}
	}

	// 
	// write vertex array
	//
     
	m_vertices = (vec3d_t *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_vertex", *ofs );

	m_num_vertex = U_ListLength( &mesh->vertex_list );
	(*ofs) += m_num_vertex * sizeof( vec3d_t );
	EasyNewInt( glmesh, "num_vertex", m_num_vertex );

	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( v = U_ListIterNext( &iter ) ) ; )
	{
		Vec3dCopy( m_vertices[v->index], v->vec );
	}

	//
	// write vref array
	//
	m_vrefs = (int *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_vref", *ofs );

	(*ofs) += m_num_vref * sizeof( int );
	EasyNewInt( glmesh, "num_vref", m_num_vref );
	
	U_ListIterInit( &iter, &strip_list );
	for ( i = 0; ( knot_list = U_ListIterNext( &iter ) ) ; )
	{
		U_ListIterInit( &knot_iter, knot_list );
		for ( ; ( knot = U_ListIterNext( &knot_iter ) ) ; )
		{
			m_vrefs[i++] = knot->vertex_ref->index;
		}
	}
	
	
	//
	// write cmd array
	//
	m_cmds = (int *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_cmd", *ofs );

	m_num_cmd++;	// for BE_CMD_END
	(*ofs) += m_num_cmd * sizeof( int );
	EasyNewInt( glmesh, "num_cmd", m_num_cmd );

	U_ListIterInit( &iter, &strip_list );
	for ( i = 0; ( knot_list = U_ListIterNext( &iter ) ) ; )
	{		
		if ( U_ListLength( knot_list ) == 3 )
		{
			m_cmds[i++] = 1;		// BE_CMD_TRIANGLES
			m_cmds[i++] = 3;
		}
		else
		{
			m_cmds[i++] = 3;		// BE_CMD_TRIANGLE_STRIP
			m_cmds[i++] = U_ListLength( knot_list );	// vref num
		}
	}
	m_cmds[i++] = 0;		// BE_CMD_END
	
	//
	// clean up
	//
	U_ListIterInit( &iter, &strip_list );
	for ( ; ( knot_list = U_ListIterNext( &iter ) ) ; )
	{
		U_CleanUpList( knot_list, NULL );
		FREE( knot_list );
	}
	U_CleanUpList( &strip_list, NULL );


	return glmesh;
}


hobj_t * TriMesh_BuildGLMesh_old( trimesh_t *mesh, unsigned char *base, int *ofs )
{
	int		i;

	int		m_num_vertex;
	vec3d_t		*m_vertices;
	int		m_num_vref;
	int		*m_vrefs;
	int		m_num_cmd;
	int		*m_cmds;
	
	int		count;

	trimesh_vertex_t	*v;
	trimesh_tri_t		*t;

	u_list_t		tri_list;
	int			num_tri;

	u_list_iter_t		iter;

	hobj_t		*glmesh;

	glmesh = EasyNewClass( "glmesh" );

	count = 0;
	TriMesh_EnumVertices( mesh, &count );
	TriMesh_SetupAllKnotVertexRefs( mesh );

	// 
	// write vertex array
	//
	m_vertices = (vec3d_t *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_vertex", *ofs );

	m_num_vertex = U_ListLength( &mesh->vertex_list );
	(*ofs) += m_num_vertex * sizeof( vec3d_t );
	EasyNewInt( glmesh, "num_vertex", m_num_vertex );

	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( v = U_ListIterNext( &iter ) ) ; )
	{
		Vec3dCopy( m_vertices[v->index], v->vec );
	}

	//
	// write vref array
	//
	m_vrefs = (int *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_vref", *ofs );

	U_InitList( &tri_list );
	U_MapGetObjList( &mesh->tri_map, &tri_list );
	num_tri = U_ListLength( &tri_list );

	m_num_vref = num_tri * 3;
	(*ofs) += m_num_vref * sizeof( int );
	EasyNewInt( glmesh, "num_vref", m_num_vref );

	U_ListIterInit( &iter, &tri_list );
	for ( i = 0; ( t = U_ListIterNext( &iter ) ) ; )
	{
		m_vrefs[i++] = t->knot_refs[0]->vertex_ref->index;
		m_vrefs[i++] = t->knot_refs[1]->vertex_ref->index;
		m_vrefs[i++] = t->knot_refs[2]->vertex_ref->index;
	}
	U_CleanUpList( &tri_list, NULL );

	//
	// write cmd array
	//
	m_cmds = (int *) &base[*ofs];
	EasyNewInt( glmesh, "ofs_cmd", *ofs );

	m_num_cmd = 3;
	(*ofs) += m_num_cmd * sizeof( int );
	EasyNewInt( glmesh, "num_cmd", m_num_cmd );

	m_cmds[0] = 1;		// BE_CMD_TRIANGLES
	m_cmds[1] = num_tri*3;	// vref num
	m_cmds[2] = 0;		// BE_CMD_END
	
	return glmesh;
}


/*
  ==============================
  ReadTriMeshFiles

  ==============================
*/
trimesh_t * ReadTriMeshFiles( char *vertex_name, char *tri_name )
{
	hobj_t		*vertex_root;
	hobj_t		*tri_root;
	
	hobj_search_iterator_t  iter;
	hobj_t			*tri;
	hobj_t			*vertex;
	hpair_t			*pair;
	trimesh_t	*mesh;
	
	mesh = NEWTYPE( trimesh_t );
	TriMesh_Init( mesh );
	
	vertex_root = ReadClassFile( vertex_name );	
	tri_root = ReadClassFile( tri_name );
	
	InitClassSearchIterator( &iter, tri_root, "face" );
	for ( ; ( tri = SearchGetNextClass( &iter ) ) ; )
	{
		unique_t	tri_id;
		unique_t	knot_ids[3];

		tri_id = StringToUnique( tri->name );

		pair = FindHPair( tri, "0" );
		if ( !pair )
			Error( "missing key '0' in tri '%s'\n", tri->name );
		knot_ids[0] = StringToUnique( pair->value );

		pair = FindHPair( tri, "1" );
		if ( !pair )
			Error( "missing key '1' in tri '%s'\n", tri->name );
		knot_ids[1] = StringToUnique( pair->value );

		pair = FindHPair( tri, "2" );
		if ( !pair )
			Error( "missing key '2' in tri '%s'\n", tri->name );
		knot_ids[2] = StringToUnique( pair->value );
		
		TriMesh_AddTri( mesh, tri_id, knot_ids );
	}

	InitClassSearchIterator( &iter, vertex_root, "vertex" );
	for ( ; ( vertex = SearchGetNextClass( &iter ) ) ; )
	{
		trimesh_vertex_t	*v;

		v = NEWTYPE( trimesh_vertex_t );

		v->id = StringToUnique( vertex->name );
		EasyFindVec3d( v->vec, vertex, "vec" );

		U_ListInsertAtHead( &mesh->vertex_list, v );
	}
	
	return mesh;
}

/*
  ==============================
  TriMesh_SetupAllKnotVertexRefs

  ==============================
*/
void TriMesh_SetupAllKnotVertexRefs( trimesh_t *mesh )
{
	u_list_iter_t		iter;
	trimesh_vertex_t	*v;
	trimesh_knot_t		*k;
	
	U_ListIterInit( &iter, &mesh->vertex_list );
	for ( ; ( v = U_ListIterNext( &iter ) ) ; )
	{
		k = U_MapSearch( &mesh->knot_map, &v->id );
		if ( !k )
		{
			v->value = -1.0;
			Vec3dInit( v->norm, 0, 0, 0 );
			continue;
		}

		k->vertex_ref = v;
	}
}

/*
  ==============================
  TriMesh_CalcAllTriNormals

  ==============================
*/
static void CalcTriNormalFunc( void *obj )
{
	trimesh_tri_t	*tri = obj;
	trimesh_vertex_t	*v1, *v2, *v3;
	vec3d_t		d1, d2;

	v1 = tri->knot_refs[0]->vertex_ref;
	v2 = tri->knot_refs[1]->vertex_ref;
	v3 = tri->knot_refs[2]->vertex_ref;

	if ( !v1 || !v2 || !v3 )
		Error( "not all knots have valid vertex_refs\n" );

	Vec3dSub( d1, v1->vec, v2->vec );
	Vec3dSub( d2, v3->vec, v2->vec );
	Vec3dUnify( d1 );
	Vec3dUnify( d2 );
	Vec3dCrossProduct( tri->norm, d1, d2 );
	Vec3dUnify( tri->norm );
//	printf( "tri_norm: %f\n", Vec3dLen( tri->norm ) );	
}
void TriMesh_CalcAllTriNormals( trimesh_t *mesh )
{
	U_MapForEach( &mesh->tri_map, CalcTriNormalFunc );
}

/*
  ==============================
  TriMesh_CalcTriArea

  ==============================
*/
fp_t TriMesh_CalcTriArea( trimesh_tri_t *tri )
{
	trimesh_vertex_t	*v1, *v2, *v3;
	vec3d_t			cross;
	fp_t			area;
	vec3d_t			d1, d2;

	v1 = tri->knot_refs[0]->vertex_ref;
	v2 = tri->knot_refs[1]->vertex_ref;
	v3 = tri->knot_refs[2]->vertex_ref;

	if ( !v1 || !v2 || !v3 )
		Error( "not all knots have valid vertex_refs\n" );
	
	
	Vec3dSub( d1, v2->vec, v1->vec );
	Vec3dSub( d2, v3->vec, v1->vec );	

	Vec3dCrossProduct( cross, d1, d2 );
	area = Vec3dLen( cross ) * 0.5;

	return area;
}

/*
  ==============================
  TriMesh_CalcAllKnotNormals

  ==============================
*/
static void CalcKnotNormalFunc_old( void *obj )
{
	trimesh_knot_t		*knot = obj;
	u_list_iter_t		iter;
	trimesh_tri_t		*t;
	fp_t			area, total_area, scale;
	vec3d_t			weight_norm;

	if ( !knot->vertex_ref )
		Error( "knot has no valid vertex_ref\n" );
	
	Vec3dInit( knot->vertex_ref->norm, 0, 0, 0 );

	// calc total area of tris around knot
	total_area = 0.0;
	U_ListIterInit( &iter, &knot->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		total_area += TriMesh_CalcTriArea( t );
	}

	U_ListIterInit( &iter, &knot->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		area = TriMesh_CalcTriArea( t );

		scale = area / total_area;
		Vec3dScale( weight_norm, scale, t->norm );
//		printf( "scale: %.2f\n", scale * 100.0 );

		Vec3dAdd( knot->vertex_ref->norm, knot->vertex_ref->norm, weight_norm );
	}
	Vec3dUnify( knot->vertex_ref->norm );
//	printf( "len: %f\n", Vec3dLen( knot->vertex_ref->norm ) );
}

static void CalcKnotNormalFunc( void *obj )
{
	trimesh_knot_t		*knot = obj;
	u_list_iter_t		iter;
	trimesh_tri_t		*t;
	fp_t			area, total_area, scale;
	vec3d_t			weight_norm;

	if ( !knot->vertex_ref )
		Error( "knot has no valid vertex_ref\n" );
	
	Vec3dInit( knot->vertex_ref->norm, 0, 0, 0 );

	// calc total area of tris around knot
	U_ListIterInit( &iter, &knot->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		Vec3dAdd( knot->vertex_ref->norm, knot->vertex_ref->norm, t->norm );
	}
	Vec3dUnify( knot->vertex_ref->norm );
}


void TriMesh_CalcAllKnotNormals( trimesh_t *mesh )
{
	U_MapForEach( &mesh->knot_map, CalcKnotNormalFunc );
}

/*
  ==============================
  TriMesh_CalcAllKnotValues

  ==============================
*/
static void CalcKnotValueFunc( void *obj )
{
	trimesh_knot_t		*knot = obj;
	u_list_iter_t		iter;
	trimesh_tri_t		*t;
	

	if ( !knot->vertex_ref )
		Error( "knot has no valid vertex_ref\n" );
	

	knot->vertex_ref->value = 0;
	U_ListIterInit( &iter, &knot->tri_list );
	for ( ; ( t = U_ListIterNext( &iter ) ) ; )
	{
		fp_t		v;

		v = (Vec3dDotProduct( knot->vertex_ref->norm, t->norm )+1.0)*0.5;
		knot->vertex_ref->value += v*v;		
	}
	
	knot->vertex_ref->value = (knot->vertex_ref->value)/(1.0*U_ListLength(&knot->tri_list) );
//	knot->vertex_ref->value = knot->vertex_ref->value * knot->vertex_ref->value;
//	printf( "%f\n", knot->vertex_ref->value );
}

void TriMesh_CalcAllKnotValues( trimesh_t *mesh )
{
	U_MapForEach( &mesh->knot_map, CalcKnotValueFunc );
}

/*
  ==============================
  TriMesh_VisitKnotTrisInOrder

  ==============================
*/
bool_t TriMesh_VisitKnotTrisInOrder( trimesh_knot_t *knot, void (*tri_func)(trimesh_tri_t *tri, trimesh_knot_t *knot ) )
{
	u_list_iter_t	iter;
	trimesh_tri_t	*tri, *start;
	int		k1, k2, k3;

	// get a tri from the knot's tri_list
	U_ListIterInit( &iter, &knot->tri_list );
	tri = U_ListIterNext( &iter );
	if ( !tri )
		Error( "no tri in knot\n" );

	start = tri;
	for(;;)
	{
		k1 = TriMesh_IsKnotRefInTri( tri, knot );
		k2 = (k1+1)>2?0:(k1+1);
		k3 = (k2+1)>2?0:(k2+1);

		tri_func( tri, knot );

		// go through edge k3-k1 to other
		tri = tri->other_refs[k3];
		if ( !tri )
		{
//			Error( "no other tri\n" );
			printf( "no other tri\n" );
			return false;
		}

		if ( tri == start )
			break;
	}	

	return true;
}

/*
  ==============================
  TriMesh_RemoveKnotAndFix

  ==============================
*/
static u_ring_t		g_knot_ring;
static u_ring_t		g_tri_ring;

fp_t TriMesh_CalcDistortion( vec3d_t v1, vec3d_t v2, vec3d_t v3 )
{
	vec3d_t		norm;
	fp_t		d, e;

#if 0
	Vec3dCopy( norm, v1 );
	Vec3dAdd( norm, norm, v2 );
	Vec3dAdd( norm, norm, v3 );
	Vec3dUnify( norm );

	d = ( Vec3dDotProduct( v1, norm )+1.0 ) * 0.5;
	e = d*d;
	d = ( Vec3dDotProduct( v2, norm )+1.0 ) * 0.5;
	e += d*d;
	d = ( Vec3dDotProduct( v3, norm )+1.0 ) * 0.5;
	e += d*d;

	e *= (1.0/3.0);
#else
	
	d = ( Vec3dDotProduct( v1, v2 )+1.0 ) * 0.5;
	d += ( Vec3dDotProduct( v1, v3)+1.0 ) * 0.5;
	d += ( Vec3dDotProduct( v2, v3 )+1.0 ) * 0.5;

	e = d*(1.0/3.0);	
#endif

	return e;
}

fp_t TriMesh_CalcTriDistortion( trimesh_knot_t *k1, trimesh_knot_t *k2, trimesh_knot_t *k3 )
{
	trimesh_vertex_t *v1, *v2, *v3;

	vec3d_t		norm;
	vec3d_t		cross;
	vec3d_t		d1, d2;
	fp_t		d;

	v1 = k1->vertex_ref;
	v2 = k2->vertex_ref;
	v3 = k3->vertex_ref;

	if ( !v1 || !v2 || !v3 )
		Error( "not all vertex_refs are vaild\n" );
      
	Vec3dSub( d1, v1->vec, v2->vec );
	Vec3dSub( d2, v3->vec, v2->vec );
	Vec3dUnify( d1 );
	Vec3dUnify( d2 );
	Vec3dCrossProduct( cross, d1, d2 );
	Vec3dUnify( cross );

	d = (Vec3dDotProduct( v1->norm, cross )+1.0)*0.5;
	d += (Vec3dDotProduct( v2->norm, cross )+1.0)*0.5;
	d += (Vec3dDotProduct( v2->norm, cross )+1.0)*0.5;
	
	d *= (1.0/3.0);

	return d;
}

fp_t TriMesh_CalcTriDistortion_old( trimesh_knot_t *k1, trimesh_knot_t *k2, trimesh_knot_t *k3 )
{
	trimesh_vertex_t *v1, *v2, *v3;

	vec3d_t		norm;
	vec3d_t		cross;
	vec3d_t		d1, d2;
	fp_t		d;

	v1 = k1->vertex_ref;
	v2 = k2->vertex_ref;
	v3 = k3->vertex_ref;

	if ( !v1 || !v2 || !v3 )
		Error( "not all vertex_refs are vaild\n" );
      
	Vec3dCopy( norm, v1->norm );
	Vec3dAdd( norm, norm, v2->norm );
	Vec3dAdd( norm, norm, v3->norm );
	Vec3dUnify( norm );

	Vec3dSub( d1, v1->vec, v2->vec );
	Vec3dSub( d2, v3->vec, v2->vec );
	Vec3dUnify( d1 );
	Vec3dUnify( d2 );
	Vec3dCrossProduct( cross, d1, d2 );
	Vec3dUnify( cross );

	d = (Vec3dDotProduct( norm, cross )+1.0)*0.5;
	
	return d;
}

static void VisitKnotFunc( trimesh_tri_t *tri, trimesh_knot_t *knot )
{
	int		k1, k2, k3;

	k1 = TriMesh_IsKnotRefInTri( tri, knot );
	k2 = (k1+1)>2?0:(k1+1);
	k3 = (k2+1)>2?0:(k2+1);
       
//	printf( "insert: #%u-#%u-#%u\n", tri->knot_refs[k1]->id, tri->knot_refs[k2]->id, tri->knot_refs[k3]->id );
	
	U_RingInsertAsNext( &g_knot_ring, tri->knot_refs[k2] );
	U_RingInsertAsNext( &g_tri_ring, tri );
}

void TriMesh_GLDBG_tri( trimesh_tri_t *tri )
{
	GLD_Begin( g_gldbg, "polygon" );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[0]->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[1]->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[2]->vertex_ref->vec );
	GLD_End( g_gldbg );
}

void TriMesh_GLDBG_knots( trimesh_knot_t *k1, trimesh_knot_t *k2, trimesh_knot_t *k3 )
{
	GLD_Begin( g_gldbg, "polygon" );
	GLD_Vertex3fv( g_gldbg, k1->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, k2->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, k3->vertex_ref->vec );
	GLD_End( g_gldbg );
}


void TriMesh_RemoveKnotAndFix( trimesh_t *mesh, trimesh_knot_t *rm_knot )
{
	int		i;
	int		num;
	trimesh_knot_t		*start, *knot, *prev, *next;
	trimesh_knot_t		*best_knot, *best_prev, *best_next;

	unique_t		knot_ids[3];
	static unique_t		tri_id_count = 1000000;

	fp_t		d, e;
	fp_t		best_v, v;
	bool_t		hack;


	if ( rm_knot->id == 31486 )
		hack = true;
	else
		hack = false;

	hack = false;

	U_InitRing( &g_knot_ring );
	U_InitRing( &g_tri_ring );

	if ( !TriMesh_VisitKnotTrisInOrder( rm_knot, VisitKnotFunc ) )
	{
		// there is a hole in the mesh, can't remove this knot ...
		return;
	}

	// test knot_ring
	num = U_RingSize( &g_knot_ring );
//	printf( "knot_ring size: %d\n", num );
	for ( i = 0; i < num; i++ )
	{
		knot = U_RingGetCurrent( &g_knot_ring );
//		printf( "knot_ring: #%u\n", knot->id );
		U_RingWalk( &g_knot_ring, 1 );
	}


	// remove all tris in tri_ring from trimesh
	GLD_BeginList( g_gldbg, "remove", "fill" );
	GLD_Color3f( g_gldbg, 1.0, 0.0, 0.0 );
	num = U_RingSize( &g_tri_ring );
	for ( i = 0; i < num; i++ )
	{
		trimesh_tri_t	*tri;
		tri = U_RingGetCurrent( &g_tri_ring );
//		printf( "remove: #%u\n", tri->id );

		if ( !hack )
			TriMesh_GLDBG_tri( tri );
		
		TriMesh_RemoveTri( mesh, tri );

		U_RingWalk( &g_tri_ring, 1 );
	}
	GLD_Color3f( g_gldbg, 1.0, 1.0, 1.0 );
	GLD_EndList( g_gldbg );
	
       
	for (;;)
	{
		
		num = U_RingSize( &g_knot_ring );

		if ( num > 3 )
		{
			best_v = -999999;
			best_knot = NULL;
			for ( i = 0; i < num; i++, U_RingWalk( &g_knot_ring, 1 ) )
			{
				trimesh_vertex_t	*v1, *v2, *v3;
				vec3d_t			d1, d2;
				
				prev = U_RingGetCurrentPrev( &g_knot_ring );
				knot = U_RingGetCurrent( &g_knot_ring );
				next = U_RingGetCurrentNext( &g_knot_ring );
				
				
				// check tri's orientation
				if ( TriMesh_DoEdgeExist( prev, knot ) ||
				     TriMesh_DoEdgeExist( knot, next ) ||
				     TriMesh_DoEdgeExist( next, prev ) )
				{
//					printf( "orientation check failed\n" );
					continue;
				}
				
				v1 = prev->vertex_ref;
				v2 = knot->vertex_ref;
				v3 = next->vertex_ref;
				
				if ( !v1 || !v2 || !v3 )
					Error( "not all knots have valid vertex_refs\n" );
				
				Vec3dSub( d1, v1->vec, v2->vec );
				Vec3dSub( d2, v3->vec, v2->vec );
				Vec3dUnify( d1 );
				Vec3dUnify( d2 );
				
				d = (Vec3dDotProduct( d1, d2 )+1.0)*0.5;

				e = TriMesh_CalcTriDistortion( prev, knot, next );

//				printf( "ring: #%u-#%u-#%u, d %f, e %f\n", prev->id, knot->id, next->id, d, e );
				
				
				v = e*1.0 + d*0.0;

				if ( 1 )
				{
					if ( v > best_v )
					{
						best_v = v;
						best_prev = prev;
						best_knot = knot;
						best_next = next;
					}										
				}								
			}

			if ( !best_knot || !best_prev || !best_next )
				Error( "can't find knot to remove\n" );
			
			if ( !U_RingSearchObj( &g_knot_ring, best_knot ) )
				Error( "? missing best_knot\n" );

			U_RingRemoveGoPrev( &g_knot_ring );

			
//			printf( "new tri #%u: #%u-#%u-#%u\n", tri_id_count, best_prev->id, best_knot->id, best_next->id );

			knot_ids[0] = best_prev->id;
			knot_ids[1] = best_knot->id;
			knot_ids[2] = best_next->id;
			
//			printf( "tri distortion: %.2f\n", TriMesh_CalcTriDistortion( best_prev, best_knot, best_next ) );
			
			if ( !hack )
			{
				TriMesh_AddTri( mesh, tri_id_count++, knot_ids );
			}
			else
			{
				TriMesh_GLDBG_knots( best_prev, best_knot, best_next );
			}
			
		}
		else if ( num == 3 )
		{
			prev = U_RingGetCurrentPrev( &g_knot_ring );
			knot = U_RingGetCurrent( &g_knot_ring );
			next = U_RingGetCurrentNext( &g_knot_ring );
			U_CleanUpRing( &g_knot_ring, NULL );

//			printf( "last new tri #%u: #%u-#%u-#%u\n", tri_id_count, prev->id, knot->id, next->id );
//			printf( "tri distortion: %.2f\n", TriMesh_CalcTriDistortion( prev, knot, next ) );
			knot_ids[0] = prev->id;
			knot_ids[1] = knot->id;
			knot_ids[2] = next->id;

			if ( !hack )
			{
				TriMesh_AddTri( mesh, tri_id_count++, knot_ids );
			}
			else
			{
				TriMesh_GLDBG_knots( prev, knot, next );
			}
			
			break;
		}
		else
		{
			Error( "less than 3 knots\n" );
		}
	}

	if ( hack )
	{
			GLD_Color3f( g_gldbg, 1.0, 1.0, 1.0 );
			GLD_EndList( g_gldbg );			
	}

}

/*
  ==============================
  TriMesh_GLDBG

  ==============================
*/

static void GLDBGFunc( void *obj )
{
	trimesh_tri_t	*tri = obj;

	GLD_Begin( g_gldbg, "polygon" );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[0]->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[1]->vertex_ref->vec );
	GLD_Vertex3fv( g_gldbg, tri->knot_refs[2]->vertex_ref->vec );
	GLD_End( g_gldbg );
}

void TriMesh_GLDBG( trimesh_t *mesh )
{
	U_MapForEach( &mesh->tri_map, GLDBGFunc );
}

#if 0
int main()
{
	trimesh_t	*trimesh;
	unique_t	knot_ids[3];
	trimesh_tri_t	*t1, *t2, *t3, *t4, *t5;

#if 0
	TriMesh_Init( &trimesh );
	
	knot_ids[0] = 0;
	knot_ids[1] = 1;
	knot_ids[2] = 2;
	t1 = TriMesh_AddTri( &trimesh, 1, knot_ids );

	knot_ids[0] = 0;
	knot_ids[1] = 2;
	knot_ids[2] = 3;
	t2 = TriMesh_AddTri( &trimesh, 2, knot_ids );

	knot_ids[0] = 0;
	knot_ids[1] = 3;
	knot_ids[2] = 4;
	t3 = TriMesh_AddTri( &trimesh, 3, knot_ids );

	knot_ids[0] = 0;
	knot_ids[1] = 4;
	knot_ids[2] = 5;
	t4 = TriMesh_AddTri( &trimesh, 4, knot_ids );

	knot_ids[0] = 0;
	knot_ids[1] = 5;
	knot_ids[2] = 1;
	t5 = TriMesh_AddTri( &trimesh, 5, knot_ids );

	TriMesh_RemoveKnotAndFix( &trimesh, t5->knot_refs[0] );

//	TriMesh_DumpTri( t1 );
//	TriMesh_DumpTri( t2 );
//	TriMesh_DumpTri( t3 );
//	TriMesh_Dump( &trimesh );

//	TriMesh_RemoveTri( &trimesh, t2 );
//	TriMesh_RemoveTri( &trimesh, t1 );
//	TriMesh_RemoveTri( &trimesh, t3 );


#else
	trimesh = ReadTriMeshFiles( "stone2_vertex.hobj", "stone2_face.hobj" );

	TriMesh_SetupAllKnotVertexRefs( trimesh );
	TriMesh_CalcAllTriNormals( trimesh );
	TriMesh_CalcAllKnotNormals( trimesh );
	TriMesh_CalcAllKnotValues( trimesh );

	TriMesh_Dump( trimesh );

	g_gldbg = GLD_BeginSession( "xxx" );
	GLD_StartRecord( g_gldbg );

	{
		u_list_iter_t	iter;
		trimesh_vertex_t	*v, *best_v;
		fp_t		best_value;
		fp_t		max_value;
		int		i;

//		max_value = 99999.9;

		GLD_BeginList( g_gldbg, "yyy", "line" );
		TriMesh_GLDBG( trimesh );
		GLD_EndList( g_gldbg );
		GLD_Update( g_gldbg );
		GLD_Pause( g_gldbg );

		for ( i = 0; i < 950; i++ )
		{

			
			best_value = -1.0;	
			best_v = NULL;
			U_ListIterInit( &iter, &trimesh->vertex_list );
			for ( ; ( v = U_ListIterNext( &iter ) ) ; )
			{
//				printf( "%f\n", v->value );
				if ( v->value > best_value && v->value > 0.0 )
				{
					best_value = v->value;
					best_v = v;
				}
			}
			
			if ( best_v )
			{
				trimesh_knot_t	*knot;
				
				knot = U_MapSearch( &trimesh->knot_map, &best_v->id );
				if ( knot )
				{
					printf( "remove %d: knot #%u with value %f\n", i, knot->id, best_value );
					
					TriMesh_RemoveKnotAndFix( trimesh, knot );
				}
				
				max_value = best_value;
			}

	TriMesh_SetupAllKnotVertexRefs( trimesh );
	TriMesh_CalcAllTriNormals( trimesh );
	TriMesh_CalcAllKnotNormals( trimesh );
	TriMesh_CalcAllKnotValues( trimesh );



		}
			GLD_BeginList( g_gldbg, "yyy", "line" );
			TriMesh_GLDBG( trimesh );
			GLD_EndList( g_gldbg );
			GLD_Update( g_gldbg );
			GLD_Pause( g_gldbg );

	}

	GLD_EndSession( g_gldbg );
#endif
	
	TriMesh_Dump( trimesh );


}
#endif
