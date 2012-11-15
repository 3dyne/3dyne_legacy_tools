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



// render.c

#include <stdio.h>                                                              
#include <string.h>                                                             
#include <sys/types.h>                                                          
#include <signal.h>                                                             
#include <unistd.h>     
#include <math.h>
#include <values.h> // MININT, MAXINT

#include "vec.h"
#include "shock.h"
#include "support.h"

#include "ui_service.h"

typedef	float	vec5d_t[5];

int		r_width;
int		r_height;
int		r_width_2;
int		r_height_2;

float		r_matrix[9];
vec3d_t		r_origin;

plane_t		r_frustum[5];

int		t_pointnum;
vec5d_t		t_points[8];

char		*t_data; //[128][128];

// Prototypes
void Vec5dInit( vec5d_t, float, float, float, float, float );

void R_SetMatrix( float, float, float );
void R_SetOrigin( vec3d_t );
void R_Vec5dRot( vec5d_t );
void R_Vec5dPer( vec5d_t );

void R_BaseFrustum();


void Vec5dInit( vec5d_t v, float a, float b, float c, float d, float e )
{
	v[0] = a;
	v[1] = b;
	v[2] = c;
	v[3] = d;
	v[4] = e;
}

void Vec5dCopy( vec5d_t vout, vec5d_t vin )
{
	vout[0] = vin[0];
	vout[1] = vin[1];
	vout[2] = vin[2];
	vout[3] = vin[3];
	vout[4] = vin[4];
}

void R_SetMatrix( float alpha, float beta, float gamma )
{
	float	alphasin, alphacos;
	float	betasin, betacos;
	float	gammasin, gammacos;

	alphasin = sin( alpha ); // six
	alphacos = cos( alpha ); // cox
	betasin  = sin( beta );  // siy
	betacos  = cos( beta );  // coy
	gammasin = sin( gamma ); // siz
	gammacos = cos( gamma ); // coz
	
	r_matrix[0] =   betacos * gammacos;
	r_matrix[1] = -(betacos * gammasin);
	r_matrix[2] =   betasin;
	r_matrix[3] =   alphasin * betasin * gammacos  -  alphacos * gammasin;
	r_matrix[4] = -(alphasin * betasin * gammasin  +  alphacos * gammacos);
	r_matrix[5] = -(alphasin * betacos);
	r_matrix[6] = -(alphacos * betasin * gammacos  +  alphasin * gammasin);
	r_matrix[7] =   alphacos * betasin * gammasin  -  alphasin * gammacos;
	r_matrix[8] =   alphacos * betacos;	  	
}

void R_SetOrigin( vec3d_t v ) 
{
	r_origin[0] = v[0];
	r_origin[1] = v[1];
	r_origin[2] = v[2];
}

void R_Vec5dRot( vec5d_t v )
{
	vec3d_t		temp;

	Vec3dSub( v, v, r_origin );
	
	temp[0] = v[0]*r_matrix[0] + v[1]*r_matrix[1] + v[2]*r_matrix[2];
	temp[1] = v[0]*r_matrix[3] + v[1]*r_matrix[4] + v[2]*r_matrix[5];
	temp[2] = v[0]*r_matrix[6] + v[1]*r_matrix[7] + v[2]*r_matrix[8];

        v[0] = temp[0];
	v[1] = temp[1];
	v[2] = temp[2];
}

void R_Vec5dPer( vec5d_t v )
{
	float	scale;

	scale = 1 - v[2]/-320.0;
	scale = 1 / scale;
//	scale = -320 / ( -320 - v[2] );
	v[0] = v[0]*scale + (float)r_width_2;
	v[1] = v[1]*scale + (float)r_height_2;
	v[2] = scale;
	v[3] *= scale;
	v[4] *= scale;
}

void SecureShutDown( int sig ) {
	
	printf("\nSecureShutDown: shutting down services ...\n");
	if ( sig == SIGSEGV ) {
		printf(" Cause: SIGSEGV.\n");
		printf(" Press <RETURN>\n");
//		X_Flush();
		getchar();
	}

	VID_ShutDown();
	UI_ShutDown();
//	X_ShutDown();
//	VID_ShutDown();

	exit(0);
}

void MyShockHandler()                                                           
{                                                                               
        __named_message( "\n" );                                                
        SecureShutDown( 0 );                                                    
} 


#define	SUBDIV		( 16 )

	static float		u, us;
	static float		v, vs;
	static float		z, zs;
	static float		scale;

	static float		zs16, us16, vs16;

	static int		sxFix, syFix;
	static int		txFix, tyFix;

void R_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{
	int		i;
	int		x;
	int		xpre, xpost;
	int		width;
	char		*scr;
/*
	static float		u, us;
	static float		v, vs;
	static float		z, zs;
	static float		scale;

	static float		zs16, us16, vs16;

	static int		sxFix, syFix;
	static int		txFix, tyFix;
*/
	volatile int	dummy;

	scr = ((char*) VID_GetVirtualPage() -> data) + y*r_width + xleft;

	width = xright - xleft;
	
	zs = ( zright - zleft ) / width;

	us = ( uright - uleft ) / width;
	vs = ( vright - vleft ) / width;

//	xpre = ( xleft + 16 ) & ( ~0xf );
//	xpost = xright & ( ~0xf );

// ---

	z = zleft;
	u = uleft;
	v = vleft;

	zs16 = zs * SUBDIV;
	us16 = us * SUBDIV;
	vs16 = vs * SUBDIV;
		
	scale = 1.0 / z;
	txFix = (int) (u * scale * 0x10000);
	tyFix = (int) (v * scale * 0x10000);

	z+=zs16;
	scale = 0x10000/z;

	for ( x = xleft; x < xright-16; x+=SUBDIV ) {

		u+=us16;
		v+=vs16;
//		z+=zs16;

//		scale = 1.0 / z;
		sxFix = (((int) (u * scale /* *0x10000*/ )) - txFix) / SUBDIV;
		syFix = (((int) (v * scale /* *0x10000*/ )) - tyFix) / SUBDIV;

		z+=zs16;
		scale = 0x10000 / z;

__asm__ __volatile__( "movl $0, %%eax"                                          
                      :                                                         
                      :                                                         
                      : "eax", "ebx", "ecx", "edx", "edi", "esi" );             


		for ( i = 0; i < SUBDIV; i++ ) {
			(*scr) = (* ((char*)t_data + ((txFix>>16)&255) + (((tyFix>>16)&255)*256) ));
			scr++;
			txFix+=sxFix;
			tyFix+=syFix;
		}	       
	}	
}

void R_NO_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	char		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	int		txFix, tyFix;

	scr = ((char*) VID_GetVirtualPage() -> data) + y*r_width + xleft;

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {
		scale = 1.0 / z;

		txFix = (int) (tx * scale );
		tyFix = (int) (ty * scale );
		(*scr) = (* ((char*)t_data + (txFix&255) + ((tyFix&255)*256) ));
		scr++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}

void R_FLAT_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{
	
	int		x;
	char		*scr;

	scr = ((char*) VID_GetVirtualPage() -> data) + y*r_width + xleft;


	for ( x = xleft; x < xright; x++ ) {
		(*scr) = 8;
		scr++;
	}	
}

void R_ORTHO_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		       int xright, float zright, float uright, float vright )
{
	
	int		x;
	char		*scr;
	
	scr = ((char*) VID_GetVirtualPage() -> data) + y*r_width + xleft;
	
	
	for ( x = xleft; x < xright; x++ ) {
		(*scr) = (* ((char*)t_data + (x&255) + ((y&255)*256) ));
			  scr++;
			  }	
	}


void R_LIN_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		     int xright, float zright, float uright, float vright )
{
	
	int		x;
	char		*scr;

	int		width;

	int		txFix, tyFix;
	int		sxFix, syFix;

	float		tx, ty;
	float		sx, sy;

	float		z, scale;

	volatile int	dummy;

	width = xright - xleft;
	scale = 1.0 / width;
	zleft = 1.0 / zleft;
	zright = 1.0 / zright;
		
	scr = ((char*) VID_GetVirtualPage() -> data) + y*r_width + xleft;

	tx = uleft * zleft;
	ty = vleft * zleft;

	sx = ( uright * zright - tx ) * scale;
	sy = ( vright * zright - ty ) * scale;

	txFix = (int) ( tx * 0x10000 );
	tyFix = (int) ( ty * 0x10000 );

	sxFix = (int) ( sx * 0x10000 );
	syFix = (int) ( sy * 0x10000 );

//		dummy+= sxFix;
//		dummy+= syFix;
//		dummy+= txFix;
//		dummy+= tyFix;

	for ( x = xleft; x < xright; x++ ) {
		(*scr) = (* ((char*)t_data + ((txFix>>16)&255) + (((tyFix>>16)&255)*256) ));
		scr++;
		txFix+=sxFix;
		tyFix+=syFix;
	}	
}

void R_RasterizeTexture()
{
	int		i;
	int		y;
	int		top, bot;
	int		ileft, iright;

	int		sy[8];
	float		height;

	float		xleft, xleftstep;
	float		xright, xrightstep;
	float		zleft, zleftstep;
	float		zright, zrightstep;

	float		u[2], ustep[2];
	float		v[2], vstep[2];
	
	top = MAXINT; // values.h
	bot = MININT;
	ileft = 0;

//	__named_message("\n"); //"top = %d, bot = %d\n", top, bot );
//	printf(" t_pointnum = %d\n", t_pointnum );
	for ( i = 0; i < t_pointnum; i++ ) {

		sy[i] = (int) t_points[i][1];
//		printf(" y = %d\n", y );

		if ( sy[i] < top ) {
			top = sy[i];
			ileft = i;
		}

		if ( sy[i] > bot )
			bot = sy[i];
	}
//	printf(" top = %d, ileft = %d, bot = %d \n", top, ileft, bot );

	iright = ileft;

	xleft = t_points[ileft][0];
	xright = t_points[iright][0];

	xleftstep = 0;
	xrightstep = 0;

	zleft = t_points[ileft][2];
	zright = t_points[iright][2];

	zleftstep = 0;
	zrightstep = 0;

	u[0] = t_points[ileft][3];
	v[0] = t_points[ileft][4];

	u[1] = t_points[iright][3];
	v[1] = t_points[iright][4];

	ustep[0] = 0;
	ustep[1] = 0;
	vstep[0] = 0;
	vstep[1] = 0;

	y = top;
	while( y < bot ) {
		
		if ( y >= sy[ileft] ) {
//			printf(" ileft = %d\n", ileft );
			do {
				xleft = t_points[ileft][0];
				zleft = t_points[ileft][2];
				u[0] = t_points[ileft][3];
				v[0] = t_points[ileft][4];
				ileft--;
				if ( ileft == -1 )
					ileft = t_pointnum - 1;
			} while( sy[ileft] <= y );
			height = sy[ileft] - y;
			xleftstep = ( t_points[ileft][0] - xleft ) / height;
			zleftstep = ( t_points[ileft][2] - zleft ) / height;
			ustep[0] = ( t_points[ileft][3] - u[0] ) / height;
			vstep[0] = ( t_points[ileft][4] - v[0] ) / height;
		}
			
		if ( y >= sy[iright] ) {
//			printf(" iright = %d\n", iright );
			do {
				xright = t_points[iright][0];
				zright = t_points[iright][2];
				u[1] = t_points[iright][3];
				v[1] = t_points[iright][4];
				iright++;
				if ( iright == t_pointnum )
					iright = 0;
			} while( sy[iright] <= y );
			height = sy[iright] - y;
			xrightstep = ( t_points[iright][0] - xright ) / height;
			zrightstep = ( t_points[iright][2] - zright ) / height;
			ustep[1] = ( t_points[iright][3] - u[1] ) / height;
			vstep[1] = ( t_points[iright][4] - v[1] ) / height;
		}

		R_DrawSpan( y, (int)xleft, zleft, u[0], v[0], (int)xright, zright, u[1], v[1] );

		xleft += xleftstep;
		xright += xrightstep;
		zleft += zleftstep;
		zright += zrightstep;
		y++;

		u[0] += ustep[0];
		v[0] += vstep[0];
		u[1] += ustep[1];
		v[1] += vstep[1];			
	}
}

void R_BaseFrustum()
{

	vec3d_t		v0, v1, v2, v3, v4;

	plane_t		p0, p1, p2, p3, p4;

	Vec3dInit( v0, -310, -230, 0 );
	Vec3dInit( v1,  310, -230, 0 );
	Vec3dInit( v2,  310,  230, 0 );
	Vec3dInit( v3, -310,  230, 0 );
	Vec3dInit( v4,    0,    0, -320 );

	Vec3dInitPlane( &r_frustum[0], v0, v3, v1 ); // z
	Vec3dInitPlane( &r_frustum[1], v4, v0, v3 ); // left
	Vec3dInitPlane( &r_frustum[2], v4, v2, v1 ); // right
	Vec3dInitPlane( &r_frustum[3], v4, v1, v0 ); // top
	Vec3dInitPlane( &r_frustum[4], v4, v3, v2 ); // bot

/*
	Vec3dPrint( r_frustum[0].norm );
	Vec3dPrint( r_frustum[1].norm );
	Vec3dPrint( p2.norm );
	Vec3dPrint( p3.norm );
	Vec3dPrint( p4.norm );
*/
//	getchar();
}

#define side_on		( 0 )
#define side_front	( 1 )
#define side_back	( 2 )

void R_FrustumClip( int frustum )
{
	int		pointnum;
	vec5d_t		points[8];
	float		dist[8];
	int		side[8];

	float		d;
	float		m;

	int		i, i0, i1;
	float		*v0, *v1;
	vec5d_t		vclip;
	
	for ( i = 0; i < t_pointnum; i++ ) {
		
		d = Vec3dDotProduct( t_points[i], r_frustum[frustum].norm ) - r_frustum[frustum].dist;
		dist[i] = d;
		
		if ( d > 0.001 ) {
			side[i] = side_front;
//			printf(" clip!\n");
		}
		else if ( d < -0.001 )
			side[i] = side_back;
		else
			side[i] = side_on;
	}
	
	pointnum = 0;
	i0 = t_pointnum - 1;
	for ( i1 = 0; i1 < t_pointnum; i0 = i1++ ) {
		
		v0 = t_points[i0];
		
		if ( side[i0] == side_on ) {
			Vec5dCopy( points[pointnum++], v0 );
			continue;
		}
		if ( side[i0] == side_back ) 
			Vec5dCopy( points[pointnum++], v0 );
		
		if ( side[i1] == side_on || side[i1] == side[i0] )
			continue;
		
		v1 = t_points[i1];
		
		m = dist[i0] / ( dist[i0] - dist[i1] );
		for ( i = 0; i < 5; i++ )
			vclip[i] = v0[i] + m*(v1[i] - v0[i]);
		
		Vec5dCopy( points[pointnum++], vclip );

	}
	
	for ( i = 0; i < pointnum; i++ )
		Vec5dCopy( t_points[i], points[i] );
	
	t_pointnum = pointnum;
}


#define M	( -512 )
#define P	( 512 )
#define T	( 256 )

int main()
{	
	int		i;
	int		face;

	float		di = PI / 180.0;
	float		alpha = 90-0.0;
	float		beta = 180-0.0;
	float		gamma = 180-0.0;

//	float		beta = 0.0;
//	float		gamma = 0.0;

	FILE		*handle;
	
	vec5d_t		cube[6][4];
	unsigned char	*texdata[6][256][256];

	timeval_t	timestart;
	timeval_t	timenow;
	usec_t		usec;

	// left
	Vec5dInit( cube[0][0], M, P, M, 0, 0 );
	Vec5dInit( cube[0][1], M, P, P, T, 0 );
	Vec5dInit( cube[0][2], M, M, P, T, T );
	Vec5dInit( cube[0][3], M, M, M, 0, T );

	// right
	Vec5dInit( cube[1][0], P, P, P, 0, 0 );
	Vec5dInit( cube[1][1], P, P, M, T, 0 );
	Vec5dInit( cube[1][2], P, M, M, T, T );
	Vec5dInit( cube[1][3], P, M, P, 0, T );

	// front
	Vec5dInit( cube[2][0], M, P, P, 0, 0 );
	Vec5dInit( cube[2][1], P, P, P, T, 0 );
	Vec5dInit( cube[2][2], P, M, P, T, T );
	Vec5dInit( cube[2][3], M, M, P, 0, T );

	// back
	Vec5dInit( cube[3][0], P, P, M, 0, 0 );
	Vec5dInit( cube[3][1], M, P, M, T, 0 );
	Vec5dInit( cube[3][2], M, M, M, T, T );
	Vec5dInit( cube[3][3], P, M, M, 0, T );

	// top
	Vec5dInit( cube[4][0], M, P, M, 0, 0 );
	Vec5dInit( cube[4][1], P, P, M, T, 0 );
	Vec5dInit( cube[4][2], P, P, P, T, T );
	Vec5dInit( cube[4][3], M, P, P, 0, T );

	// bot
	Vec5dInit( cube[5][0], M, M, P, 0, 0 );
	Vec5dInit( cube[5][1], P, M, P, T, 0 );
	Vec5dInit( cube[5][2], P, M, M, T, T );
	Vec5dInit( cube[5][3], M, M, M, 0, T );


//	Vec5dInit( t_points[0], -128, -128, 0, 0, 0 );
//	Vec5dInit( t_points[1], 128, -128, 0, 0, 128 );
//	Vec5dInit( t_points[2], 128, 128, 0, 128, 128 );
//	Vec5dInit( t_points[3], -128, 128, 0, 128, 0 );
//	t_pointnum = 4;

	// Signal handler setzen 	
	signal( SIGSEGV, (void(*) ()) SecureShutDown );
	signal( SIGABRT, (void(*) ()) SecureShutDown );
	signal( SIGTERM, (void(*) ()) SecureShutDown );
	signal( SIGQUIT, (void(*) ()) SecureShutDown );
	signal( SIGINT,  (void(*) ()) SecureShutDown );

	SOS_SetShockHandler( MyShockHandler ); 

	UI_StartUp( "ui_svgalib.so" );
	VID_StartUp();
	VID_SetModeByName( 640, 480, 8 );    
	handle = fopen( "quake21.pal", "rb" );
	__chkptr( handle );
	VID_LoadPal( handle );
	fclose( handle );   
	r_width = 640;
	r_width_2 = r_width / 2;
	r_height = 480;
	r_height_2 = r_height / 2;

	// left
	handle = fopen( "sky2_left.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[0], 256*256, 1, handle );
	fclose( handle );

	// right
	handle = fopen( "sky2_right.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[1], 256*256, 1, handle );
	fclose( handle );

	// front
	handle = fopen( "sky2_front.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[2], 256*256, 1, handle );
	fclose( handle );

	// back
	handle = fopen( "sky2_back.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[3], 256*256, 1, handle );
	fclose( handle );

	// top
	handle = fopen( "sky2_top.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[4], 256*256, 1, handle );
	fclose( handle );

	// bot
	handle = fopen( "sky2_bottom.arr", "rb" );
	__chkptr( handle )
	fseek( handle, 0x2c+4 , SEEK_SET );
	fread( texdata[5], 256*256, 1, handle );
	fclose( handle );

	R_BaseFrustum();

//	for ( alpha = 0; alpha < 90; alpha+=5 ) {

	GetTimeval( &timestart );
	for ( beta = 0; beta < 360; beta+=1 ) {
		alpha = gamma = beta;
//		alpha = 0;
//		gamma = 180;

		Vec3dInit( r_origin, 0, 0, 0 );
		R_SetMatrix( alpha * di, beta * di, gamma * di );
		
		for ( face = 0; face < 6; face++ ) {
//			printf("\nface %d:\n", face );
			for ( i = 0; i < 4; i++ )
				Vec5dCopy( t_points[i], cube[face][i] );
			t_pointnum = 4;
			t_data = texdata[face];

			for ( i = 0; i < t_pointnum; i++ ) {
				R_Vec5dRot( t_points[i] );
			}
			
			R_FrustumClip( 0 );
			R_FrustumClip( 1 );
			R_FrustumClip( 2 );
			R_FrustumClip( 3 );
			R_FrustumClip( 4 );
			
			for ( i = 0; i < t_pointnum; i++ ) {
				R_Vec5dPer( t_points[i] );
			}
			R_RasterizeTexture(); 
		}
		VID_Refresh();
//		usleep(10000);
	}
	GetTimeval( &timenow );
	usec = DiffTimeval( &timenow, &timestart );
	fprintf( stderr, " sec = %f, frames per second: %f \n", usec/1000000.0, 360/(usec/1000000.0) );

	getchar();
	
	VID_ShutDown();
	UI_ShutDown();
}
