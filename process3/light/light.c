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

void SetupMaterial( char *ident, material_t *mat, patch_t *list, hmanager_t *texturehm, hmanager_t *materialhm, hmanager_t *texturematerialhm )
{
	hpair_t		*pair;
	hobj_t		*material;

	// is there a special material for 'texture'
	pair = FindHPair( HManagerGetRootClass( texturematerialhm ), ident );
	if ( !pair )
	{
		// no, take default material
		material = HManagerSearchClassName( materialhm, "default" );
		if ( !material )
			Error( "can't find 'default' material class.\n" );
	}
	else
	{
		material = HManagerSearchClassName( materialhm, pair->value );
		if ( !material )
			Error( "can't find material '%s' for texture with ident '%s'.\n", pair->value, ident );
	}

	
	// 
	// normal_scale
	//
	
	pair = FindHPair( material, "normal_scale" );
	if ( pair )
	{
		fp_t		normal_scale;
		patch_t		*p;
		
		HPairCastToFloat_safe( &normal_scale, pair );
		
		for ( p = list; p ; p=p->next )
			Vec3dMA( p->center, normal_scale, p->norm, p->origin );
	}
	else
	{
		// keep normal_scale = 1
		// done by SetupFacePatches
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
			
			for ( p = list; p ; p=p->next )
				Vec3dCopy( p->color, self_color );
		}
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

void SetupFacesMaterial( face_t *list, hmanager_t *texturehm, hmanager_t *materialhm, hmanager_t *texturematerialhm )
{
	face_t		*f;
	hpair_t		*pair;
	hobj_t		*texture;

	for ( f = list; f ; f=f->next )
	{
		pair = FindHPair( f->texdef, "texture" );
		if ( !pair )
			Error( "missing 'texture' in texdef '%s'.\n", f->texdef->name );
		texture = HManagerSearchClassName( texturehm, pair->value );

		if ( !texture )
			Error( "texdef '%s' can't find texture '%s'.\n", f->texdef->name, pair->value );
		pair = FindHPair( texture, "ident" );
		if ( !pair )
			Error( "missing 'ident' in texture '%s'.\n", texture->name );
		
		SetupMaterial( pair->value, &f->mat, f->patches, texturehm, materialhm, texturematerialhm );
	}
}

void SetupCSurfacesMaterial( bsurface_t *list, hmanager_t *texturehm, hmanager_t *materialhm, hmanager_t *texturematerialhm )
{
	bsurface_t	*cs;
	hpair_t		*pair;

	for ( cs = list; cs ; cs=cs->next )
	{
		pair = FindHPair( cs->self, "ident" );
		if ( !pair )
			Error( "missing 'ident' in curved surface '%s'.\n", cs->self->name );

		SetupMaterial( pair->value, &cs->mat, cs->patches, texturehm, materialhm, texturematerialhm );
	}
}

#if 0
void SetupFacesMaterial_old( face_t *list, hmanager_t *texturehm, hmanager_t *materialhm, hmanager_t *texturematerialhm )
{
	face_t		*f;
	hpair_t		*pair;
	hobj_t		*material;
	hobj_t		*texture;

	printf( "setup materials ...\n" );

	for ( f = list; f ; f=f->next )
	{
		pair = FindHPair( f->texdef, "texture" );
		if ( !pair )
			Error( "missing 'texture' in texdef '%s'.\n", f->texdef->name );
		texture = HManagerSearchClassName( texturehm, pair->value );

		if ( !texture )
			Error( "texdef '%s' can't find texture '%s'.\n", f->texdef->name, pair->value );
		pair = FindHPair( texture, "ident" );
		if ( !pair )
			Error( "missing 'ident' in texture '%s'.\n", texture->name );
		
		// is there a special material for 'texture'
		pair = FindHPair( HManagerGetRootClass( texturematerialhm ), pair->value );
		if ( !pair )
		{
			// no, take default material
			material = HManagerSearchClassName( materialhm, "default" );
			if ( !material )
				Error( "can't find 'default' material class.\n" );
		}
		else
		{
			material = HManagerSearchClassName( materialhm, pair->value );
			if ( !material )
				Error( "can't find material '%s' for ident of texture '%s'.\n", pair->value, texture->name );
		}

	
		// 
		// normal_scale
		//
	
		pair = FindHPair( material, "normal_scale" );
		if ( pair )
		{
			fp_t		normal_scale;
			patch_t		*p;

			HPairCastToFloat_safe( &normal_scale, pair );

			for ( p = f->patches; p ; p=p->next )
				Vec3dMA( p->center, normal_scale, p->norm, p->origin );
		}
		else
		{
			// keep normal_scale = 1
			// done by SetupFacePatches
		}

		//
		// no_light
		//
		pair = FindHPair( material, "no_light" );
		if ( pair )
		{
			f->no_light = true;
		}
		else
		{
			f->no_light = false;
		}

		//
		// self_color
		// 
		pair = FindHPair( material, "self_color" );
		if ( pair )
		{
			vec3d_t		self_color;
			patch_t		*p;

			HPairCastToVec3d_safe( self_color, pair );
			if ( self_color[0] == 0.0 && self_color[1] == 0.0 && self_color[2] == 0.0 )
			{
				f->self_light = false;
			}
			else
			{
				f->self_light = true;
				
				for ( p = f->patches; p ; p=p->next )
					Vec3dCopy( p->color, self_color );
			}
		}
		else
		{
			f->self_light = false;
		}
		
		//
		// emit_color
		//
		pair = FindHPair( material, "patch_value" );
		if ( pair )
		{
			HPairCastToFloat( &f->emit_value, pair );	

			pair = FindHPair( material, "emit_color" );
			if ( !pair )
				Error( "missing 'emit_color' in material '%s'.\n", material->name );
			HPairCastToVec3d_safe( f->emit_color, pair );
			f->emit_light = true;				

			if ( f->emit_value == 0.0 )
				f->emit_light = false;
		}
		else
		{
			f->emit_light = false;
			f->emit_value = 0.0;
			Vec3dInit( f->emit_color, 0, 0, 0 );
		}
	
		//
		// spec_pow
		//
		pair = FindHPair( material, "spec_pow" );
		if ( pair )
		{
			HPairCastToFloat_safe( &f->spec_pow, pair );
		}
		else
		{
			f->spec_pow = 1.0;
		}

		//
		// is_sky
		//
		pair = FindHPair( material, "is_sky" );
		if ( pair )
		{
			f->is_sky = true;
		}
		else
		{
			f->is_sky = false;
		}
	}
}
#endif

/*
  ====================
  Light

  ====================
*/
void Light( hmanager_t *athm, face_t *list, bsurface_t *cslist )
{
	hobj_search_iterator_t	iter;
	hobj_t		*light;
	face_t		*f;
	bsurface_t	*cs;
	int		num;

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
					num++;
				}
			}

			//
			// light curved surfaces
			//

			for ( cs = cslist; cs ; cs=cs->next )
			{
				if ( cs->mat.no_light || cs->mat.self_light )
					continue;	

				if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
				{
					Lighting_CurvedSurfacePatches( cs );
					num++;
				}
			}

			printf( "%d objects lighted\n", num );
		}
	}

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
					num++;
				}
			}

			//
			// light curved surfaces
			//

			for ( cs = cslist; cs ; cs=cs->next )
			{
				if ( cs->mat.no_light || cs->mat.self_light )
					continue;	

				if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
				{
					Lighting_CurvedSurfacePatches( cs );
					num++;
				}
			}			

			printf( "%d objects lighted\n", num );
		}
	}	       
}

void LightVolume( hmanager_t *athm, map3_t *map )
{
	hobj_search_iterator_t	iter;
	hobj_t		*light;
	face_t		*f;
	int		num;

	Lighting_SetConvergence( 0.05 );

#if 0
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
			for ( f = list; f ; f=f->next )
			{
				if ( f->no_light || f->self_light )
					continue;

				if ( Lighting_CullTest( f->min3d, f->max3d, f->pl->norm, f->pl->dist ) )
				{
					Lighting_FacePatches( f );
					num++;
				}
			}
			printf( "%d faces lighted\n", num );
		}
	}
#endif

	//
	// do all 'spotlight' classes
	//
	InitClassSearchIterator( &iter, HManagerGetRootClass( athm ), "spotlight" );
	{
		for ( ; ( light = SearchGetNextClass( &iter ) ); )
		{
			printf( "spotlight %s: light volume\n", light->name );

			Lighting_SetupSpotLight( light );
			Lighting_Volume( map );

//			printf( "%d faces lighted\n", num );
		}
	}	       
}
  
void EmittingFaceLight_Volume( face_t *list, map3_t *map )
{
	face_t		*f, *f2;
	int		num;

	Lighting_SetConvergence( 0.05 );

	for ( f = list; f ; f=f->next )
	{
		if ( !f->mat.emit_light )
			continue;

		printf( "emit: " );
		Lighting_SetupFaceLight( f );

		num = 0;
		printf( " lighting volume ...\n" );
		Lighting_Volume( map );		      	
	}
	printf( "ok.\n" );
}

void EmittingFaceLight_Faces( face_t *list, bsurface_t *cslist )
{
	face_t		*f, *f2;
	bsurface_t	*cs;
	int		num;

	Lighting_SetConvergence( 0.01 );

	for ( f = list; f ; f=f->next )
	{
		if ( !f->mat.emit_light )
			continue;

		printf( "emit: " );
		Lighting_SetupFaceLight( f );

		num = 0;
		for ( f2 = list; f2; f2=f2->next )
		{
			if ( f2->mat.no_light || f2->mat.self_light )
				continue;
			
			if ( f == f2 )
				continue;

			if ( Lighting_CullTest( f2->min3d, f2->max3d, f2->pl->norm, f2->pl->dist ) )
			{
				Lighting_FacePatches( f2 );
				num++;
			}
		}

		//
		// light curved surfaces
		//
		
		for ( cs = cslist; cs ; cs=cs->next )
		{
			if ( cs->mat.no_light || cs->mat.self_light )
				continue;

			if ( Lighting_CullTest( cs->min3d, cs->max3d, NULL, 0 ) )
			{
				Lighting_CurvedSurfacePatches( cs );
				num++;
			}
		}
		
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
	return NEW( face_t );       
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


face_t * CompileBrushClass( hmanager_t *brushhm, hmanager_t *planehm, hmanager_t *texdefhm, hmanager_t *texturehm )
{
	face_t		*list, *f;

	hobj_search_iterator_t	brushiter;
	hobj_search_iterator_t	surfiter;
	hobj_search_iterator_t	polyiter;

	hobj_t		*brush;
	hobj_t		*surface;
	hobj_t		*plane;
	hobj_t		*texdef;
	hobj_t		*poly;
	hpair_t		*pair;

	cplane_t	*pl;

	int		face_num = 0;

	printf( "compile brush class ...\n" );
	
	list = NULL;

	InitClassSearchIterator( &brushiter, HManagerGetRootClass( brushhm ), "bspbrush" );
	for ( ; ( brush = SearchGetNextClass( &brushiter ) ); )
	{
		InitClassSearchIterator( &surfiter, brush, "surface" );
		for ( ; ( surface = SearchGetNextClass( &surfiter ) ); )
		{
			pair = FindHPair( surface, "texdef" );
			if ( !pair )
			{
				// surface has no texdef, so it will get no lighting
				continue;
			}
			texdef = HManagerSearchClassName( texdefhm, pair->value );
			if ( !texdef )
				Error( "surface '%s' can't find texdef '%s'.\n", surface->name, pair->value );			
			
			pair = FindHPair( surface, "plane" );
			if ( !pair )
				Error( "missing 'plane' in surface '%s'.\n", surface->name );
			plane = HManagerSearchClassName( planehm, pair->value );
			if ( !plane )
				Error( "surface '%s' can't find plane '%s'.\n", surface->name, pair->value );
			pl = GetClassExtra( plane );

			InitClassSearchIterator( &polyiter, surface, "polygon" );
			for ( ; ( poly = SearchGetNextClass( &polyiter ) ); )
			{
				f = NewFace();
				f->self = poly;
				f->surface = surface;
				f->brush = brush;
				f->texdef = texdef;
				f->next = list;
				list = f;

				f->pl = pl;
				f->p = BuildPolygonFromPolygonClass( poly );

				face_num++;
			}
		}
	}		

	printf( " %d faces.\n", face_num );

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
	char		*in_node_name;
	char		*in_brush_name;
	char		*in_plane_name;
	char		*in_texdef_name;
	char		*in_texture_name;
	char		*in_at_name;
	char		*in_material_name;
	char		*in_texture_material_name;
	char		*in_mapnode_name;
	char		*in_bsurface_name;
	char		*out_brush_name;
	char		*out_lightdef_name;
	char		*out_volume_name;

	char		*texpath;
	char		texture_buf[256];
	char		material_buf[256];

	hmanager_t	*nodehm;
	hmanager_t	*brushhm;
	hmanager_t	*planehm;
	hmanager_t	*texdefhm;
	hmanager_t	*texturehm;
	hmanager_t	*athm;
	hmanager_t	*materialhm;
	hmanager_t	*texturematerialhm;
	hmanager_t	*bsurfacehm;
	
	hmanager_t		*mapnodehm;		// pvs

	face_t		*facelist;
	bsurface_t	*bsurfacelist;

	FILE		*h;
	
	vec3d_t		skyvec = { -0.2, -1.0, 0.5 };
//		vec3d_t		skycolor = { 0.3, 0.3, 0.5 };
//		vec3d_t		skycolor = { 0.5, 0.5, 0.9 };
	vec3d_t		skycolor = { 1.0, 1.0, 1.0 };
//		vec3d_t		skycolor = { 0.5, 0.5, 0.5 };
	
	MicroProfile_begin();

	printf( "===== light - builds lightmaps for a bspbrush class =====\n" );
	SetCmdArgs( argc, argv );

	in_node_name = GetCmdOpt2( "-n" );
	in_brush_name = GetCmdOpt2( "-b" );
	in_plane_name = GetCmdOpt2( "-pl" );
	in_texdef_name = GetCmdOpt2( "-td" );
	in_texture_name = GetCmdOpt2( "-tex" );
	in_at_name = GetCmdOpt2( "-at" );
	in_material_name = GetCmdOpt2( "-m" );
	in_texture_material_name = GetCmdOpt2( "-tm" );
	in_mapnode_name = GetCmdOpt2( "-map" );
	in_bsurface_name = GetCmdOpt2( "-bs" );
	out_brush_name = GetCmdOpt2( "-ob" );
	out_lightdef_name = GetCmdOpt2( "-ld" );
	out_volume_name = GetCmdOpt2( "-v" );

	CDB_StartUp( 0 );

	texpath = CDB_GetString( "process3/texture_path" ); 

	if( !texpath )
		Error( "missing 'process3/texture_path' in cdb\n" );
		
	if ( !in_node_name )
	{
		in_node_name = "_bspout_bspnode.hobj";
		printf( " default input node class: %s\n", in_node_name );		
	}
	else
	{
		printf( " input node class: %s\n", in_node_name );
	}

	if ( !in_brush_name )
	{
		in_brush_name = "_surfmerge_bspbrush.hobj";
		printf( " default input brush class: %s\n", in_brush_name );		
	}
	else
	{
		printf( " input brush class: %s\n", in_brush_name );
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

	if ( !in_texdef_name )
	{
		in_texdef_name = "_texdef.hobj";
		printf( " default input texdef class: %s\n", in_texdef_name );		
	}
	else
	{
		printf( " input texdef class: %s\n", in_texdef_name );
	}

	if ( !in_texture_name )
	{
		in_texture_name = "_texture.hobj";
		printf( " default input texture class: %s\n", in_texture_name );		
	}
	else
	{
		printf( " input texture class: %s\n", in_texture_name );
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

	if ( !in_bsurface_name )
	{
		in_bsurface_name = "csurfaces.hobj";
		printf( " default input curved surface class: %s\n", in_bsurface_name );
	}
	else
	{
		printf( " input curved surface class: %s\n", in_bsurface_name );
	}

	if ( !out_brush_name )
	{
		out_brush_name = "_light_bspbrush.hobj";
		printf( " default output brush class: %s\n", out_brush_name );		
	}
	else
	{
		printf( " output brush class: %s\n", out_brush_name );
	}

	if ( !out_lightdef_name )
	{
		out_lightdef_name = "_light_lightdef.hobj";
		printf( " default lightdef class: %s\n", out_lightdef_name );		
	}
	else
	{
		printf( " output lightdef class: %s\n", out_lightdef_name );
	}	

	if ( !out_volume_name )
	{
		out_volume_name = "_light_volume.bin";
		printf( " default volume binary: %s\n", out_volume_name );
	}
	else
	{
		printf( " output volume binary: %s\n", out_volume_name );
	}

	if ( !in_material_name )
	{
		strncpy( material_buf, texpath, 256 );
		strcat( material_buf, "/material.hobj" );
//		in_material_name = "/mnt/gs/agony1/arch00/textures/material.hobj.debug";
//		in_material_name = "/home/mcb/art/material.hobj.debug";
		in_material_name = material_buf;
		printf( " default material class: %s\n", in_material_name );
	}
	else
	{
		printf( " material class: %s\n", in_material_name );
 	}

	if ( !in_texture_material_name )
	{
		strncpy( texture_buf, texpath, 256 );
		strcat( texture_buf, "/texture_material.hobj" );

//		in_texture_material_name = "/mnt/gs/agony1/arch00/textures/texture_material.hobj.debug";
//		in_texture_material_name = "/home/mcb/art/texture_material.hobj.debug";
		in_texture_material_name = texture_buf;
		printf( " default texture material class: %s\n", in_texture_material_name );
	}
	else
 	{
		printf( " texture material class: %s\n", in_texture_material_name );
	}

	if ( !in_mapnode_name )
	{
		printf( "no pvs\n" );
	}
	else
	{
		printf( " input mapnode class: %s\n", in_mapnode_name );
	}

	printf( "load material class ...\n" );
	if ( !(materialhm = NewHManagerLoadClass( in_material_name ) ) )
	     Error( "load failed.\n" );

	printf( "load texture material class ...\n" );
	if ( !(texturematerialhm = NewHManagerLoadClass( in_texture_material_name ) ) )
		Error( "load failed.\n" );

	printf( "load node class ...\n" );
	if ( !(nodehm = NewHManagerLoadClass( in_node_name ) ) )
		Error( "load failed.\n" );

	printf( "load brush class ...\n" );
	if ( !(brushhm = NewHManagerLoadClass( in_brush_name ) ) )
		Error( "load failed.\n" );

	printf( "load texdef class ...\n" );
	if ( !(texdefhm = NewHManagerLoadClass( in_texdef_name ) ) )
		Error( "load failed.\n" );

	printf( "load texture class ...\n" );
	if ( !(texturehm = NewHManagerLoadClass( in_texture_name ) ) )
		Error( "load failed.\n" );

	printf( "load archetype class ...\n" );
	if ( !(athm = NewHManagerLoadClass( in_at_name ) ) )
		Error( "load failed.\n" );

	printf( "load curved surface class ...\n" );
	if ( !(bsurfacehm = NewHManagerLoadClass( in_bsurface_name ) ) )
		Error( "load failed.\n" );
	
	planehm = ReadPlaneClass( in_plane_name );

	CompileNodeClass( nodehm, planehm );

	facelist = CompileBrushClass( brushhm, planehm, texdefhm, texturehm );
	bsurfacelist = CompileBSurfaceClass( bsurfacehm );


	if ( in_mapnode_name )
	{
		printf( "load mapnode class ...\n" );
		if ( !(mapnodehm = NewHManagerLoadClass( in_mapnode_name ) ) )
			Error( "load failed.\n" );
		PVS_CompileMapnodeClass( mapnodehm, planehm, brushhm, facelist );
	}


	if ( CheckCmdSwitch2( "--calc-fieldvectors" ) )
	{
		SetupPatches( facelist, true ); // fix patchsize of 16
		// calc volume fields
		printf( "Switch: --calc-fieldvectors\n" );
		DistributeFieldVectors( facelist );
	}

	else if ( CheckCmdSwitch2( "--only-sky" ) )
	{
		printf( "Switch: --only-sky\n" );
		SetupPatches( facelist, false ); // allow dynamic patchsize
		SetupFacesMaterial( facelist, texturehm, materialhm, texturematerialhm );

		SetupForSkyTrace( facelist, brushhm );
		SkyLight( facelist, skyvec, skycolor );

		BuildLightmaps( facelist, out_lightdef_name );
		
		h = fopen( out_brush_name, "w" );
		if ( !h )
			Error( "can't open file.\n" );
		WriteClass( HManagerGetRootClass( brushhm ), h );
		fclose( h );		
	}
	else
	{
		// normal light
		SetupPatches( facelist, false ); // allow dynamic patchsize
		SetupBSurfacePatches( bsurfacelist );
		SetupFacesMaterial( facelist, texturehm, materialhm, texturematerialhm );
		SetupCSurfacesMaterial( bsurfacelist, texturehm, materialhm, texturematerialhm );
		
		if ( CheckCmdSwitch2( "--with-sky" ) )
		{
			printf( "Switch: --with-sky\n" );
			if ( !mapnodehm )
				printf( "no sky light without pvs !\n" );
			else
			{
				SetupForSkyTrace( facelist, brushhm );
				SkyLight( facelist, skyvec, skycolor );	
			}
		}


		Light( athm, facelist, bsurfacelist  );
		if ( !CheckCmdSwitch2( "--no-facelight" ) )
		{
//			EmittingFaceLight( facelist );
			EmittingFaceLight_Faces( facelist, bsurfacelist );
		}
		else
		{
			printf( "Switch: --no-facelight\n" );
		}

		if ( CheckCmdSwitch2( "--with-volume" ) )
		{
			map3_t		*map;
			printf( "Switch: --with-volume\n" );
			map = NewMap3Hash();
			LightVolume( athm, map );
			EmittingFaceLight_Volume( facelist, map );
			Map3Chop01( map );
			h = fopen( out_volume_name, "w" );
			if ( !h )
				Error( "can't open output volume binary.\n" );
			WriteMap3( map, h );
			fclose( h );
		}

		BuildLightmaps( facelist, out_lightdef_name );
		BuildLightmaps_curved_surface( bsurfacelist, bsurfacehm );
		
		h = fopen( out_brush_name, "w" );
		if ( !h )
			Error( "can't open file.\n" );
		WriteClass( HManagerGetRootClass( brushhm ), h );
		fclose( h );
	}

	HManagerSaveID();

	MicroProfile_end();

	exit(0);
}
