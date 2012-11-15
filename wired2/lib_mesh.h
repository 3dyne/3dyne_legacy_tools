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



// lib_mesh.h

#ifndef __lib_mesh
#define __lib_mesh

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

#include "lib_math.h"

#ifdef __cplusplus
extern "C"
{
#endif


typedef struct uvmesh_s
{
	int		upointnum;
	int		vpointnum;
	vec3d_t		p[4];		// varibale sized
} uvmesh_t;

uvmesh_t* NewUVMesh( int usize, int vsize );
void FreeUVMesh( uvmesh_t *mesh );

void SetUVMeshPoint( uvmesh_t *mesh, int u, int v, vec3d_t pos );
void GetUVMeshPoint( uvmesh_t *mesh, int u, int v, vec3d_t pos );

//uvmesh_t* New2U2VMeshFromPolygon( polygon_t *in );
uvmesh_t* Subdivied2U2VMeshTo3U3V( uvmesh_t *in );

#ifdef __cplusplus
}
#endif

#endif
