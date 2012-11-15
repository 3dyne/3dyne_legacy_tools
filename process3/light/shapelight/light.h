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



// light.h

#ifndef __light
#define __light

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"
#include "lib_bezier.h"
#include "lib_container.h"

#include "lib_mprof.h"
#include "lib_math_3dnow.h"

#include "../../shared/defs.h"

#include "hashmap.h"

// hack: need list head for all faces on a plane
#define HAVE_OWN_CPLANE_TYPE
typedef struct cplane_s
{
	hobj_t		*self;

	vec3d_t		norm;
	fp_t		dist;
	int		type;

	struct cplane_s	*flipplane;

	// internal
	int		count;	// for bsp quick test

	// hack
	struct face_s	*facehead;

} cplane_t;

#include "../../csg/cbspbrush.h"

typedef enum
{
	ProjectionType_X = 0,
	ProjectionType_Y = 1,
	ProjectionType_Z = 2
} projectionType;

typedef struct world_scrap_s
{
	int	x, y;
	fp_t	area;

	// plane and patchsize comes from face
	// c_comp and s_comp comes from patch

	struct world_scrap_s	*next;
} world_scrap_t;

typedef struct patch_s
{
//	polygon_t	*p;
	int		x, y;
	// patch polygon center for raytracing
//	vec3d_t		origin;	// real center
	vec3d_t		trace_origin; // shifted center
	
	// patch center for lighting
	vec3d_t		light_origin;

	vec3d_t		norm;
	
//	vec3d_t		color;
//	vec3d_t		spec;	

	fp_t		intens_diffuse;
	fp_t		intens_specular;

	struct patch_s	*next;
	world_scrap_t	*scrap_head;
} patch_t;


typedef struct material_s
{
	bool_t		no_light;
	bool_t		self_light;	// self_color is immediatly copied to the patches
	bool_t		emit_light;
	vec3d_t		emit_color;
	fp_t		emit_value;
	fp_t		spec_pow;
	bool_t		is_sky;	
} material_t;

typedef struct face_s
{
	// below data is setup by CompileBrushClass
	hobj_t		*shape;
	hobj_t		*self;
	hobj_t		*lightdef;	// created by FaceListBuildLightdefs

	cplane_t	*pl;	
	polygon_t	*p;

	struct face_s	*next;
	struct face_s	*next2;		// list sorted by plane

	// BuildFacePatches
	projectionType	projection;
	vec2d_t		min2d, max2d;
	vec3d_t		min3d, max3d;
	
	vec2d_t		wmin, wmax;	// lightmap covers at least this rectangle
	int		width, height;

	fp_t		patchsize;
	struct patch_s	*patches;
	int		count;

	material_t	mat;
} face_t;

typedef struct bsurface_s
{
	// below data is setup by CompileCSurfaceClass
	hobj_t		*shape;
	hobj_t		*self;
	hobj_t		*lightdef;	// created by CSurfListBuildLightdefs

	surface_ctrl_t	*ctrl;	

	int		width, height;	// lightmap

	vec3d_t		min3d, max3d;

	material_t	mat;

	struct bsurface_s	*next;

	// BuildCSurfacePatches
	struct patch_s	*patches;
	
} bsurface_t;

// misc in light.c
projectionType	GetProjectionTypeOfPlane( cplane_t *pl );
void		GetProjectionVecs( vec3d_t right, vec3d_t up, projectionType type );
void		ProjectVec3d( vec2d_t out, vec3d_t in, projectionType type );

// trace.c
typedef enum
{
	NodeType_node = 0,
	NodeType_solid,
	NodeType_empty
} nodeType;

typedef struct node_s
{
	hobj_t		*self;
	nodeType	type;
	bool_t		is_sky;
	bool_t		ignore;		// if set, ignore solid, tread as empty

	cplane_t	*pl;
	int		child[2];
	
} node_t;

void CompileNodeClass( hmanager_t *nodehm, hmanager_t *planehm );
bool_t TraceLine( vec3d_t from, vec3d_t to );
node_t *FindLeafForPoint( vec3d_t pos );
int TraceLine2( vec3d_t from, vec3d_t to );
void TraceLine2_GetNearestSplit( vec3d_t from, vec3d_t split, node_t **n );
void SetupForSkyTrace( face_t *list, hmanager_t *brushhm );


// patch.c
patch_t * NewPatch( void );
void SetupPatches( face_t *list, bool_t fixedsize );

// csurface.c
bsurface_t * CompileBSurfaceClass( hmanager_t *bsurfacehm );
void SetupBSurfacePatches( bsurface_t *list );

bsurface_t * BuildSurfaceLightList( hmanager_t *shapehm );

// lighting.c
void	Lighting_SetConvergence( fp_t conv );
void	Lighting_SetupPointLight( hobj_t *light );
void	Lighting_SetupFaceLight( face_t * );
void	Lighting_SetupSpotLight( hobj_t *light );
bool_t	Lighting_CullTest( vec3d_t min, vec3d_t max, vec3d_t norm, fp_t dist );
void	Lighing_FacePatches( face_t *f );
void Lighting_Volume( map3_t *map );

hobj_t * Lighting_CreateLightSource();

// lightmap.c
void	BuildLightmaps( face_t *list );

// sky.c
void SkyLight( face_t *list, vec3d_t dir, vec3d_t color );

// pvs.c
typedef struct listnode_s
{
	face_t		*face;
	struct listnode_s	*next;
} listnode_t;


typedef struct mapnode_s 
{
	hobj_t		*self;

	cplane_t	*pl;

	// node
	int		firstbit, lastbit;	// continues bitpos of all leafs in this node
	int		child[2];


	// empty leaf
	vec3d_t		center;

	int		visleaf;	// for debug draw, set by CompileVisleafClass

	bool_t		visinfo;
	unsigned char	can_see[4096];
	int		leafrefnum;
	int		startleafref;

	listnode_t		*faces;

} mapnode_t;
extern int		r_visleafnum;
extern int		r_leafbitpos[];
extern mapnode_t	r_mapnodes[];

mapnode_t * PVS_FindMapnode( vec3d_t point );

void Balance_AddWorldPatchScrap( fp_t scrap_area, cplane_t *pl, int x, int y, fp_t patchsize, fp_t c_comp, fp_t s_comp );
bool_t Balance_GetWorldPatch( cplane_t *pl, int x, int y, fp_t patchsize, fp_t *c_comp, fp_t *s_comp );

// lightmap
void Lightmap_Begin( char *bin_name, char *cls_name );
void Lightmap_Add( void *data, int size, hobj_t *shape, hobj_t *source, int type );
void Lightmap_End( void );

#endif
