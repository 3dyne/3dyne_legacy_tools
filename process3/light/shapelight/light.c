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



// light.c

#include "light.h"
#include "cdb_service.h"


/*
  ====================
  misc

  ====================
*/
projectionType GetProjectionTypeOfPlane( cplane_t *pl )
{
	int	type;
	type = pl->type & PLANE_AXIS_MASK;

	if ( type == PLANE_X || type == PLANE_ANYX )
		return ProjectionType_X;
	else if ( type == PLANE_Y || type == PLANE_ANYY )
		return ProjectionType_Y;
	else if ( type == PLANE_Z || type == PLANE_ANYZ )
		return ProjectionType_Z;

	Error( "GetProjectionTypeOfPlane: can't get type.\n" );
	return 0;
}

void GetProjectionVecs( vec3d_t right, vec3d_t up, projectionType type )
{
	if ( type == ProjectionType_X )
	{
		Vec3dInit( right, 0, 0, 1 );
		Vec3dInit( up, 0, 1, 0 );
	}
	else if ( type == ProjectionType_Y )
	{
		Vec3dInit( right, 1, 0, 0 );
		Vec3dInit( up, 0, 0, 1 );
	}
	else if ( type == ProjectionType_Z )
	{
		Vec3dInit( right, 1, 0, 0 );
		Vec3dInit( up, 0, 1, 0 );
	}
}

void ProjectVec3d( vec2d_t out, vec3d_t in, projectionType type )
{
	if ( type == ProjectionType_X )
	{
		out[0] = in[2];
		out[1] = in[1];		
	}
	else if ( type == ProjectionType_Y )
	{
		out[0] = in[0];
		out[1] = in[2];
	}
	else if ( type == ProjectionType_Z )
	{
		out[0] = in[0];
		out[1] = in[1];
	}
	else
	{
		Error( "ProjectVec3d: unkown projection type.\n" );
	}
}



/*
  ====================
  Material

  ====================
*/

void SetupMaterial( char *ident, material_t *mat, patch_t *list, hobj_t *material )
{
	hpair_t		*pair;

	// 
	// normal_scale
	//
	
	pair = FindHPair( material, "normal_scale" );
	if ( pair )
	{
		patch_t		*p;
		fp_t		normal_scale;
		
		HPairCastToFloat_safe( &normal_scale, pair );
		
		for ( p = list; p ; p=p->next )
			Vec3dMA( p->light_origin, normal_scale, p->norm, p->light_origin );
	}
	else
	{
		patch_t		*p;

		for ( p = list; p ; p=p->next )
			Vec3dMA( p->light_origin, 1.0, p->norm, p->light_origin );	
	}
	
	//
	// no_light
	//
	pair = FindHPair( material, "no_light" );
	if ( pair )
	{
		mat->no_light = true;
	}
	else
	{
		mat->no_light = false;
	}

	//
	// self_color
	// 
	pair = FindHPair( material, "self_color" );
	if ( pair )
	{
#if 1
		vec3d_t		self_color;
		patch_t		*p;
		
		HPairCastToVec3d_safe( self_color, pair );
		if ( self_color[0] == 0.0 && self_color[1] == 0.0 && self_color[2] == 0.0 )
		{
			mat->self_light = false;
		}
		else
		{
			mat->self_light = true;
			
//			for ( p = list; p ; p=p->next )
//				Vec3dCopy( p->color, self_color );
		}
#else

		printf( "WARNING: self_color disabled\n" );
		mat->self_light = false;
#endif
	}
	else
	{
		mat->self_light = false;
	}
		
	//
	// emit_color
	//
	pair = FindHPair( material, "patch_value" );
	if ( pair )
	{
		HPairCastToFloat( &mat->emit_value, pair );	
		
		pair = FindHPair( material, "emit_color" );
		if ( !pair )
			Error( "missing 'emit_color' in material '%s'.\n", material->name );
		HPairCastToVec3d_safe( mat->emit_color, pair );
		mat->emit_light = true;				
		
		if ( mat->emit_value == 0.0 )
			mat->emit_light = false;
	}
	else
	{
		mat->emit_light = false;
		mat->emit_value = 0.0;
		Vec3dInit( mat->emit_color, 0, 0, 0 );
	}
	
	//
	// spec_pow
	//
	pair = FindHPair( material, "spec_pow" );
	if ( pair )
	{
		HPairCastToFloat_safe( &mat->spec_pow, pair );
	}
	else
	{
		mat->spec_pow = 1.0;
	}

	//
	// is_sky
	//
	pair = FindHPair( material, "is_sky" );
	if ( pair )
	{
		mat->is_sky = true;
	}
	else
	{
		mat->is_sky = false;
	}
}

void SetupPolygonMaterial( face_t *list, hmanager_t *materialhm )
{
	face_t		*f;
	hpair_t		*pair;
	hobj_t		*material;

	for ( f = list; f ; f=f->next )
	{
		pair = FindHPair( f->shape, "material" );

		if ( !pair )
			Error( "missing 'material' in shape '%s'.\n", f->shape->name );

		material = HManagerSearchClassName( materialhm, pair->value );

		if ( !material )
		{
			Error( "can't find material '%s'\n", pair->value );
		}

		SetupMaterial( pair->value, &f->mat, f->patches, material );
	}
}

void SetupSurfacesMaterial( bsurface_t *list, hmanager_t *materialhm )
{
	bsurface_t	*cs;
	hpair_t		*pair;
	hobj_t		*material;

	for ( cs = list; cs ; cs=cs->next )
	{
		pair = FindHPair( cs->shape, "material" );
		if ( !pair )
			Error( "missing 'material' in shape '%s'.\n", cs->shape->name );
		
		material = HManagerSearchClassName( materialhm, pair->value );

		if ( !material )
		{
			Error( "can't find material '%s'\n", pair->value );
		}

		SetupMaterial( pair->value, &cs->mat, cs->patches, material );
	}
}


/*
  ====================
  Light

  ====================
*/
void Light( hmanager_t *athm, face_t *list, bsurface_t *cslist, hobj_t *light_source_root )
{
	hobj_search_iterator_t	iter;
	hobj_t		*light;
	face_t		*f;
	bsurface_t	*cs;
	int		num;

	u_list_t	hit_list;	

	hobj_t		*source;
				
	Lighting_SetConvergence( 0.05 );

	//
	// do all normal 'light' classes
	// 
	InitClassSearchIterator( &iter, HManagerGetRootClass( athm ), "light" );
	{
		for ( ; ( light = SearchGetNextClass( &iter ) ); )
		{
			printf( "pointlight %s: ", light->name );			
			
			Lighting_SetupPointLight( light );
			num = 0;

			source = Lighting_CreateLightSource();

			InsertClass( light_source_root, source );
			

			//
			// put all objects, lit by the light into the hit_list
			//
			U_InitList( &hit_list );

			//
			// light faces
			//

			for ( f = list; f ; f=f->next )
			{
				if ( f->mat.no_light || f->mat.self_light )
					continue;

				if ( Lighting_CullTest( f->min3d, f->max3d, f->pl->norm, f->pl->dist ) )
				{
					Lighting_FacePatches( f );
					U_ListInsertAtHead( &hit_list, f );
					num++;
				}
			}

			// balance world scarps
			FaceListSetupWorldPatches( &hit_list );
			Balance_AllScraps();
			
			// build lightmaps
			FaceListBuildLightmaps( &hit_list, source );
			Balance_CleanUpNodes();

			// clean up
			FaceListInitAllPatches( &hit_list );
			U_CleanUpList( &hit_list, NULL );

#if 1
			//
			// light curved surfaces
			//

			//
			// put all objects, lit by the light into the hit_list
			//
			U_InitList( &hit_list );

			for ( cs = cslist; cs ; cs=cs->next )
			{
				if ( cs->mat.no_light || cs->mat.self_light )
					continue;	

				if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
				{
					Lighting_CurvedSurfacePatches( cs );
					U_ListInsertAtHead( &hit_list, cs );
					num++;
				}
			}

			CSurfListBuildLightmaps( &hit_list, source );
			CSurfListInitAllPatches( &hit_list );
			U_CleanUpList( &hit_list, NULL );
#endif

			printf( "%d objects lighted\n", num );
		}
	}

#if 1
	//
	// do all 'spotlight' classes
	//
	InitClassSearchIterator( &iter, HManagerGetRootClass( athm ), "spotlight" );
	{
		for ( ; ( light = SearchGetNextClass( &iter ) ); )
		{
			printf( "spotlight %s: ", light->name );

			Lighting_SetupSpotLight( light );
			num = 0;

			source = Lighting_CreateLightSource();

			InsertClass( light_source_root, source );

			//
			// put all objects, lit by the light into the hit_list
			//
			U_InitList( &hit_list );

			//
			// light faces
			//

			for ( f = list; f ; f=f->next )
			{
				if ( f->mat.no_light || f->mat.self_light )
					continue;
				
				if ( Lighting_CullTest( f->min3d, f->max3d, f->pl->norm, f->pl->dist ) )
				{
					Lighting_FacePatches( f );
					U_ListInsertAtHead( &hit_list, f );
					num++;
				}
			}

			// balance world scarps
			FaceListSetupWorldPatches( &hit_list );
			Balance_AllScraps();
			
			// build lightmaps
			FaceListBuildLightmaps( &hit_list, source );
			Balance_CleanUpNodes();

			// clean up
			FaceListInitAllPatches( &hit_list );
			U_CleanUpList( &hit_list, NULL );

#if 1
			//
			// light curved surfaces
			//

			//
			// put all objects, lit by the light into the hit_list
			//
			U_InitList( &hit_list );

			for ( cs = cslist; cs ; cs=cs->next )
			{
				if ( cs->mat.no_light || cs->mat.self_light )
					continue;	

				if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
				{
					Lighting_CurvedSurfacePatches( cs );
					U_ListInsertAtHead( &hit_list, cs );
					num++;
				}
			}

			CSurfListBuildLightmaps( &hit_list, source );
			CSurfListInitAllPatches( &hit_list );
			U_CleanUpList( &hit_list, NULL );

#endif
			printf( "%d objects lighted\n", num );
		}
	}	       
#endif
}


void EmittingFaceLight_Faces( face_t *list, bsurface_t *cslist, hobj_t *light_source_root )
{
	face_t		*f, *f2;
	bsurface_t	*cs;
	int		num;

	u_list_t	hit_list;	
	hobj_t		*source;

	Lighting_SetConvergence( 0.01 );

	for ( f = list; f ; f=f->next )
	{
		if ( !f->mat.emit_light )
			continue;
		
		printf( "emit: " );

		Lighting_SetupFaceLight( f );
		num = 0;

		source = Lighting_CreateLightSource();

		InsertClass( light_source_root, source );
		
		//
		// put all objects, lit by the light into the hit_list
		//
		U_InitList( &hit_list );

		for ( f2 = list; f2; f2=f2->next )
		{
			if ( f2->mat.no_light || f2->mat.self_light )
				continue;
			
			if ( f == f2 )
				continue;

			if ( Lighting_CullTest( f2->min3d, f2->max3d, f2->pl->norm, f2->pl->dist ) )
			{
				Lighting_FacePatches( f2 );
				U_ListInsertAtHead( &hit_list, f2 );
				num++;
			}
		}

		// balance world scarps
		FaceListSetupWorldPatches( &hit_list );
		Balance_AllScraps();
		
		// build lightmaps
		FaceListBuildLightmaps( &hit_list, source );
		Balance_CleanUpNodes();
		
		// clean up
		FaceListInitAllPatches( &hit_list );
		U_CleanUpList( &hit_list, NULL );
		

#if 1
		//
		// light curved surfaces
		//

		U_InitList( &hit_list );

		for ( cs = cslist; cs ; cs=cs->next )
		{
			if ( cs->mat.no_light || cs->mat.self_light )
				continue;

			if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
			{
				Lighting_CurvedSurfacePatches( cs );
				U_ListInsertAtHead( &hit_list, cs );
				num++;
			}
		}

		CSurfListBuildLightmaps( &hit_list, source );
		CSurfListInitAllPatches( &hit_list );
		U_CleanUpList( &hit_list, NULL );
#endif	
	
		printf( "%d objects lighted\n", num );
	}
	printf( "ok.\n" );
}


/*
  ====================
  CompileBrushClass

  ====================
*/
face_t * NewFace( void )
{
	return NEWTYPE( face_t );       
}

void FreeFace( face_t *f )
{
	free( f );
}

polygon_t * BuildPolygonFromPolygonClass( hobj_t *polygon )
{
	polygon_t	*p;
	hpair_t		*pair;
	int		i, num;
	char		tt[256];
	
	pair = FindHPair( polygon, "num" );
	if ( !pair )
		Error( "missing pointnum 'num' of polygon '%s'.\n", polygon->name );
	HPairCastToInt_safe( &num, pair );

	p = NewPolygon( num );
	p->pointnum = num;

	for ( i = 0; i < num; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( polygon, tt );
		if ( !pair )
			Error( "missing point '%s' of polygon '%s'.\n", tt, polygon->name );
		HPairCastToVec3d( p->p[i], pair );
	}

	return p;
}



/*
  ==============================
  BuildPolygonLightList

  ==============================
*/
face_t *   BuildPolygonLightList( hmanager_t *shapehm, hmanager_t *planehm )
{
	int		i;
	face_t		*list, *f;

	hobj_search_iterator_t	iter;

	hobj_t		*shape;
	hobj_t		*plane;
	hobj_t		*polygon;
	hpair_t		*pair;

	int		pointnum;
	char		tt[256];

	int		face_num = 0;
	
	list = NULL;

	InitClassSearchIterator( &iter, HManagerGetRootClass( shapehm ), "shape" );
	for ( ; ( shape = SearchGetNextClass( &iter ) ); )
	{	
		
		pair = FindHPair( shape, "tess_name" );
		if ( !pair )
			Error( "missing key 'tess_name'\n" );

		if ( !strcmp( pair->value, "sface" ) || 
		     !strcmp( pair->value, "cface" ) ||
		     !strcmp( pair->value, "meshtile" ) )
		{
			
		}
		else
		{
			continue;
		}
		
		// no material, no light
		if ( !FindHPair( shape, "material" ) )
			continue;

		polygon = FindClassType( shape, "polygon" );
		if ( !polygon )
			Error( "missing class 'polygon' in shape '%s'\n", shape->name );


		f = NewFace();

		pair = FindHPair( polygon, "plane" );
		if ( !pair )
			Error( "missing 'plane' in polygon '%s'.\n", polygon->name );
		plane = HManagerSearchClassName( planehm, pair->value );
		if ( !plane )
			Error( "polygon '%s' can't find plane '%s'.\n", polygon->name, pair->value );
		f->pl = GetClassExtra( plane );
	
		if ( !f->pl )
			Error( "(null) plane\n" );
	
		f->self = polygon;
		f->shape = shape;
		f->next = list;
		list = f;
		
		f->p = BuildPolygonFromPolygonClass( polygon );
		EasyFindInt( &pointnum, polygon, "num" );
		f->p = NewPolygon( pointnum );
		f->p->pointnum = pointnum;
		for ( i = 0; i < pointnum; i++ )
		{
			sprintf( tt, "%d", i );
			EasyFindVec3d( f->p->p[i], polygon, tt );
		}
		
		face_num++;
	}
	
	printf( " %d polygons\n", face_num );
	return list;
}


/*
  ====================
  ReadPlaneClass

  ====================
*/
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
		pl = NewCPlane();

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


int main( int argc, char *argv[] )
{
	// common
	char		*in_node_name;
	char		*in_plane_name;
	char		*in_at_name;
	char		*out_at_name;
	char		*in_material_name;

	// shapes
	char		*in_shape_name;
	char		*out_shape_name;

	hmanager_t	*nodehm;
	hmanager_t	*planehm;
	hmanager_t	*athm;
	hmanager_t	*materialhm;
	
	hmanager_t		*shapehm;
	
	face_t		*facelist;
	bsurface_t	*bsurfacelist;

	FILE		*h;

	hobj_t		*light_source_root;
	
	
	MicroProfile_begin();

	printf( "===== light - builds lightmaps for a bspbrush class =====\n" );
	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-n" );
	in_plane_name = GetCmdOpt2( "-pl" );
	in_at_name = GetCmdOpt2( "-ati" );
	in_material_name = GetCmdOpt2( "-m" );

	in_shape_name = GetCmdOpt2( "-i" );
	out_shape_name = GetCmdOpt2( "-o" );

	CDB_StartUp( 0 );

	if ( !in_node_name )
	{
		in_node_name = "_bspout_bspnode.hobj";
		printf( " default input node class: %s\n", in_node_name );		
	}
	else
	{
		printf( " input node class: %s\n", in_node_name );
	}

	if ( !in_plane_name )
	{
		in_plane_name = "_plane.hobj";
		printf( " default input plane class: %s\n", in_plane_name );		
	}
	else
	{
		printf( " input plane class: %s\n", in_plane_name );
	}


	if ( !in_at_name )
	{
		in_at_name = "ats.hobj";
		printf( " default input archetype class: %s\n", in_at_name );		
	}
	else
	{
		printf( " input archetype class: %s\n", in_at_name );
	}

	if ( !in_shape_name )
	{
		Error( "no input shape class\n" );
	}
	else
	{
		printf( "input shape class: %s\n", in_shape_name );
	}

	if ( !out_shape_name )
	{
		Error( "no output shape class\n" );
	}
	else
	{
		printf( " output shape class: %s\n", out_shape_name );
	}

	if ( !in_material_name )
	{
		printf( "no material class name\n" );
	}
	else
	{
		printf( " material class: %s\n", in_material_name );
 	}


	printf( "load material class ...\n" );
	if ( !(materialhm = NewHManagerLoadClass( in_material_name ) ) )
	     Error( "load failed.\n" );

	printf( "load node class ...\n" );
	if ( !(nodehm = NewHManagerLoadClass( in_node_name ) ) )
		Error( "load failed.\n" );

	printf( "load shape class ...\n" );
	if ( !(shapehm = NewHManagerLoadClass( in_shape_name ) ) )
		Error( "load failed.\n" );

	printf( "load archetype class ...\n" );
	if ( !(athm = NewHManagerLoadClass( in_at_name ) ) )
		Error( "load failed.\n" );

	
	planehm = ReadPlaneClass( in_plane_name );


	CompileNodeClass( nodehm, planehm );
	
	facelist = BuildPolygonLightList( shapehm, planehm );
	bsurfacelist = BuildSurfaceLightList( shapehm );

	Balance_Init();

	// normal light
	SetupPatches( facelist, false ); // allow dynamic patchsize
	SetupBSurfacePatches( bsurfacelist );

	SetupPolygonMaterial( facelist, materialhm );
	SetupSurfacesMaterial( bsurfacelist, materialhm );

	FaceListBuildLightdefs( facelist );
	CSurfListBuildLightdefs( bsurfacelist );
	Lightmap_Begin( "_lightmap.bin", "_lightmap.hobj" );

	// all objects that emit light, creates a 'lightsource' object
	light_source_root = NewClass( "lightsources", "lightsources0" );

#if 1
	Light( athm, facelist, bsurfacelist, light_source_root );
	if ( !CheckCmdSwitch2( "--no-facelight" ) )
	{
		EmittingFaceLight_Faces( facelist, bsurfacelist, light_source_root );
	}
	else
	{
		printf( "Switch: --no-facelight\n" );
	}
#endif

//	SetupWorldPatches( facelist );
//	Balance_Dump();
//	Balance_AllScraps();



//	BuildLightmaps( facelist );                  
//	BuildLightmaps_curved_surface( bsurfacelist );      
	
	Lightmap_End();

	h = fopen( out_shape_name, "w" );                               
	if ( !h )                                                       
		Error( "can't open file.\n" );                          
	WriteClass( HManagerGetRootClass( shapehm ), h );               
	fclose( h );

	h = fopen( "_light_source.hobj", "w" );
	if ( !h )
		Error( "can't open file\n" );
	WriteClass( light_source_root, h );
	fclose( h );

	HManagerSaveID();

	MicroProfile_end();

	exit(0);
}
