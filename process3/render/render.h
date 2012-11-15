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



// render.h

#ifndef __render
#define __render

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
//#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"
#include "arr.h"

#include "defs.h"

#include "../csg/cbspbrush.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "sys_dep.h"

#include "r_defs.h"
#include "r_state.h"

#include "r_math.h"
#include "r_frustum.h"

#include "g_mapdefs.h"
#include "g_trace.h"

#include "r_lplayout.h"

//#define GL_LOCK_ARRAYS_EXT
//#define GL_ARB_MULTITEXTURE_EXT


#define RND	( (_Random()%1000)/1000.0 )

#define	SIZE_X		( 640 )
#define SIZE_Y		( 480 )

//
// a_main.c
//

extern g_map_t *a_map;

//
// r_lightmap.c
//

// lightdefs

typedef enum
{
	LightmapType_diffuse,
	LightmapType_specular
} lightmapType;

typedef struct lightmapinfo_s
{
	lp_box_t	*box;	// for layouting

	int		lightpage;
	int		xofs;
	int		yofs;

	fp_t		xofs2;		// = xofs/128.0
	fp_t		yofs2;		// = yofs/128.0
	
	lightmapType	type;
} lightmapinfo_t;

typedef struct lightdef_s
{
	hobj_t		*self;

	int		width;
	int		height;

	vec2d_t		shift;

	fp_t		patchsize;
	fp_t		scale;		// = 1.0 / (patchsize * 128.0) OPT

	// valid after FillLightPageRecursive
	int		lightmapnum;
	lightmapinfo_t	lightmaps[4];

	
	// hack
	projectionType	projection;

} lightdef_t;


#define MAX_LIGHTDEFS	( 8192 )

extern int		r_lightdefnum;
extern lightdef_t	r_lightdefs[];

// lightpages
#define LIGHTPAGE_WIDTH		( 128 )
#define LIGHTPAGE_HEIGHT	( 128 )

typedef struct lightpage_s
{
	int		texobj;
	unsigned short	image[LIGHTPAGE_WIDTH*LIGHTPAGE_HEIGHT];
} lightpage_t;


#define MAX_LIGHTPAGES	( 64 )

extern int		r_lightpagenum;
extern lightpage_t	r_lightpages[];

void CompileLightdefClass( hmanager_t *lightdefhm );
void SetupLightPages( void );

//
// r_texture.c
//

#define TEXTURE_PATH	"/mnt/gs/agony1/arch00/textures"
//#define TEXTURE_PATH	"/usr/tmp/agony1/arch00/textures/"
//#define TEXTURE_PATH	"/e/3d/agony1/arch00/textures/"
#define ART_PATH	"/home/mcb/art/"
//#define ART_PATH	"/e/3d/art/"

typedef struct texture_s
{
	hobj_t		*self;
	
	arr_t		*arr;
	fp_t		inv_width;
	fp_t		inv_height;
	int		texobj;

	// special hacks
	bool_t		is_sky;
} texture_t;

#define MAX_TEXTURES	( 256 )

extern int		r_texturenum;
extern texture_t	r_textures[MAX_TEXTURES];

void CompileTextureClass( hmanager_t *texturehm );
void SetupTextures( void );

//
// r_texdef.c
//

typedef struct texdef_s
{
	int		texture;

	vec2d_t		shift;
	vec2d_t		vecs[2];
	
	projectionType	projection;
} texdef_t;

#define MAX_TEXDEFS		( 1024 )

extern int		r_texdefnum;
extern texdef_t		r_texdefs[];

void CompileTexdefClass( hmanager_t *texdefhm, hmanager_t *texturehm );

//
// r_misc.c
//

unsigned char* Image565ToImage888( unsigned short *in, int pixelnum );
GLuint Misc_GenTexture_TGA_8888( char *name );
void ProjectVec3d( vec2d_t out, vec3d_t in, projectionType type );
void CalcVertex( vec4d_t out, vec3d_t in );

//
// r_field.c
//
void SetupField( char *plane_name, char *node_name );
void RunParticle( void );
void FieldTest( vec3d_t pos );

//
// r_initgl.c
//
void R_InitGL( void );

#endif
