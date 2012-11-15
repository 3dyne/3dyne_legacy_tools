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



// lib_mesh.c

#include "lib_mesh.h"

/*
  ====================
  NewUVMesh

  ====================
*/
uvmesh_t* NewUVMesh( int usize, int vsize )
{
	uvmesh_t	*mesh;
	int		size;

	size = (int) (long int) ((uvmesh_t *)0)->p[usize*vsize];
	mesh = (uvmesh_t *) malloc( size );
	memset( mesh, 0, size );

	mesh->upointnum = usize;
	mesh->vpointnum = vsize;

	return mesh;
}



/*
  ====================
  FreeUVMesh

  ====================
*/
void FreeUVMesh( uvmesh_t *mesh )
{
	free( mesh );
}



/*
  ====================
  SetUVMeshPoint

  ====================
*/
void SetUVMeshPoint( uvmesh_t *mesh, int u, int v, vec3d_t pos )
{
	Vec3dCopy( mesh->p[u+v*mesh->upointnum], pos );
}



/*
  ====================
  GetUVMeshPoint

  ====================
*/
void GetUVMeshPoint( uvmesh_t *mesh, int u, int v, vec3d_t pos )
{
	Vec3dCopy( pos, mesh->p[u+v*mesh->upointnum] );
}



/*
  ====================
  Subdivied2U2VMeshTo3U3V

  ====================
*/
uvmesh_t* Subdivied2U2VMeshTo3U3V( uvmesh_t *in )
{
	uvmesh_t	*out;
	vec3d_t		p, q;

	if ( in->upointnum != 2 &&
	     in->vpointnum != 2 )
		return NULL;

	out = NewUVMesh( 3, 3 );
       
	GetUVMeshPoint( in, 0, 0, p );
	SetUVMeshPoint( out, 0, 0, p );
	GetUVMeshPoint( in, 1, 0, p );
	SetUVMeshPoint( out, 2, 0, p );
	GetUVMeshPoint( in, 0, 1, p );
	SetUVMeshPoint( out, 0, 2, p );
	GetUVMeshPoint( in, 1, 1, p );
	SetUVMeshPoint( out, 2, 2, p );

	// out10 = (in00 + in10 ) / 2
	GetUVMeshPoint( in, 0, 0, p );
	GetUVMeshPoint( in, 1, 0, q );
	Vec3dAdd( p, p, q );
	Vec3dScale( p, 0.5, p );
	SetUVMeshPoint( out, 1, 0, p );
		
	// out12 = (in01 + in11 ) / 2
	GetUVMeshPoint( in, 0, 1, p );
	GetUVMeshPoint( in, 1, 1, q );
	Vec3dAdd( p, p, q );
	Vec3dScale( p, 0.5, p );
	SetUVMeshPoint( out, 1, 2, p );

	// out01 = (in00 + in01 ) / 2
	GetUVMeshPoint( in, 0, 0, p );
	GetUVMeshPoint( in, 0, 1, q );
	Vec3dAdd( p, p, q );
	Vec3dScale( p, 0.5, p );
	SetUVMeshPoint( out, 0, 1, p );

	// out21 = (in10 + in11 ) / 2
	GetUVMeshPoint( in, 1, 0, p );
	GetUVMeshPoint( in, 1, 1, q );
	Vec3dAdd( p, p, q );
	Vec3dScale( p, 0.5, p );
	SetUVMeshPoint( out, 2, 1, p );

	// out11 = (in00 + in10 + in01 + in11 ) / 4
	GetUVMeshPoint( in, 0, 0, p );
	GetUVMeshPoint( in, 1, 0, q );
	Vec3dAdd( p, p, q );
	GetUVMeshPoint( in, 0, 1, q );
	Vec3dAdd( p, p, q );
	GetUVMeshPoint( in, 1, 1, q );
	Vec3dAdd( p, p, q );
	Vec3dScale( p, 0.25, p );
	SetUVMeshPoint( out, 1, 1, p );
    
	return out;
}
