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



// r_misc.c

#include "render.h"

#include "tga.h" 

unsigned char* Image565ToImage888( unsigned short *in, int pixelnum )
{

	int		i;
	unsigned short		pixel565;
	unsigned char		*pixel888;
	unsigned char		*rgb888;
	
	rgb888 = (unsigned char*) malloc( pixelnum * 3 );
	pixel888 = rgb888;

	for ( i = 0; i < pixelnum; i++ )
	{
		pixel565 = *in++;

		*pixel888++ = ((pixel565>>11)&31)<<3;
		*pixel888++ = ((pixel565>>5)&63)<<2;
		*pixel888++ = ((pixel565&31))<<3;
	}

	return rgb888;
}

GLuint Misc_GenTexture_TGA_8888( char *name )
{
	FILE		*h;
	tga_t		*tga;
	int		i;
	GLuint		texobj;
	unsigned char            *tmp, *ptr;

	h = fopen( name, "r" );
	if ( !h )
		Error( "Misc_GenTexture_TGA: can't open file.\n" );
	tga = TGA_Read( h );
	TGA_Dump( tga );
	fclose( h );
	if ( !tga )
		Error( "Misc_GenTexture_TGA: tga failed.\n" );

	glGenTextures( 1, &texobj );
	glBindTexture( GL_TEXTURE_2D, texobj );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	tmp = (unsigned char *) malloc( tga->image_width*tga->image_height*4 );
	ptr = tmp;
	for ( i = 0; i < tga->image.pixels; i++ )
	{

		*ptr++ = tga->image.red[i];
		*ptr++ = tga->image.green[i];
		*ptr++ = tga->image.blue[i];
		*ptr++ = tga->image.alpha[i];
	}
	TGA_Free( tga );

	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, tga->image_width, tga->image_height, GL_RGBA, GL_UNSIGNED_BYTE, tmp );
	free( tmp );

	return texobj;
}

/*
  ==================================================
  vertex stuff

  ==================================================
*/

void CalcVertex( vec4d_t out, vec3d_t in )
{
        vec3d_t         tmp; 
	Vec3dScale( tmp, 1.0/16.0, r_origin );
        Vec3dSub( tmp, in, tmp /*r_origin*/ );
        Matrix3Vec3dRotate( tmp, tmp, r_matrix );
        out[0] = tmp[0];
        out[1] = tmp[1]*1.33;
        out[3] = 1.0 + tmp[2] / 1.0;
        out[2] = 1.0; // !!!
}

void ProjectVec3d( vec2d_t out, vec3d_t in, projectionType type )
{
	if ( ( type & ProjectionType_Mask ) == ProjectionType_X )
	{
		out[0] = in[2];
		out[1] = in[1];		
	}
	else if ( ( type & ProjectionType_Mask ) == ProjectionType_Y )
	{
		out[0] = in[0];
		out[1] = in[2];
	}
	else if ( ( type & ProjectionType_Mask ) == ProjectionType_Z )
	{
		out[0] = in[0];
		out[1] = in[1];
	}
	else
	{
		Error( "ProjectVec3d: unkown projection type.\n" );
	}
}
