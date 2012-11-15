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



// r_csurface.c

#include "r_csurface.h"
#include "render.h"

/*
  ==============================
  CompileCSurfaceClass

  ==============================
*/

#define MAX_CSURFACEDEFS	( 1024 )

int		r_csurfacedefnum;
csurfacedef_t	r_csurfacedefs[MAX_CSURFACEDEFS];

int FindIndexForTextureIdent( char *ident )
{
	int		i;
	hpair_t		*pair;

	for ( i = 0; i < r_texturenum; i++ )
	{
		pair = FindHPair( r_textures[i].self, "ident" );
		if ( !pair )
			Error( "missing 'ident'\n" );
		if ( !strcmp( pair->value, ident ) )
			return i;
	}

	Error( "can't find texture ident '%s'\n", ident );
}

void CompileCSurfaceClass( hmanager_t *csurfacehm, hmanager_t *texturehm )
{
	hobj_search_iterator_t	iter;
	hobj_t			*csurf;
	hpair_t			*pair;
	char			tt[256];

	r_csurfacedefnum = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( csurfacehm ), "csurface" );
	for ( ; ( csurf = SearchGetNextClass( &iter ) ) ; )
	{
		int		texture;
		int		upointnum;
		int		vpointnum;
		int		u, v;
		vec2d_t		shift, scale;
		vec2d_t		vec0, vec1, vec2;
		vec2d_t		tmp;
		surface_ctrl_t	*ctrl;
		surface_ctrl_t	*tctrl;
		int		width, height;

		if ( r_csurfacedefnum == MAX_CSURFACEDEFS )
			Error( "reached MAX_CSURFACEDEFS\n" );

		pair = FindHPair( csurf, "ident" );
		if ( !pair )
			Error( "missing 'ident'\n" );
		texture = FindIndexForTextureIdent( pair->value );

		// upointnum
		pair = FindHPair( csurf, "upointnum" );
		if ( !pair )
			Error( "missing 'upointnum'\n" );
		HPairCastToInt_safe( &upointnum, pair );

		// vpointnum
		pair = FindHPair( csurf, "vpointnum" );
		if ( !pair )
			Error( "missing 'vpointnum'\n" );
		HPairCastToInt_safe( &vpointnum, pair );
		
		ctrl = NewBezierSurface( upointnum, vpointnum );		
		
		// ctrlpoints
		for ( u = 0; u < upointnum; u++ )
		{
			for ( v = 0; v < vpointnum; v++ )
			{
				vec3d_t		point;

				sprintf( tt, "u%d_v%d", u, v );
				pair = FindHPair( csurf, tt );
				if ( !pair )
					Error( "missing control point '%s' in curved surface '%s'\n", tt, csurf->name );
				HPairCastToVec3d_safe( point, pair );

				SetSurfaceCtrlPoint( ctrl, u, v, point );				
			}
		}

		//
		// tctrl ctrlpoints
		//
		
		// shift
		pair = FindHPair( csurf, "shift" );
		if ( !pair )
			Error( "missing 'shift'\n" );
		HPairCastToVec2d_safe( shift, pair );

		// vec0
		pair = FindHPair( csurf, "vec0" );
		if ( !pair )
			Error( "missing 'vec0'\n" );
		HPairCastToVec2d_safe( vec0, pair );

		// vec1
		pair = FindHPair( csurf, "vec1" );
		if ( !pair )
			Error( "missing 'vec1'\n" );
		HPairCastToVec2d_safe( vec1, pair );

		// vec2
		pair = FindHPair( csurf, "vec2" );
		if ( !pair )
			Error( "missing 'vec2'\n" );
		HPairCastToVec2d_safe( vec2, pair );

		// scale
		pair = FindHPair( csurf, "scale" );
		if ( !pair )
			Error( "missind 'scale'\n" );
		HPairCastToVec2d_safe( scale, pair );

		// width
		pair = FindHPair( csurf, "width" );
		if ( !pair )
			Error( "missing 'width'\n" );
		HPairCastToInt_safe( &width, pair );

		// height
		pair = FindHPair( csurf, "height" );
		if ( !pair )
			Error( "missing 'height'\n" );
		HPairCastToInt_safe( &height, pair );

		tctrl = NewBezierSurface( 2, 2 );
		SetSurfaceCtrlPoint3f( tctrl, 0, 0, shift[0], shift[1], 0 );
		tmp[0] = shift[0] + vec0[0]*scale[0];
		tmp[1] = shift[1] + vec0[1]*scale[1];
		SetSurfaceCtrlPoint3f( tctrl, 1, 0, tmp[0], tmp[1], 0 );
		tmp[0] = shift[0] + vec1[0]*scale[0];
		tmp[1] = shift[1] + vec1[1]*scale[1];
		SetSurfaceCtrlPoint3f( tctrl, 1, 1, tmp[0], tmp[1], 0 );
		tmp[0] = shift[0] + vec2[0]*scale[0];
		tmp[1] = shift[1] + vec2[1]*scale[1];
		SetSurfaceCtrlPoint3f( tctrl, 0, 1, tmp[0], tmp[1], 0 );
		
		r_csurfacedefs[r_csurfacedefnum].self = csurf;
		r_csurfacedefs[r_csurfacedefnum].width = width;
		r_csurfacedefs[r_csurfacedefnum].height = height;

		r_csurfacedefs[r_csurfacedefnum].ctrl = ctrl;
		r_csurfacedefs[r_csurfacedefnum].tctrl = tctrl;
		r_csurfacedefs[r_csurfacedefnum].texture = texture;
		R_OptimizeCSurfaceCtrl( &r_csurfacedefs[r_csurfacedefnum] );

		r_csurfacedefnum++;		
	}

	printf( " %d curved surfaces\n", r_csurfacedefnum );
}


/*
  ==============================
  R_OptimizeCSurfaceCtrl

  ignores all csurface with 
  orders != 3
  ==============================
*/
void R_OptimizeCSurfaceCtrl( csurfacedef_t *csurf )
{
	int		u, v;
	bool_t		u_remove;
	bool_t		v_remove;
	surface_ctrl_t	*ctrl;
	
	ctrl = csurf->ctrl;

	if ( ctrl->uorder != 3 || ctrl->vorder != 3 )
		return;

	u_remove = false;
	v_remove = false;

	// can drop uorder from 3 to 2 ?
	for ( v = 0; v < 3; v++ )
	{
		vec3d_t		p1, p2;
		vec3d_t		t;

		GetSurfaceCtrlPoint( ctrl, 0, v, p1 );
		GetSurfaceCtrlPoint( ctrl, 1, v, t );
		GetSurfaceCtrlPoint( ctrl, 2, v, p2 );

		if ( !Vec3dCheckColinear( p1, p2, t ) )
			break;
	}
	
	if ( v == 3 )
		v_remove = true;

	// can drop vorder from 3 to 2 ?
	for ( u = 0; u < 3; u++ )
	{
		vec3d_t		p1, p2;
		vec3d_t		t;

		GetSurfaceCtrlPoint( ctrl, u, 0, p1 );
		GetSurfaceCtrlPoint( ctrl, u, 1, t );
		GetSurfaceCtrlPoint( ctrl, u, 2, p2 );

		if ( !Vec3dCheckColinear( p1, p2, t ) )
			break;
	}
	
	if ( u == 3 )
		u_remove = true;


#if 1
	
	if ( u_remove )
	{
		csurf->upointnum = 2;
	}
	else
	{
		csurf->upointnum = 3;
	}

	if ( v_remove )
	{
		csurf->vpointnum = 2;
	}
	else
	{
		csurf->vpointnum = 3;
	}

	
#else
	if ( u_remove && v_remove )
	{
		surface_ctrl_t		*ctrl2;
		vec3d_t			tmp;

		printf( "uv\n" );
		
		ctrl2 = NewBezierSurface( 2, 2 );

		GetSurfaceCtrlPoint( ctrl, 0, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 1, tmp );

		GetSurfaceCtrlPoint( ctrl, 0, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 1, tmp );

		FreeBezierSurface( ctrl );
		csurf->ctrl = ctrl2;
		return ctrl;
	}
	else if ( u_remove && !v_remove )
	{
		surface_ctrl_t		*ctrl2;
		vec3d_t			tmp;

		printf( "u\n" );
		
		ctrl2 = NewBezierSurface( 2, 3 );
		
		GetSurfaceCtrlPoint( ctrl, 0, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 0, 1, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 1, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 1, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 1, tmp );

		GetSurfaceCtrlPoint( ctrl, 0, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 2, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 2, tmp );
		
		FreeBezierSurface( ctrl );
		csurf->ctrl = ctrl2;
		return;
	}
	else if ( !u_remove && v_remove )
	{
		surface_ctrl_t		*ctrl2;
		vec3d_t			tmp;

		printf( "v\n" );
		
		ctrl2 = NewBezierSurface( 3, 2 );
		
		GetSurfaceCtrlPoint( ctrl, 0, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 0, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 0, 1, tmp );

		GetSurfaceCtrlPoint( ctrl, 1, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 1, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 1, 1, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 0, tmp );
		SetSurfaceCtrlPoint( ctrl2, 2, 0, tmp );

		GetSurfaceCtrlPoint( ctrl, 2, 2, tmp );
		SetSurfaceCtrlPoint( ctrl2, 2, 1, tmp );

		FreeBezierSurface( ctrl );		
		csurf->ctrl = ctrl2;
		return;
	}
#endif


	return;
}


void R_SetupCSurfaceLightmaps( void )
{
	int		i;

	R_LightPage_BeginRegister();
	for ( i = 0; i < r_csurfacedefnum; i++ )
	{
		r_csurfacedefs[i].box = R_LightPage_RegisterBox( r_csurfacedefs[i].width, r_csurfacedefs[i].height );
	}
	R_LightPage_EndRegister();
	R_LightPage_FillRegisteredBoxes();

	for ( i = 0; i < r_csurfacedefnum; i++ )
	{
		int		max_lightmap_size;
		hpair_t		*pair;
		unsigned short		lightmap[128*128];	

		pair = FindHPair( r_csurfacedefs[i].self, "diffuse" );
		if ( !pair )
			Error( "missing 'diffuse' in csurfacedef '%s'.\n", r_csurfacedefs[i].self->name );
				
		max_lightmap_size = 128*128*2;
		HPairCastToBstring_safe( lightmap, &max_lightmap_size, pair );
		
		LightPage_InsertSubImage( &r_lightpages[r_csurfacedefs[i].box->lightpage],
					  r_csurfacedefs[i].box->xofs,
					  r_csurfacedefs[i].box->yofs,
					  r_csurfacedefs[i].width,
					  r_csurfacedefs[i].height,
					  lightmap );
	}


}


void R_CSurfaceSetLOD( int pointnum )
{
	int		i;

	if ( pointnum < 3 )
		pointnum = 3;

	for ( i = 0; i < r_csurfacedefnum; i++ )
	{
		if ( r_csurfacedefs[i].upointnum >= 3 )
		{
			r_csurfacedefs[i].upointnum = pointnum;
		}

		if ( r_csurfacedefs[i].vpointnum >= 3 )
		{
			r_csurfacedefs[i].vpointnum = pointnum;
		}		
			
	}
}
