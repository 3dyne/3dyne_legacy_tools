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



// r_cssetup.c

#include "r_csurface.h"
#include "render.h"

/*
  ==============================
  R_BeginCSurfaceSetup

  ==============================
*/
void R_BeginCSurfaceSetup( void )
{
	
}



/*
  ==============================
  R_EndCSurfaceSetup

  ==============================
*/
void R_EndCSurfaceSetup( void )
{
	
}



/*
  ==============================
  R_CSurfaceSetup
  
  ==============================
*/
void R_CSurfaceSetup( csurfacedef_t *csurf )
{
	surface_points_t	*mesh;
	surface_points_t	*tmesh;
	surface_ctrl_t		*lctrl;
	surface_points_t	*lmesh;
	int			u, v;
	int		i;
	texture_t	*texture;

//	printf( "csurface: %dx%d\n", csurf->upointnum, csurf->vpointnum );

	glEnable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glPolygonMode( GL_FRONT, GL_FILL );

	glColor3f( 1.0, 1.0, 1.0 );

	mesh = EvalSurfacePoints( csurf->ctrl, csurf->upointnum, csurf->vpointnum );

#if 1
	//
	// texture
	//
	tmesh = EvalSurfacePoints( csurf->tctrl, csurf->upointnum, csurf->vpointnum );

	texture = &r_textures[csurf->texture];
	glBindTexture( GL_TEXTURE_2D, texture->texobj );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_DECAL );

	for ( v = 0; v < csurf->vpointnum-1; v++ )
	{
		glBegin( GL_TRIANGLE_STRIP );
		for ( u = 0; u < csurf->upointnum; u++ )
		{			
			for ( i = 0; i < 2; i++ )
			{
				vec3d_t		point;
				vec3d_t		tpoint;
				vec4d_t		w;
				

				GetSurfacePoint( mesh, u, v+i, point );
				GetSurfacePoint( tmesh, u, v+i, tpoint );
				
				tpoint[0]*=texture->inv_width;
				tpoint[1]*=texture->inv_width;

				Vec3dScale( point, 1.0/16.0, point );
				CalcVertex( w, point );
				
				glTexCoord2f( tpoint[0], tpoint[1] );
				glVertex4fv( w );
			}
		}
		glEnd();
	}

	FreeSurfacePoints( tmesh );
#endif

	//
	// diffuse lightmap
	//
	lctrl = NewBezierSurface( 2, 2 );
	SetSurfaceCtrlPoint3f( lctrl, 0, 0, 0, 0, 0 );
	SetSurfaceCtrlPoint3f( lctrl, 1, 0, 1, 0, 0 );
	SetSurfaceCtrlPoint3f( lctrl, 1, 1, 1, 1, 0 );
	SetSurfaceCtrlPoint3f( lctrl, 0, 1, 0, 1, 0 );

	lmesh = EvalSurfacePoints( lctrl, csurf->upointnum, csurf->vpointnum );

	glBindTexture( GL_TEXTURE_2D, r_lightpages[csurf->box->lightpage].texobj );
	glTexEnvi( GL_TEXTURE_2D, GL_TEXTURE_ENV_MODE, GL_MODULATE );
	glEnable( GL_BLEND );
	glBlendFunc(GL_ZERO, GL_SRC_COLOR);

	for ( v = 0; v < csurf->vpointnum-1; v++ )
	{
		glBegin( GL_TRIANGLE_STRIP );
		for ( u = 0; u < csurf->upointnum; u++ )
		{			
			for ( i = 0; i < 2; i++ )
			{
				vec3d_t		point;
				vec3d_t		lpoint;
				vec4d_t		w;
				

				GetSurfacePoint( mesh, u, v+i, point );
				GetSurfacePoint( lmesh, u, v+i, lpoint );
//				lpoint[0]=1.0-lpoint[0];
//				lpoint[1]=1.0-lpoint[1];

				lpoint[0]*=(15.0)/128.0;
//				lpoint[0]*=(1.0/(1.0*128.0));
				lpoint[0]+=(csurf->box->xofs/128.0);
				lpoint[0]+=0.5/128.0;

				lpoint[1]*=(15.0)/128.0;
//				lpoint[1]*=(1.0/(64.0*128.0));
				lpoint[1]+=(csurf->box->yofs/128.0);
				lpoint[1]+=0.5/128.0;
//				printf( "%f %f, ", lpoint[0], lpoint[1] );
				
				Vec3dScale( point, 1.0/16.0, point );
				CalcVertex( w, point );
				
				glTexCoord2f( lpoint[0], lpoint[1] );
				glVertex4fv( w );
			}
		}
		glEnd();
	}
	FreeBezierSurface( lctrl );
	FreeSurfacePoints( lmesh );

	FreeSurfacePoints( mesh );	
}
