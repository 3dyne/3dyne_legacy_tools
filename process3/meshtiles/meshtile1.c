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



// meshtile1.c

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_math.h"
#include "lib_poly.h"
#include "lib_hobj.h"
#include "lib_container.h"
#include "lib_trimesh.h"
#include "lib_gldbg.h"
#include "cmdpars.h"    

gld_session_t	*gld;

unsigned char	glmesh_base[1024*1024*16];
int		glmesh_ofs = 0;

void CalcPoint( vec3d_t out, vec3d_t in, vec3d_t norm, fp_t dist, fp_t u_shift, fp_t v_shift, fp_t rotate, vec2d_t scale )
{
	vec2d_t		tmp, tmp2;

	fp_t		angle, s, c;
	vec2d_t		vec0, vec1;
	
	angle = rotate / 180.0*M_PI;
	
	s = sin( angle );
	c = cos( angle );
	
	// shift
	tmp[0] = in[0] + u_shift;
	tmp[1] = in[1] + v_shift;
	
	// rotate + scale

	tmp[0] *= scale[0];
	tmp[1] *= scale[1];

	tmp2[0] = tmp[0]*c - tmp[1]*s;
	tmp2[1] = tmp[0]*s + tmp[1]*c;

	
	// projection
	Vec3dProjectOrthoOnPlane( out, tmp2, norm, dist, LibMath_GetNormType( norm ) );
	Vec3dMA( out, in[2], norm, out );
}


/*
  ==============================
  GenBaseTile

  ==============================
*/
u_list_t * GenBaseTile( u_list_t *raw_poly_list, fp_t rotate, vec2d_t scale, fp_t u_shift, fp_t v_shift, vec3d_t norm, fp_t dist )
{
	int		i;
	polygon_t	*p, *pnew;
	u_list_iter_t	iter;
	u_list_t		*list;

//	printf( "gen: shf: %f %f scl: %f %f\n", u_shift, v_shift, scale[0], scale[1] );

	list = NEWTYPE( u_list_t );
	U_InitList( list );

	U_ListIterInit( &iter, raw_poly_list );
	for( ; ( p = U_ListIterNext( &iter ) ) ; )
	{		
		pnew = CopyPolygon( p );
		U_ListInsertAtHead( list, pnew );
		for ( i = 0; i < p->pointnum; i++ )
		{
			CalcPoint( pnew->p[i], p->p[i], norm, dist, u_shift, v_shift, rotate, scale );
		}
	}	
	return list;
}

void DrawPolygonList( u_list_t *list )
{
	int		i;
	u_list_iter_t	iter;
	polygon_t	*p;

	U_ListIterInit( &iter, list );
	for( ; ( p = U_ListIterNext( &iter ) ) ; )
	{	
		if ( !p )
			continue;

		GLD_Begin( gld, "polygon" );
		for ( i = 0; i < p->pointnum; i++ )
		{
			GLD_Vertex3fv( gld, p->p[i] );
		}
		GLD_End( gld );
	}	
}

/*
  ==================================================
  Base Tile Mesh

  ==================================================
*/

typedef struct scan_frag_s
{
	fp_t		dist;	
	polygon_t	*p;	
} scan_frag_t;

u_list_t * ScanPolygon( polygon_t *poly, vec3d_t norm, fp_t dist, fp_t step )
{
	fp_t		max_d;
	int		i;
	fp_t		d;
	polygon_t	*remain;

	u_list_t	*frag_list;

//	printf( "dist: %f, step: %f\n", dist,step );
//	Vec3dPrint( norm );

	// get max dist of poly towards the plane
	
	max_d = -999999.9;
	for ( i = 0; i < poly->pointnum; i++ )
	{
		d = Vec3dDotProduct( norm, poly->p[i] ) - dist;
		if ( d > max_d )
			max_d = d;
	}

	if ( max_d < 0.0 )
	{
		d = -ceil( (-max_d)/step ) * step;
	}
	else
	{
		d = ceil( (max_d)/step ) * step;
	}

	dist += d;
	
	remain = CopyPolygon( poly );

	frag_list = NEWTYPE( u_list_t );
	U_InitList( frag_list );

	for( ; remain ; )
	{
		polygon_t	*front, *back;
		
//		printf( "split dist: %f\n", dist );

		SplitPolygon( remain, norm, dist, &front, &back );
		
		if ( front )
		{
			scan_frag_t	*frag;

			frag = NEWTYPE( scan_frag_t );
			frag->p = front;
			frag->dist = dist;
			U_ListInsertAtHead( frag_list, frag );
			
//			printf( "hit!\n" );
			
		}
		
		dist -= step;
		remain = back;
	}
	return frag_list;
}

u_list_t * GenBaseTileMesh( polygon_t *surf_poly, vec3d_t norm, fp_t dist, vec2d_t vec0, vec2d_t vec1, vec2d_t shift, u_list_t *raw_poly_list, fp_t u_size, fp_t v_size, fp_t rotate, vec2d_t scale )
{
	int		type;
	vec3d_t		u_vec;
	vec3d_t		v_vec;
	u_list_t		*u_frag_list;	
	u_list_t		*v_frag_list;	

	u_list_iter_t	u_iter;
	u_list_iter_t	v_iter;
	scan_frag_t	*u_frag;
	scan_frag_t	*v_frag;

	int		num_frag;

	u_list_t		*tile_list;

	fp_t		angle, s, c;
	vec2d_t		tmp[2] = { {1,0},{0,1} };
	vec2d_t		tmp2[2];

	tile_list = NEWTYPE( u_list_t );
	U_InitList( tile_list );

	type = LibMath_GetNormType( norm );

	
	angle = rotate / 180.0*M_PI;
	s = sin( angle );
	c = cos( angle );       
	tmp2[0][0] = tmp[0][0]*c - tmp[0][1]*s;
	tmp2[0][1] = tmp[0][0]*s + tmp[0][1]*c;

	tmp2[1][0] = tmp[1][0]*c - tmp[1][1]*s;
	tmp2[1][1] = tmp[1][0]*s + tmp[1][1]*c;

	if ( (type&libMathNormType_axis_mask) == libMathNormType_x )
	{
		Vec3dInit( u_vec, 0.0, tmp2[0][1], tmp2[0][0] );
		Vec3dInit( v_vec, 0.0, tmp2[1][1], tmp2[1][0] );			
	}
	else if ( (type&libMathNormType_axis_mask) == libMathNormType_y )
	{
		Vec3dInit( u_vec, tmp2[0][0], 0.0, tmp2[0][1] );
		Vec3dInit( v_vec, tmp2[1][0], 0.0, tmp2[1][1] );			
	}
	else if ( (type&libMathNormType_axis_mask) == libMathNormType_z )
	{
		Vec3dInit( u_vec, tmp2[0][0], tmp2[0][1], 0.0 );
		Vec3dInit( v_vec, tmp2[1][0], tmp2[1][1], 0.0 );			
	} 

//	printf( "scale: %f %f\n", scale[0], scale[1] );

	num_frag = 0;
//	printf( "u_split:\n" );
	u_frag_list = ScanPolygon( surf_poly, u_vec, shift[0], u_size * scale[0] );
	U_ListIterInit( &u_iter, u_frag_list );
	for ( ; ( u_frag = U_ListIterNext( &u_iter ) ) ; )
	{
//		printf( "v_split:\n" );
		v_frag_list = ScanPolygon( u_frag->p, v_vec, shift[1], v_size * scale[1] );
		U_ListIterInit( &v_iter, v_frag_list );
		for ( ; ( v_frag = U_ListIterNext( &v_iter ) ) ; )
		{
			u_list_t	*list;
			void		*obj;

			list = GenBaseTile( raw_poly_list, rotate, scale, u_frag->dist/scale[0], v_frag->dist/scale[1], norm, dist );

			for ( ; (obj = U_ListRemoveAtHead( list ) ); )
			{
				U_ListInsertAtHead( tile_list, obj );
			}
//			U_ListInsertListAtHead( tile_list, GenBaseTile( raw_poly_list, rotate, scale, u_frag->dist, v_frag->dist, norm, dist ) );
			
			num_frag++;
		}
	}
	
	printf( " %d tile frags\n", num_frag );

	if ( !(type&libMathNormType_pos) )
	{
		u_list_iter_t	iter;
		polygon_t	*p;

		printf( " flip !\n" );
		U_ListIterInit( &iter, tile_list );
		for ( ; ( p = U_ListIterNext( &iter ) ) ; )
		{
			int		i;
			vec3d_t		v[128];
			if ( p->pointnum >= 128 )
				Error( "too many points in polygon, can't flip\n" );
			for ( i = 0; i < p->pointnum; i++ )
				Vec3dCopy( v[i], p->p[i] );
			for ( i = 0; i < p->pointnum; i++ )
				Vec3dCopy( p->p[i], v[(p->pointnum-1)-i] );
		}
	}

	return tile_list;
}


/*
  ==================================================
  Polygon List Stuff

  ==================================================
*/

/*
  ==============================
  ReadPolygonList

  ==============================
*/
u_list_t * ReadPolygonList( char *name )
{
	u_list_t	*poly_list;
	hobj_t		*poly_root;
	hobj_search_iterator_t	iter;
	hobj_t		*polygon;
	tokenstream_t	*ts;

	poly_list = NEWTYPE( u_list_t );

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "can't open raw polygon class '%s'\n", name );

	poly_root = ReadClass( ts );
	EndTokenStream( ts );
	
	U_InitList( poly_list );
	InitClassSearchIterator( &iter, poly_root, "polygon" );
	for ( ; ( polygon = SearchGetNextClass( &iter ) ) ; ) 
	{
		polygon_t		*p;

		p = CreatePolygonFromClass( polygon );
		if ( !p )
			Error( "CreatePolygonFromClass failed\n" );

		U_ListInsertAtHead( poly_list, p );
	}

	return poly_list;
}

/*
  ==============================
  NormalizePolygonList

  ==============================
*/
void NormalizePolygonList( u_list_t *poly_list )
{
	int			i;
	u_list_iter_t		iter;
	polygon_t		*p;
	vec3d_t			min, max;
	vec3d_t			scale;

	Vec3dInitBB( min, max, 999999.9 );
	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		for ( i = 0; i < p->pointnum; i++ )
		{
			Vec3dAddToBB( min, max, p->p[i] );
		}
	}

	// shift to (0,0,0)
	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		for ( i = 0; i < p->pointnum; i++ )
		{
			Vec3dSub( p->p[i], p->p[i], min );
		}
	}	

	// scale to (0,0,0)-(1,1,1)
	scale[0] = 1.0/(max[0]-min[0]);
	scale[1] = 1.0/(max[1]-min[1]);
	scale[2] = 1.0/(max[2]-min[2]);

	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		for ( i = 0; i < p->pointnum; i++ )
		{
			p->p[i][0] *= scale[0];
			p->p[i][1] *= scale[1];
			p->p[i][2] *= scale[2];
		}
	}	
}

/*
  ==============================
  ScalePolygonList

  ==============================
*/
void ScalePolygonList( u_list_t *poly_list, vec3d_t scale )
{
	int			i;
	u_list_iter_t		iter;
	polygon_t		*p;

	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		for ( i = 0; i < p->pointnum; i++ )
		{
			p->p[i][0] *= scale[0];
			p->p[i][1] *= scale[1];
			p->p[i][2] *= scale[2];
		}
	}
	
}

/*
  ==============================
  ShiftPolygonList

  ==============================
*/
void ShiftPolygonList( u_list_t *poly_list, vec3d_t shift )
{
	int			i;
	u_list_iter_t		iter;
	polygon_t		*p;

	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		for ( i = 0; i < p->pointnum; i++ )
		{
			p->p[i][0] += shift[0];
			p->p[i][1] += shift[1];
			p->p[i][2] += shift[2];
		}
	}
	
}


void ClipPolygonList( u_list_t *poly_list, vec3d_t norm, fp_t dist )
{
	u_list_iter_t		iter;
	polygon_t		*p;
	
	U_ListIterInit( &iter, poly_list );	
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{		

		U_ListIterRemoveGoPrev( &iter );			

		ClipPolygonInPlace( &p, norm, dist );

		if ( p )
		{
			U_ListIterInsertAsNext( &iter, p );
		}
		
	}
}

/*
  ====================
  ReadPlaneClass

  ====================
*/
typedef struct cplane_s
{
	hobj_t		*self;
	vec3d_t		norm;
	fp_t		dist;
	int		type;
	struct cplane_s	*flipplane;
} cplane_t;

hmanager_t * ReadPlaneClass( char *name )
{
	tokenstream_t	*ts;
	hobj_t		*planecls;
	hmanager_t	*hm;
	hobj_search_iterator_t	iter;
	hobj_t		*plane;
	hobj_t		*flipplane;
	cplane_t		*pl;
	int		num;
	hpair_t		*pair;

	ts = BeginTokenStream( name );
	planecls = ReadClass( ts );
	EndTokenStream( ts );

	hm = NewHManager();
	HManagerSetRootClass( hm, planecls );
	HManagerRebuildHash( hm );

	//
	// create compiled planes
	//

	fprintf( stderr, "load plane class and compile ...\n" );

	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		pl = NEWTYPE( cplane_t );

		// plane norm
		pair = FindHPair( plane, "norm" );
		if ( !pair )
			Error( "missing plane normal.\n" );
		HPairCastToVec3d_safe( pl->norm, pair );

		// plane dist
		pair = FindHPair( plane, "dist" );
		if ( !pair )
			Error( "missing plane distance.\n" );
		HPairCastToFloat_safe( &pl->dist, pair );
		
		// plane type
		pair = FindHPair( plane, "type" );
		if ( !pair )
			Error( "missing plane type.\n" );
		HPairCastToInt_safe( &pl->type, pair );

		pl->self = plane;
		SetClassExtra( plane, pl );
		
	}

	//
	// resolve clsref_flipplane
	//
	InitClassSearchIterator( &iter, planecls, "plane" );

	for ( num = 0; ( plane = SearchGetNextClass( &iter ) ); num++ )
	{
		// plane flipplane clsref
		pair = FindHPair( plane, "flipplane" );
		if ( !pair )
			Error( "missinig clsref flipplane" );

		flipplane = HManagerSearchClassName( hm, pair->value );
		if ( !flipplane )
			Error( "can't resolve clsref flipplane.\n" );

		pl = GetClassExtra( plane );
		pl->flipplane = GetClassExtra( flipplane );
	}

	printf( " %d planes\n", num );

	return hm;
}

/*
  ==============================
  BuildTriMesh

  ==============================
*/

void BuildTriMesh( hobj_t *shape, u_list_t *poly_list )
{
	u_list_iter_t		iter;
	polygon_t		*p;
	trimesh_t		trimesh;
	unique_t	ids[3];

	TriMesh_Init( &trimesh );

	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		if ( p->pointnum == 3 )
		{
			ids[0] = TriMesh_AddFreeVertex( &trimesh, p->p[0], 0.5 ); 
			ids[1] = TriMesh_AddFreeVertex( &trimesh, p->p[1], 0.5 ); 
			ids[2] = TriMesh_AddFreeVertex( &trimesh, p->p[2], 0.5 ); 

			if ( ids[0] == ids[1] || ids[0] == ids[2] || ids[1] == ids[2] )
			{
			}
			else
			{
				TriMesh_AddTri( &trimesh, UNIQUE_INVALIDE, ids );
			}
			
		}
		else
		{
			int		j;
			polygon_t		*tmp;
			
			tmp = NewPolygon( 3 );
			tmp->pointnum = 3;
			
			Vec3dCopy( tmp->p[0], p->p[0] );

			for ( j = 1; j < p->pointnum-1; j++ )
			{

				Vec3dCopy( tmp->p[1], p->p[j] );
				Vec3dCopy( tmp->p[2], p->p[j+1] );

				ids[0] = TriMesh_AddFreeVertex( &trimesh, tmp->p[0], 0.5 ); 
				ids[1] = TriMesh_AddFreeVertex( &trimesh, tmp->p[1], 0.5 ); 
				ids[2] = TriMesh_AddFreeVertex( &trimesh, tmp->p[2], 0.5 ); 

				if ( ids[0] == ids[1] || ids[0] == ids[2] || ids[1] == ids[2] )
				{
				}
				else
				{
					TriMesh_AddTri( &trimesh, UNIQUE_INVALIDE, ids );
				}

			}
			FreePolygon( tmp );
		}
	}

	TriMesh_Dump( &trimesh );

	{
		hobj_t		*glmesh;		

		printf( "glmesh: ofs %d - ", glmesh_ofs );
		glmesh = TriMesh_BuildGLMesh( &trimesh, glmesh_base, &glmesh_ofs );
		printf( "%d\n", glmesh_ofs-1 );

		InsertClass( shape, glmesh );
	}
}

/*
  ==============================
  BuildMeshTileShape

  ==============================
*/

hobj_t * BuildMeshTileShape( hobj_t *surf_poly, hobj_t *plane, hobj_t *texdef, hpair_t *mat_pair, u_list_t *poly_list )
{
	hobj_t		*shape;
	hobj_t		*tmp;
	hobj_t		*polygon;
	hobj_t		*meshtile;

	u_list_iter_t	iter;
	polygon_t	*p;	

	int		poly_num;

	shape = EasyNewClass( "shape" );

	InsertHPair( shape, NewHPair2( "string", "tess_name", "meshtile" ) );

	polygon = DeepCopyClass( surf_poly );
	EasyNewClsref( polygon, "plane", plane );

	InsertClass( shape, polygon );

	if ( mat_pair )
	{
		InsertHPair( shape, NewHPair2( "ref", "material", mat_pair->value ) );
	}
	else
	{
		InsertHPair( shape, NewHPair2( "ref", "material", "default" ) );
	}

	{
		hobj_t		*texdef0;

		texdef0 = EasyNewClass( "proj_texdef0" );
		InsertClass( shape, texdef0 );
		
		EasyNewClsref( texdef0, "texdef", texdef );
	}


	meshtile = EasyNewClass( "meshtile" );
	InsertClass( shape, meshtile );

#if 0
	poly_num = 0;
	U_ListIterInit( &iter, poly_list );
	for ( ; ( p = U_ListIterNext( &iter ) ) ; )
	{
		if ( p->pointnum == 3 )
		{
			InsertClass( meshtile, CreateClassFromPolygon( p ) );
			poly_num++;
		}
		else
		{
			int		j;
			polygon_t		*tmp;
			
			tmp = NewPolygon( 3 );
			tmp->pointnum = 3;
			
			Vec3dCopy( tmp->p[0], p->p[0] );

			for ( j = 1; j < p->pointnum-1; j++ )
			{

				Vec3dCopy( tmp->p[1], p->p[j] );
				Vec3dCopy( tmp->p[2], p->p[j+1] );
				InsertClass( meshtile, CreateClassFromPolygon( tmp ) );
				poly_num++;
			}
			FreePolygon( tmp );
		}
	}

	EasyNewInt( meshtile, "polynum", poly_num );
#endif

	return shape;
}

int main( int argc, char *argv[] )
{
	char		*in_brush_name;
	char		*out_shape_name;
	char		*out_glmesh_name;
	char		*in_plane_name;
	char		*in_texdef_name;
	char		*in_texture_name;
	char		*in_tm_name;
	char		*path_name;
	
	hobj_t		*brush_root;	
	hmanager_t	*plane_hm;
	hmanager_t	*texdef_hm;
	hmanager_t	*texture_hm;
	hobj_t		*tm_root;
	hobj_t		*meshtile_root;

	hobj_search_iterator_t	brush_iter;
	hobj_search_iterator_t	surf_iter;
	hobj_search_iterator_t	surf2_iter;
	hobj_t		*brush;
	hobj_t		*surf;
	hobj_t		*surf2;
	
	hobj_t		*shape_root;
	hobj_t		*shape;
	FILE		*h;

	int		num_total_tris;

	char		tt[256];


	gld = GLD_BeginSession( "xxx" );
	GLD_StartRecord( gld );
	GLD_BeginList( gld, "polys", "line" );

	puts( "===== meshtile1 - create meshtiles from surfaces =====" );
	SetCmdArgs( argc, argv );

	if ( argc == 1 )
	{
		puts( "usage:" );
		puts( " -i	: input bspbrush class" );
		puts( " -o	: output shape class" );
		puts( " -obin	: output glmesh binary" );
		puts( " -pl	: input plane class" );
		puts( " -td	: input texdef class" );
		puts( " -tx	: input texture class" );
		puts( " -tm	: input texture material class" );
		puts( " -path	: config path to ./shape_config and ./meshtiles" );

		exit(-1);
	}

	in_brush_name = GetCmdOpt2( "-i" );
	out_shape_name = GetCmdOpt2( "-o" );
	out_glmesh_name = GetCmdOpt2( "-obin" );
	in_plane_name = GetCmdOpt2( "-pl" );
	in_texdef_name = GetCmdOpt2( "-td" );
	in_texture_name = GetCmdOpt2( "-tx" );
	in_tm_name = GetCmdOpt2( "-tm" );
	path_name = GetCmdOpt2( "-path" );

	if ( !in_brush_name )
		Error( "no input bspbrush class\n" );
	if ( !out_shape_name )
		Error( "no output shape class\n" );
	if ( !out_glmesh_name )
		Error( "no output glmesh binary\n" );
	if ( !in_plane_name )
		Error( "no input plane class\n" );
	if ( !in_texdef_name )
		Error( "no input texdef class\n" );
	if ( !in_texture_name )
		Error( "no input texture class\n" );
	if ( !in_tm_name )
		Error( "no input texture material class\n" );
	if ( !path_name )
		Error( "no config path\n" );

	brush_root = ReadClassFile( in_brush_name );
	if ( !brush_root )
		Error( "can't read bspbrush class\n" );

	texdef_hm = NewHManagerLoadClass( in_texdef_name );
	if ( !texdef_hm )
		Error( "can't read texdef class\n" );

	plane_hm = ReadPlaneClass( in_plane_name );

	texture_hm = NewHManagerLoadClass( in_texture_name );
	if ( !texture_hm )
		Error( "can't read texture class\n" );

	tm_root = ReadClassFile( in_tm_name );
	if ( !tm_root )
		Error( "can't read texture material class" );

	sprintf( tt, "%s/shape_config/meshtile.hobj", path_name );
	meshtile_root = ReadClassFile( tt );
	if ( !meshtile_root )
		Error( "can't read meshtile class ( %s )\n", tt );


	shape_root = NewClass( "shapes", "meshtiles0" );
	
	//
	// for all c5 brushes
	//

	num_total_tris = 0;

	InitClassSearchIterator( &brush_iter, brush_root, "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &brush_iter ) ) ; )
	{
		int		b_contents;
		int		num_surf;

		vec3d_t		v;
		hobj_t		*surf_poly_obj;
		polygon_t	*surf_poly;
		

		EasyFindInt( &b_contents, brush, "content" );
		if ( b_contents != 5 )
		{
			continue;
		}


		//
		// for all surfaces
		//
		InitClassSearchIterator( &surf_iter, brush, "surface" );
		for ( num_surf = 0; ( surf = SearchGetNextClass( &surf_iter ) ) ; num_surf++ )
		{
			int	s_contents;

			hobj_t	*plane;
			hobj_t	*texdef;
			hobj_t	*texture;
			hobj_t	*meshtile;
			hpair_t	*pair;
			hpair_t	*mat_pair;
#if 1
			EasyFindInt( &s_contents, surf, "content" );

			if ( !(s_contents & 32) )
			{				
				// no substructur flag
				continue;
			}
#endif
//			GenerateMeshtile( surf, plane_hm, texdef_hm, 

			plane = EasyLookupClsref( surf, "plane", plane_hm );
			texdef = EasyLookupClsref( surf, "texdef", texdef_hm );
			texture = EasyLookupClsref( texdef, "texture", texture_hm );

			pair = FindHPair( texture, "ident" );
			if ( !pair )
				Error( "missing key 'ident'\n" );

			meshtile = FindClass( meshtile_root, pair->value );
			if ( !meshtile )
			{
				Error( "no meshtile defs for ident '%s'\n", pair->value );
			}

			mat_pair = FindHPair( tm_root, pair->value );
			
			{
				int		i, j;
				vec3d_t		norm;
				fp_t		dist;

				hobj_t		*plane2;
				vec3d_t		norm2;
				fp_t		dist2;				

				fp_t		rotate;
				vec2d_t		shift;
				vec2d_t		scale;
				vec2d_t		vec0, vec1;

				fp_t		u_size, v_size, height;
				u_list_t	*raw_poly_list;


				u_list_t		*base_tile_mesh;
				

				surf_poly_obj = FindClassType( surf, "polygon" );
				surf_poly = CreatePolygonFromClass( surf_poly_obj );

				EasyFindVec3d( norm, plane, "norm" );
				EasyFindFloat( &dist, plane, "dist" );
				EasyFindVec2d( shift, texdef, "shift" );
				EasyFindVec2d( vec0, texdef, "vec0" );
				EasyFindVec2d( vec1, texdef, "vec1" );
			     

				if ( vec0[0] == 0.0 && vec0[1] == 0.0 )
				{
					vec0[0] = 1.0;
				}
				if ( vec1[0] == 0.0 && vec1[1] == 0.0 )
				{
					vec1[1] = 1.0;
				}

				EasyFindVec2d( scale, texdef, "scale" );
				EasyFindFloat( &rotate, texdef, "rotate" );
				
				EasyFindFloat( &u_size, meshtile, "u_size" );
				EasyFindFloat( &v_size, meshtile, "v_size" );
				EasyFindFloat( &height, meshtile, "height" );
				
				pair = FindHPair( meshtile, "raw_path" );
				if ( !pair )
					Error( "missing key 'raw_path'\n" );
				sprintf( tt, "%s/%s", path_name, pair->value );
				raw_poly_list = ReadPolygonList( tt );
				if ( !raw_poly_list )
					Error( "can't load raw polygons\n" );

				printf( "%s: %d raw polygons/tile \n", pair->value, U_ListLength( raw_poly_list ) );
				NormalizePolygonList( raw_poly_list );
				{
					vec3d_t		scl;
					scl[0] = u_size;
					scl[1] = v_size;
					scl[2] = height;
					ScalePolygonList( raw_poly_list, scl );
				}

				{
					vec3d_t		shf;
					shf[0] = 0.0;
					shf[1] = 0.0;
					shf[2] = -height;
					ShiftPolygonList( raw_poly_list, shf );
				}
				
				base_tile_mesh = GenBaseTileMesh( surf_poly, norm, dist, vec0, vec1, shift, raw_poly_list, u_size, v_size, rotate, scale );

				//
				// clip base by all surface planes
				//

				InitClassSearchIterator( &surf2_iter, brush, "surface" );
				for ( ; ( surf2 = SearchGetNextClass( &surf2_iter ) ) ; )
				{
					if ( surf2 == surf )
						continue;
					
					EasyFindInt( &s_contents, surf2, "content" );
					
					if ( !(s_contents & 32) )
					{				
						// no substructur flag
						continue;
					}

					plane2 = EasyLookupClsref( surf2, "plane", plane_hm );
					EasyFindVec3d( norm2, plane2, "norm" );
					EasyFindFloat( &dist2, plane2, "dist" );
					
					ClipPolygonList( base_tile_mesh, norm2, dist2 );
				}


				for ( i = 0; i < surf_poly->pointnum; i++ )
				{
					j = (i+1==surf_poly->pointnum)?0:(i+1);
					
					// search surf, which the edge is on
					InitClassSearchIterator( &surf2_iter, brush, "surface" );
					for ( ; ( surf2 = SearchGetNextClass( &surf2_iter ) ) ; )
					{
						if ( surf2 == surf )
							continue;

						plane2 = EasyLookupClsref( surf2, "plane", plane_hm );
						EasyFindVec3d( norm2, plane2, "norm" );
						EasyFindFloat( &dist2, plane2, "dist" );

						if ( fabs(Vec3dDotProduct( surf_poly->p[i], norm2 )-dist2 ) < 0.1 &&
						     fabs(Vec3dDotProduct( surf_poly->p[j], norm2 )-dist2 ) < 0.1 )
						{
							// that's the surf

							vec3d_t		delta1, delta2;
							vec3d_t		cross;
							
							Vec3dSub( delta1, surf_poly->p[j], surf_poly->p[i] );
							Vec3dAdd( delta2, norm, norm2 );
							Vec3dUnify( delta1 );
							Vec3dUnify( delta2 );
							
							Vec3dCrossProduct( cross, delta1, delta2 );
							Vec3dUnify( cross );

							dist2 = Vec3dInitPlane2( cross, surf_poly->p[i] );
							ClipPolygonList( base_tile_mesh, cross, dist2 );
							break;							
						}
					}					
				}

//				DrawPolygonList( base_tile_mesh );

				//
				// build meshtile shape
				//
				plane = EasyLookupClsref( surf, "plane", plane_hm );
				shape = BuildMeshTileShape( surf_poly_obj, plane, texdef, mat_pair, base_tile_mesh );
				BuildTriMesh( shape, base_tile_mesh );
				InsertClass( shape_root, shape );

				num_total_tris += U_ListLength( base_tile_mesh );
			}
			
			
		}
	}

	printf( " generated %d triangles\n", num_total_tris );

	GLD_EndList( gld );       
	GLD_Update( gld );
	GLD_Pause( gld );
	GLD_EndSession( gld );
	

	h = fopen( out_shape_name, "w" );
	if ( !h )
		Error( "can't write shape class\n" );
	WriteClass( shape_root, h );
	fclose( h );

	h = fopen( out_glmesh_name, "w" );
	if ( !h )
		Error( "can't write glmesh binary\n" );
	fwrite( glmesh_base, glmesh_ofs, 1, h );
	fclose( h );

	HManagerSaveID();

	exit(0);
}
