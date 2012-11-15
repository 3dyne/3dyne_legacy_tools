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



// r_sky.c

#ifndef __r_sky
#define __r_sky

#include "render.h"
#include "tga.h"

extern matrix3_t	sky_matrix;

void CalcSkyVertex( vec4d_t out, vec3d_t in )
{
        vec3d_t         tmp; 
//	Vec3dScale( tmp, 1.0/16.0, r_origin );
//        Vec3dSub( tmp, in, tmp /*r_origin*/ );
        Matrix3Vec3dRotate( tmp, in, sky_matrix );
        out[0] = tmp[0];
        out[1] = tmp[1]*1.33;
        out[3] = 1.0 + tmp[2] / 1.0;
        out[2] = 1.0; // !!!	
}

void R_RenderSkyBox( void )
{
	FILE	*h;
	tga_t	*sky;
	int	i, j;
	unsigned char	skydata[6][256*256*3];

	static int skyobjs[6] = { -1, -1, -1, -1, -1, -1 };

	static vec5d_t		skydef[6*4] =
	{
		// 
		// left side
		//
		{ -1.0,  1.0, -1.0, 0.0, 0.0 },
		{ -1.0,  1.0,  1.0, 1.0, 0.0 },
		{ -1.0, -1.0,  1.0, 1.0, 1.0 },
		{ -1.0, -1.0, -1.0, 0.0, 1.0 },
		
		
		//
		// front side
		//
		{ -1.0,  1.0,  1.0, 0.0, 0.0 },
		{  1.0,  1.0,  1.0, 1.0, 0.0 },
		{  1.0, -1.0,  1.0, 1.0, 1.0 },
		{ -1.0, -1.0,  1.0, 0.0, 1.0 },
		
		
		//
		// right side
		//
		{  1.0,  1.0,  1.0, 0.0, 0.0 },
		{  1.0,  1.0, -1.0, 1.0, 0.0 },
		{  1.0, -1.0, -1.0, 1.0, 1.0 },
		{  1.0, -1.0,  1.0, 0.0, 1.0 },
		
		
		//
		// back side
		//
		{  1.0,  1.0, -1.0, 0.0, 0.0 },
		{ -1.0,  1.0, -1.0, 1.0, 0.0 },
		{ -1.0, -1.0, -1.0, 1.0, 1.0 },
		{  1.0, -1.0, -1.0, 0.0, 1.0 },
		
		//
		// top side
		//
//		{ -1.0,  1.0, -1.0, 0.0, 0.0 },
//		{  1.0,  1.0, -1.0, 1.0, 0.0 },
//		{  1.0,  1.0,  1.0, 1.0, 1.0 },
//		{ -1.0,  1.0,  1.0, 0.0, 1.0 },
		{ -1.0,  1.0, -1.0, 0.0, 1.0 },
		{  1.0,  1.0, -1.0, 0.0, 0.0 },
		{  1.0,  1.0,  1.0, 1.0, 0.0 },
		{ -1.0,  1.0,  1.0, 1.0, 1.0 },
		
		//
		// bottom side
		//
//		{ -1.0, -1.0,  1.0, 0.0, 0.0 },
//		{  1.0, -1.0,  1.0, 1.0, 0.0 },
//		{  1.0, -1.0, -1.0, 1.0, 1.0 },
//		{ -1.0, -1.0, -1.0, 0.0, 1.0 }	

		{ -1.0, -1.0,  1.0, 1.0, 0.0 },
		{  1.0, -1.0,  1.0, 1.0, 1.0 },
		{  1.0, -1.0, -1.0, 0.0, 1.0 },
		{ -1.0, -1.0, -1.0, 0.0, 0.0 }	
	};


	//
	// init
	//
	

	if ( skyobjs[0] == -1 )
	{
		printf( "R_RenderSkyBox: load textures ...\n" );

//		h = fopen( ART_PATH "/sky/sky4/rgb_2.tga", "r" );
		h = fopen( ART_PATH "/sky/shore.tga", "r" );
		if ( !h )
			Error( "can't open sky.\n" );
		sky = TGA_Read( h );
		fclose( h );

		for ( i = 0; i < 6; i++ )
		{			
			for ( j = 0; j < 256*256; j++ )
			{
				skydata[i][j*3] = sky->image.red[j+256*256*i];
				skydata[i][j*3+1] = sky->image.green[j+256*256*i];
				skydata[i][j*3+2] = sky->image.blue[j+256*256*i];
			}
			
			glGenTextures( 1, &skyobjs[i] );
			glBindTexture( GL_TEXTURE_2D, skyobjs[i] );
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP);
			
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 256, 256, 
				     0, GL_RGB, GL_UNSIGNED_BYTE, &skydata[i][0] );
		}		
		TGA_Free( sky );
	}


	//
	// draw
	// 
	glEnable( GL_TEXTURE_2D );
	glDisable(GL_BLEND);
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );
	for ( i = 0; i < 6; i++ )
	{
		glBindTexture( GL_TEXTURE_2D, skyobjs[i] );
		glBegin( GL_TRIANGLE_FAN );
		glColor3f( 1.0, 1.0, 1.0 );
		for ( j = 0; j < 4; j++ )
		{
			vec4d_t		w;
			vec3d_t		v;
			Vec3dScale( v, 1024.0, &skydef[i*4+j][0] );
			CalcSkyVertex( w, v );
			glTexCoord2f( skydef[i*4+j][3], skydef[i*4+j][4] );
//			glVertex3f( w[0]/w[3], w[1]/w[3], 0.0 );
			glVertex4fv( w );
//			glVertex3f( skydef[i*4+j][0]*2.0, skydef[i*4+j][1]*2.0, skydef[i*4+j][2]*2.0 );
		}
		glEnd();
	}
	
}

#endif
