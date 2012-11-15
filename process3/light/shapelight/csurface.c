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



// csurface.c

#include "light.h"

bsurface_t * NewBSurface( void )
{
	bsurface_t		*csurf;

	csurf = NEWTYPE( bsurface_t );
	return csurf;
}

void FreeCSurface( bsurface_t *csurf )
{
	FREE( csurf );
}

/*
  ==============================
  CompileCSurfaceClass

  ==============================
*/

bsurface_t * CompileBSurfaceClass_old( hmanager_t *bsurfacehm )
{
#if 0
	hobj_search_iterator_t	iter;
	hobj_t			*csurf;
	hpair_t			*pair;
	bsurface_t		*head;
	int			bsurfnum;
	char			tt[256];
	

	head = NULL;
	bsurfnum = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( bsurfacehm ), "csurface" );
	for ( ; ( csurf = SearchGetNextClass( &iter ) ) ; )
	{
		int		upointnum;
		int		vpointnum;
		int		u, v;
		vec2d_t		tmp;
		surface_ctrl_t	*ctrl;
		bsurface_t	*cs;


		cs = NewBSurface();
//		pair = FindHPair( csurf, "ident" );
//		if ( !pair )
//			Error( "missing 'ident'\n" );


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

#if 0
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
#endif		

		cs->next = head;
		head = cs;
		cs->ctrl = ctrl;		
		cs->self = csurf;
		bsurfnum++;
	}

	printf( " %d curved surfaces\n", bsurfnum );	

	return head;
#endif
}

/*
  ==============================
  BuildSurface

  ==============================
*/
bsurface_t * BuildSurface( hobj_t *csurf )
{
	int		upointnum;
	int		vpointnum;
	int		u, v;
	vec2d_t		tmp;
	surface_ctrl_t	*ctrl;
	bsurface_t	*cs;
	hpair_t		*pair;
	char		tt[256];

	cs = NewBSurface();

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
	cs->ctrl = ctrl;
	
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
	
	return cs;
}

/*
  ==============================
  BuildSurfaceLightList

  ==============================
*/
bsurface_t * BuildSurfaceLightList( hmanager_t *shapehm )
{
	bsurface_t	*list, *bs;

	hobj_t		*shape;
	hobj_t		*csurf;
	hpair_t		*pair;
	hobj_search_iterator_t	iter;

	list = NULL;

	InitClassSearchIterator( &iter, HManagerGetRootClass( shapehm ), "shape" );
	for ( ; ( shape = SearchGetNextClass( &iter ) ) ; )
	{
		pair = FindHPair( shape, "tess_name" );
		if ( !pair )
			Error( "missing key 'tess_name'\n" );
	
		if ( !strcmp( pair->value, "csurf" ) )
		{

		}
		else
		{
			continue;
		}

		// no material, no light
		if ( !FindHPair( shape, "material" ) )
			continue;	

		csurf = FindClassType( shape, "csurf" );
		if ( !csurf ) 
			Error( "missing class 'csurf' in shape '%s'\n", shape->name );

		bs = BuildSurface( csurf );

		bs->shape = shape;
		bs->self = csurf;

		bs->next = list;
		list = bs;
	}

	return list;
}


/*
  ====================
  FixBoarderNormals

  EvalSurfaceNormals doesn't
  calc the right normals on
  boarder curves.
  ====================
*/
void FixNormal( vec3d_t out, vec3d_t in )
{
	fp_t            b;
        int             i, n;
        
        b = -1.0;
        n = -1;
        for ( i = 0; i < 3; i++ )
        {
                if ( b < fabs(in[i]) )
                {
                        b = fabs(in[i]);
                        n = i;
                }
        }
        
        if ( n == -1 )
        {
                Vec3dCopy( out, in );
                return;
        }

        if ( b > 0.98 )
        {
//              printf( "fixed %f", b );

                for ( i = 0; i < 3; i++ )
                {
                        if ( i != n ) 
                                out[i] = 0.0;
                        else
                        {
                                if ( in[n] < 0.0 )
                                        out[n] = -1.0;
                                else
                                        out[n] = 1.0;
                        }
                }
//              printf( "( %f %f %f ) ( %f %f %f )\n", in[0], in[1], in[2], out[0], out[1], out[2] )
        }
        else
        {
                Vec3dCopy( out, in );
        }
}

void FixBoarderNormals( surface_points_t *sp )
{
        int     u, v;
        vec3d_t         tmp;
        for ( u = 0; u < sp->upointnum; u++ )
        {
                GetSurfacePoint( sp, u, 0, tmp );
                FixNormal( tmp, tmp );
                SetSurfacePoint( sp, u, 0, tmp );

                GetSurfacePoint( sp, u, sp->vpointnum-1, tmp );
                FixNormal( tmp, tmp );
                SetSurfacePoint( sp, u, sp->vpointnum-1, tmp );
        }

        for ( v = 0; v < sp->vpointnum; v++ )
        {
                GetSurfacePoint( sp, 0, v, tmp );
                FixNormal( tmp, tmp );
                SetSurfacePoint( sp, 0, v, tmp );

                GetSurfacePoint( sp, sp->upointnum-1, v, tmp );
                FixNormal( tmp, tmp );
                SetSurfacePoint( sp, sp->upointnum-1, v, tmp );
        }
}



/*
  ==============================
  BuildBSurfacePatches

  ==============================
*/
int	bsurf_patch_num = 0;

fp_t ApproxCurveLength( int num, vec3d_t pts[] )
{
	int		i;
	vec3d_t		v;
	fp_t		l;

	l = 0;
	for( i = 0; i < num-1; i++ )
	{
		Vec3dSub( v, pts[i], pts[i+1] );
		l += Vec3dLen( v );
	}

	return l;
}

void ApproxLightmapSizeOfBSurface( surface_ctrl_t *ctrl, int *width, int *height )
{
	int		i;
	vec3d_t		pts[3];
	fp_t		l1, l2;
	fp_t		umax, vmax;
	surface_points_t	*mesh;

	// approx width, height

	mesh = EvalSurfacePoints( ctrl, 3, 3 );

	// v=0, u=0..2
	for( i = 0; i < 3; i++ )
		GetSurfacePoint( mesh, i, 0, pts[i] );
	l1 = ApproxCurveLength( 3, pts );
	
	// v=2, u=0..2
	for( i = 0; i < 3; i++ )
		GetSurfacePoint( mesh, i, 2, pts[i] );
	l2 = ApproxCurveLength( 3, pts );
	
	if ( l1 >= l2 )
		umax = l1;
	else
		umax = l2;


	// u=0, v=0..2
	for( i = 0; i < 3; i++ )
		GetSurfacePoint( mesh, 0, i, pts[i] );
	l1 = ApproxCurveLength( 3, pts );
	
	// u=2, v=0..2
	for( i = 0; i < 3; i++ )
		GetSurfacePoint( mesh, 2, i, pts[i] );
	l2 = ApproxCurveLength( 3, pts );
	
	if ( l1 >= l2 )
		vmax = l1;
	else
		vmax = l2;

//	printf( "umax = %f, vmax = %f\n", umax/16.0, vmax/16.0 );
	FreeSurfacePoints( mesh );

	*width = (int)ceil(umax/16.0)+1;
	*height = (int)ceil(vmax/16.0)+1;

//	*width = 16;
//	*height = 8;
}

void BuildBSurfacePatches( bsurface_t *cs )
{
	int			width, height;
	surface_points_t	*mesh;
	surface_points_t	*nmesh;
	int		u, v;
	vec3d_t		pos;
	patch_t                 *patch;


	ApproxLightmapSizeOfBSurface( cs->ctrl, &width, &height );
//	printf( "width = %d, height = %d\n", width, height );
	//
	cs->patches = NULL;

	mesh = EvalSurfacePoints( cs->ctrl, width, height );
	nmesh = EvalSurfaceNormals( mesh );
	FixBoarderNormals( nmesh );
	
	Vec3dInitBB( cs->min3d, cs->max3d, 999999.9 );
	for ( u = 0; u < width; u++ )
		for ( v = 0; v < height; v++ )	
		{
			GetSurfacePoint( mesh, u, v, pos );
                        patch = NewPatch();

//                        patch->x = v;
//                        patch->y = u;
                        patch->x = u;
                        patch->y = v;

                        Vec3dCopy( patch->light_origin, pos );
                        Vec3dCopy( patch->trace_origin, pos );

                        Vec3dAddToBB( cs->min3d, cs->max3d, pos );
                        
                        GetSurfacePoint( nmesh, u, v, pos );
                        Vec3dFlip( pos, pos );
                        Vec3dCopy( patch->norm, pos );

//			Vec3dMA( patch->trace_origin, 1.0, patch->norm, patch->center );

                        patch->next = cs->patches;
                        cs->patches = patch;
			bsurf_patch_num++;
                }
        
	cs->width = width;
	cs->height = height;

        FreeSurfacePoints( mesh );
        FreeSurfacePoints( nmesh );
}



/*
  ==============================
  SetupBSurfacePatches

  ==============================
*/
void SetupBSurfacePatches( bsurface_t *list )
{
	bsurface_t		*bsurf;

	printf ( "setup curved surface patches ...\n" );

	bsurf_patch_num = 0;
	for ( bsurf = list; bsurf ; bsurf=bsurf->next )
	{
		BuildBSurfacePatches( bsurf );
	}
	
	printf( " %d patches\n", bsurf_patch_num );
}
