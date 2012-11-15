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



// r_model.c

#include "render.h"
#include "r_model.h"
#include "r_videoscape.h"
#include "g_map3cache.h"

#define MAX_MODEL_VECS		( 1024 )
//static r_model_t		*current;
static vsc_model_t		*current;

int				vecnum;
static vec3d_t			vecs[MAX_MODEL_VECS];
static vec3d_t			cols[MAX_MODEL_VECS];
static vec3d_t			cols2[MAX_MODEL_VECS];

/*
  ==============================
  R_NewModel

  ==============================
*/
r_model_t * R_NewModel( int trinum )
{
	r_model_t	*model;
	int		size;

	size = (int)&(((r_model_t *)0)->vecs[trinum*3]);
	model = (r_model_t *) NEWBYTES( size );

	model->trinum = trinum;
	return model;
}



/*
  ==============================
  R_FreeModel

  ==============================
*/
void R_FreeModel( r_model_t *model )
{
	FREE( model );
}



/*
  ==============================
  R_BeginModel

  ==============================
*/
void R_BeginModel( vsc_model_t *model )
{
	current = model;
	vecnum = 0;
}



/*
  ==============================
  R_EndModel

  ==============================
*/
void R_EndModel( void )
{

}



/*
  ==============================
  R_InitModel

  ==============================
*/
void R_InitModel( void )
{
	int		i;
	
	for ( i = 0; i < current->pointnum; i++ )
	{
		Vec3dScale( vecs[i], 1.0/1.0, current->p[i] );
	}
//	vecnum = current->trinum*3;
}



/*
  ==============================
  R_RotateModel

  ==============================
*/
void R_RotateModel( fp_t roll, fp_t pitch, fp_t yaw )
{
	int		i;
	matrix3_t	matrix;

	Matrix3SetupRotate( matrix, roll*DEG2RAD, pitch*DEG2RAD, yaw*DEG2RAD );
	
	for ( i = 0; i < current->pointnum; i++ )
	{
		Matrix3TransformPoint( vecs[i], vecs[i], matrix );
	}
}


/*
  ==============================
  R_TranslateModel

  ==============================
*/
void R_TranslateModel( vec3d_t origin )
{
	int		i;
	vec3d_t		tmp;
	Vec3dScale( tmp, 1.0/1.0, origin );

	for ( i = 0; i < current->pointnum; i++ )
	{
		Vec3dAdd( vecs[i], vecs[i], tmp );
	}
}

/*
  ==============================
  R_ColorModel

  ==============================
*/
void R_ColorModel( map3_cache_t *cache )
{
	int		i;

	if ( cache )
	{
		ivec3d_t	upos;	
//		vec3d_t		pos;

		for ( i = 0; i < current->pointnum; i++ )
		{
			void		*addr;
			fp_t		*color;
			vec3d_t		delta;

			IVec3dRint( upos, vecs[i] );
			IVec3dUnitSnap( upos, upos, 16 );	
			
			addr = G_AccessMap3Cache( cache, upos );
			color = (fp_t *) addr;
			if ( !color )
			{
				Vec3dInit( cols2[i], 1, 1, 1 );
			}
			else
			{
				Vec3dCopy( cols2[i], color );
			}

			Vec3dSub( delta, cols2[i], cols[i] );
			Vec3dMA( cols[i], 0.1, delta, cols[i] );
		}
	}
}

/*
  ==============================
  R_ModelToWorld

  ==============================
*/
void R_ModelToWorld( void )
{
	int		i;

	for ( i = 0; i < current->pointnum; i++ )
	{
		vec3d_t		tmp;
		Vec3dScale( vecs[i], 1.0/16.0, vecs[i] );
		Vec3dScale( tmp, 1.0/16.0, r_origin );
		Vec3dSub( tmp, vecs[i], tmp /*r_origin*/ );
		Matrix3Vec3dRotate( tmp, tmp, r_matrix );

		vecs[i][0] = tmp[0];
		vecs[i][1] = tmp[1]*1.33;
		vecs[i][2] = 1.0+tmp[2]/1.0;
	}
}


/*
  ==============================
  R_DrawModel

  ==============================
*/
extern map3_cache_t		*r_volume;	// hack

void R_DrawModel( void )
{
	int		i;
	vsc_face_t	*f;


	glDisable( GL_TEXTURE_2D );
	glDisable( GL_BLEND );
	glColor3f( 1.0, 1.0, 1.0 );

	glBegin( GL_TRIANGLES );
	for ( f = current->faces; f ; f=f->next )
	{
		glBegin( GL_POLYGON );
		for ( i = 0; i < f->pointnum; i++ )
		{
			int		ref;
			
			ref = f->p[i];
			glColor3fv( cols[ref] );
			glVertex4f( vecs[ref][0], vecs[ref][1], 1.0, vecs[ref][2] );
		}
	}
	glEnd();
}



void R_ModelTest( vec3d_t pos, bool_t update )
{
	static vsc_model_t		*model = NULL;
	static fp_t	yaw = 0.0;
	static vec3d_t		origin = { 208+128, 64+32, -576+512 };

	if ( !model )
	{
		model = VSC_LoadModel( ART_PATH "test1.obj" );
		if ( !model )
			Error( "can't load model.\n" );		
		VSC_DumpModel( model );
	}

	if ( update )
	{
		Vec3dCopy( origin, pos );
	}
#if 1
	if ( model )
	{
//		vec3d_t		origin = { 208+128, 64+32, -576+512 };
		R_BeginModel( model );
		R_InitModel();
//		R_RotateModel( yaw, yaw, 0 );
		yaw+=1.0;
		R_TranslateModel( origin );
		R_ColorModel( r_volume );
		R_ModelToWorld();
		R_EndModel();
		R_DrawModel();		
	}
#endif
}
