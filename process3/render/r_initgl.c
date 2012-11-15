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



// r_initgl.c

#include "render.h"

gl_extensions_t		gl_ext;	// available gl extensions

void R_InitGL_Extensions( void )
{
	const char	*exten;

	memset( &gl_ext, 0, sizeof( gl_extensions_t ) );

	exten = ( const char * ) glGetString( GL_EXTENSIONS );

	printf( "R_InitGL_Extensions: %s\n", exten );
	
	if ( strstr( exten, "GL_ARB_multitexture" ) )
	{
		printf( " available: GL_ARB_multitexture\n" );
		gl_ext.have_arb_multitexture = true;
	}

	gl_ext.have_arb_multitexture = false;

	if ( strstr( exten, "GL_EXT_vertex_array" ) )
	{
		printf( " available: GL_EXT_vertex_array\n" );
		gl_ext.have_ext_va = true;
	}

	if ( strstr( exten, "GL_EXT_compiled_vertex_array" ) )
	{
		printf( " available: GL_EXT_compiled_vertex_array\n" );
		gl_ext.have_ext_cva = true;
	}

}

void R_InitGL_Version( void )
{
	printf( "R_InitGL_Version: %s\n", (char *) glGetString(GL_VERSION));	
}

void R_InitGL( void )
{
	printf( "R_InitGL:\n" );

	glEnable(GL_CULL_FACE);
	glFrontFace( GL_CW );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );
	
//	glDisable( GL_DITHER );

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);	
	glDepthFunc( GL_GEQUAL );
	glDepthRange( 0.0, 1.0 );	

	glViewport( 0, 0, (GLint)SIZE_X, (GLint)SIZE_Y );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
       	glLoadIdentity();	

	R_InitGL_Version();
	R_InitGL_Extensions();
}
