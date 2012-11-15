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



// brush.h

#ifndef __brush_included
#define __brush_included

#ifdef __cplusplus
extern "C"
{
#endif

#include "vec.h"
#include "texture.h"

#include "lib_unique.h"

/*
typedef struct {
	vec3d_t		norm;
	float		dist;
} plane_t;
*/

// =========================================================

#define		MAX_POINTS_ON_POLYGON	( 256 )

#define		SIDE_BACK	( 0 )	// for ClipFunctions
#define		SIDE_FRONT	( 1 )
#define		SIDE_ON		( 2 )

#define		ON_EPSILON	( 0.01 )

typedef struct {
	int		pointnum;
	vec5d_t		p[8];
} polygon_t;

typedef struct face_s {
	unique_t	id;
	
	plane_t		plane;
	polygon_t	*polygon;

	texturedef_t	texdef;
	unsigned int	contents;

	struct face_s	*next;
} face_t;

typedef struct brush_s {
	unique_t	id;

//	int		status;
//	int		visible;

	int		select;

	unsigned int	contents;

	vec3d_t		min, max;
	face_t		*faces;	
	struct brush_s	*next;
} brush_t;

polygon_t*	NewPolygon( int points );
void		FreePolygon( polygon_t *polygon );
face_t*		NewFace( void );
void		FreeFace( face_t *face );
//face_t*		NewFaceFromFace( face_t *original );
brush_t*	NewBrush();
void		FreeBrush( brush_t *brush );
void		DumpStat( void );

void		PolygonCenter( polygon_t *in, vec3d_t center );

void		TextureAxisFromPlane( plane_t *, float *, float * );
polygon_t*	BasePolygonForPlane( plane_t *plane, texturedef_t *texdef  );
polygon_t*	ClipPolygonByPlane( polygon_t *polygon, plane_t *plane );
void		SplitPolygonByPlane( polygon_t *polygon, plane_t *plane, polygon_t **front, polygon_t **back );

void		ClipBrushFaces( brush_t *brush );
void		CalcBrushBounds( brush_t *brush );
void		SplitBrushByPlane( brush_t *brush, plane_t *plane, brush_t **front, brush_t **back );

int		AddFaceToBrush( brush_t *brush, face_t *fnew ); // with test ( used by clipper )
int		CheckBrushAndPlane( brush_t *brush, plane_t *plane ); // ( used by face move )
void		CleanUpBrush( brush_t *brush );
void		CopyBrush( brush_t **out, brush_t *in );

void		FreeBrushPolygons( brush_t *brush );
void		FreeBrushFaces( brush_t *brush );

face_t*		ClipRayByBrush( brush_t *brush, vec3d_t from, vec3d_t to );

void	PlaneFromPolygon( polygon_t *in, vec3d_t norm, fp_t *dist );


#ifdef __cplusplus
}
#endif

#endif 
