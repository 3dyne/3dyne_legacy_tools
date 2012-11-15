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



// lib_math.h
//		formerly known as vec.h

#ifndef __lib_math
#define __lib_math

#ifdef __cplusplus
extern "C"
{
#endif

#ifndef ON_EPSILON
#define ON_EPSILON	( 0.01 ) // 0.01
#endif

typedef int	ivec3d_t[3];
	
typedef float	fp_t;

typedef	fp_t	vec5d_t[5];
typedef fp_t	vec4d_t[4];
typedef fp_t	vec3d_t[3];
typedef fp_t	vec2d_t[2];       

typedef fp_t	matrix3_t[3][3];


// fix me: is this the right place for bool_t ?
#ifndef __cplusplus
typedef enum {
	false = 0,
	true = 1
} bool_t;
#else
typedef bool	bool_t;
#endif

#if 0
typedef struct {
	fp_t		dist;
	vec3d_t		norm;

} plane_t; 
#endif

//
// some macros ...
//	
#define	_Vec3dInit( v, x,y,z )	{v[0]=x, v[1]=y, v[2]=z;}
#define _Vec2dCopy( out, in )	{ out[0]=in[0], out[1]=in[1]; }
#define _Vec3dCopy( out, in )	{ out[0]=in[0], out[1]=in[1], out[2]=in[2]; }
#define _Vec4dCopy( out, in )	{ out[0]=in[0], out[1]=in[1], out[2]=in[2], out[3]=in[3]; }
#define _Vec3dAdd( out, in1, in2 )	{ out[0]=in1[0]+in2[0], out[1]=in1[1]+in2[1], out[2]=in1[2]+in2[2]; }
#define _Vec3dSub( out, in1, in2 )	{ out[0]=in1[0]-in2[0], out[1]=in1[1]-in2[1], out[2]=in1[2]-in2[2]; }
#define _Vec3dScale( out, s, in )	{ out[0]=in[0]*(s), out[1]=in[1]*(s), out[2]=in[2]*(s); }
#define _Vec3dMA( out, s, in1, in2 )	{ out[0]=(s)*in1[0]+in2[0], out[1]=(s)*in1[1]+in2[1], out[2]=(s)*in1[2]+in2[2]; }
#define _Vec3dDotProduct( in1, in2 )	( in1[0]*in2[0]+in1[1]*in2[1]+in1[2]*in2[2] )

#define _IVec3dCopy( out, in )	{ out[0]=in[0], out[1]=in[1], out[2]=in[2]; }

#define _UnitSnap( x, snap ) ( (x>=0) ? (x/snap) : (((x+1)/snap)-1))

#define _Rint( x ) ( floor( 0.5+x ) )

//
// print vectors 
//

void Vec2dPrint( vec2d_t in );
void Vec3dPrint( vec3d_t in );
void VecndPrint( fp_t *in, int num );

void IVec3dPrint( ivec3d_t in );

//
// init vectors
//

void Vec2dInit( vec2d_t out, fp_t in1, fp_t in2 );
void Vec3dInit( vec3d_t out, fp_t in1, fp_t in2, fp_t in3 );

//
// copy vectors: out = in
//

void Vec2dCopy( vec2d_t out, vec2d_t in );
void Vec3dCopy( vec3d_t out, vec3d_t in );
void Vec4dCopy( vec4d_t out, vec4d_t in );
void Vec5dCopy( vec5d_t out, vec5d_t in );
void VecndCopy( fp_t *out, fp_t *in, int num );

void IVec3dCopy( ivec3d_t out, ivec3d_t in );

//
// round to nearest integer
//       
void Vec2dRint( vec2d_t out, vec2d_t in );
void Vec3dRint( vec3d_t out, vec3d_t in );

void IVec3dRint( ivec3d_t out, vec3d_t in );

void IVec3dUnitSnap( ivec3d_t out, ivec3d_t in, int snap );	// snap to floor

//
// add vectors: out = in1 + in2
//

void Vec2dAdd( vec2d_t out, vec2d_t in1, vec2d_t in2 );
void Vec3dAdd( vec3d_t out, vec3d_t in1, vec3d_t in2 );
void VecndAdd( fp_t *out, fp_t *in1, fp_t *in2, int num );

//
// sub vectors: out = in1 - in2
//
void Vec2dSub( vec2d_t out, vec2d_t in1, vec2d_t in2 );
void Vec3dSub( vec3d_t out, vec3d_t in1, vec3d_t in2 );
void VecndSub( fp_t *outm, fp_t *in1, fp_t *in2, int num );

//
// scale vectors: out = scale * in
//

void Vec2dScale( vec2d_t out, fp_t scale, vec2d_t in );
void Vec3dScale( vec3d_t out, fp_t scale, vec3d_t in );
void VecndScale( fp_t *out, fp_t scale, fp_t *in, int num );

//
// flip vector: out = -in
//
void Vec2dFlip( vec2d_t out, vec2d_t in );
void Vec3dFlip( vec3d_t out, vec3d_t in );

//
// multiply and add: out = scale * in1 + in2
//
void Vec2dMA( vec2d_t out, fp_t scale, vec2d_t in1, vec2d_t in2 );
void Vec3dMA( vec3d_t out, fp_t scale, vec3d_t in1, vec3d_t in2 );

//
// vector length
//
fp_t Vec2dLen( vec2d_t in );
fp_t Vec3dLen( vec3d_t in );
fp_t VecndLen( fp_t *in, int num );

//
// normalize vectors
//
void Vec2dUnify( vec2d_t out, vec2d_t in );
fp_t Vec3dUnify( vec3d_t inout );
void Vec3dUnify2( vec3d_t out, vec3d_t in );

//
// dot product of two vectors: scale = in1 o in2
//
fp_t Vec2dDotProduct( vec2d_t in1, vec2d_t in2 );
fp_t Vec3dDotProduct( vec3d_t in1, vec3d_t in2 );

//
// cross product of two vectors: out = in1 x in2
//
void Vec2dLeft( vec2d_t out, vec2d_t in );
void Vec3dCrossProduct( vec3d_t out, vec3d_t in1, vec3d_t in2 );

//
// colinear check
// 
bool_t	Vec3dCheckColinear( vec3d_t p1, vec3d_t p2, vec3d_t t );

//
// init plane from three vectors
//
fp_t Vec3dInitPlane2( vec3d_t norm, vec3d_t pos );
bool_t Vec3dInitPlane( vec3d_t norm, fp_t *dist, vec3d_t in1, vec3d_t in2, vec3d_t in3 );

bool_t Vec2dInitStraight( vec2d_t norm, fp_t *dist, vec2d_t in1, vec2d_t in2 );

typedef enum 
{
	libMathNormType_x = 0,
	libMathNormType_y = 1,
	libMathNormType_z = 2,
	libMathNormType_axis_mask = 3,
	libMathNormType_exact = 4,
	libMathNormType_pos = 8
} libMathNormType;

libMathNormType LibMath_GetNormType( vec3d_t norm );

//
// project
//

void Vec3dProjectOnPlane( vec3d_t out, vec3d_t norm, fp_t dist, vec3d_t in );
void Vec3dProjectOrthoOnPlane( vec3d_t out, vec2d_t in, vec3d_t norm, fp_t dist, libMathNormType type );

//
// bounding box
//
void Vec2dInitBB( vec2d_t min, vec2d_t max, fp_t pos );
void Vec2dAddToBB( vec2d_t min, vec2d_t max, vec2d_t in );

void Vec3dInitBB( vec3d_t min, vec3d_t max, fp_t pos );
void Vec3dAddToBB( vec3d_t min, vec3d_t max, vec3d_t in );

//
// matrix
//

void Matrix3Concat( matrix3_t out, matrix3_t in1, matrix3_t in2 );
void Matrix3SetupRotate( matrix3_t out, fp_t roll, fp_t pitch, fp_t yaw );
void Matrix3Vec3dRotate( vec3d_t out, vec3d_t in, matrix3_t matrix );

#ifdef __cplusplus
}
#endif

#endif
