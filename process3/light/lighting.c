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



// lighting.c

#include "light.h"

typedef enum
{
	LightingState_none,
	LightingState_pointlight,
	LightingState_spotlight,
	LightingState_facelight
} lightingState;

#define LIGHTING_CONVERGENCE	( 0.05 )

static lightingState	lighting_state;
static fp_t		lighting_convergence = LIGHTING_CONVERGENCE;
static vec3d_t		lighting_origin;
static vec3d_t		lighting_color;
static fp_t		lighting_value;
static fp_t		lighting_value_square;
static fp_t		lighting_maxdist;
static vec3d_t		lighting_min;
static vec3d_t		lighting_max;
static bool_t		lighting_nospec;
static face_t		*lighting_emitface;
static bool_t		lighting_usenormal;

static vec3d_t		lighting_spotdirection;
static fp_t		lighting_spotangle;
static fp_t		lighting_spotfalloff;

static fp_t		current_spec_pow = 0.0;
static fp_t		spec_tab[100];

void Lighting_SetupSpecTab( fp_t spec_pow )
{
	int		i;
	fp_t		x;

	if ( current_spec_pow == spec_pow )
		return;

	current_spec_pow = spec_pow;

	for ( i = 0; i < 100; i++ )
	{
		x = (fp_t)(i)/100.0;
		spec_tab[i] = ((-(cos( x*M_PI ))/2.0) +0.5) * pow(x, current_spec_pow );		
	}
}

void Lighting_SetConvergence( fp_t conv )
{
	lighting_convergence = conv;
}

void Lighting_SetupPointLight( hobj_t *light )
{
	int		i;
	hpair_t		*pair;

	// get origin
	pair = FindHPair( light, "origin" );
	if ( !pair )
		Error( "missing 'origin' in pointlight '%s'.\n", light->name );
	HPairCastToVec3d_safe( lighting_origin, pair );

	// get color
	pair = FindHPair( light, "color" );
	if ( !pair )
		Error( "missing 'color' in pointlight '%s'.\n", light->name );
	HPairCastToVec3d_safe( lighting_color, pair );

	// get value
	pair = FindHPair( light, "value" );
	if ( !pair )
		Error( "missing 'value' in pointlight '%s'.\n", light->name );
	HPairCastToFloat( &lighting_value, pair );

	// get nospec ...
	pair = FindHPair( light, "nospec" );
	if ( pair )
	{
		lighting_nospec = true;
		printf( "nospec=true " );
	}
	else
	{
		lighting_nospec = false;
	}

	// finish setup
	lighting_value_square = lighting_value * lighting_value;
	lighting_maxdist = sqrt( lighting_value_square / lighting_convergence/*LIGHTING_CONVERGENCE*/ );

	for ( i = 0; i < 3; i++ )
	{
		lighting_min[i] = lighting_origin[i] - lighting_maxdist;
		lighting_max[i] = lighting_origin[i] + lighting_maxdist;
	}

	lighting_state = LightingState_pointlight;
	lighting_usenormal = true;
}

void Lighting_SetupSpotLight( hobj_t *light )
{
	int		i;
	hpair_t		*pair;

	// get origin
	pair = FindHPair( light, "origin" );
	if ( !pair )
		Error( "missing 'origin' in pointlight '%s'.\n", light->name );
	HPairCastToVec3d_safe( lighting_origin, pair );

	// get color
	pair = FindHPair( light, "color" );
	if ( !pair )
		Error( "missing 'color' in pointlight '%s'.\n", light->name );
	HPairCastToVec3d_safe( lighting_color, pair );

	// get value
	pair = FindHPair( light, "value" );
	if ( !pair )
		Error( "missing 'value' in pointlight '%s'.\n", light->name );
	HPairCastToFloat( &lighting_value, pair );

	// get nospec ...
	pair = FindHPair( light, "nospec" );
	if ( pair )
	{
		lighting_nospec = true;
		printf( "nospec=true " );
	}
	else
	{
		lighting_nospec = false;
	}

	// get angle 
	pair = FindHPair( light, "angle" );
	if ( !pair )
		Error( "missing 'angle' in spotlight '%s'.\n", light->name );
	HPairCastToFloat_safe( &lighting_spotangle, pair );
	lighting_spotangle /= 2.0;
	lighting_spotangle = cos( lighting_spotangle / 180.0*M_PI );

	// get direction
	pair = FindHPair( light, "direction" );
	if ( !pair )
		Error( "missing 'direction' in spotlight '%s'.\n", light->name );
	HPairCastToVec3d_safe( lighting_spotdirection, pair );
	Vec3dUnify( lighting_spotdirection );

	// get falloff
	pair = FindHPair( light, "falloff" );
	if ( !pair )
		Error( "missing 'falloff' in spotlight '%s'.\n", light->name );
	HPairCastToFloat_safe( &lighting_spotfalloff, pair );
	

	// finish setup
	lighting_value_square = lighting_value * lighting_value;
	lighting_maxdist = sqrt( lighting_value_square / lighting_convergence/*LIGHTING_CONVERGENCE*/ );

	for ( i = 0; i < 3; i++ )
	{
		lighting_min[i] = lighting_origin[i] - lighting_maxdist;
		lighting_max[i] = lighting_origin[i] + lighting_maxdist;
	}
	
	lighting_state = LightingState_spotlight;	
	lighting_usenormal = true;
}

void Lighting_SetupFaceLight( face_t *f )
{
	int		i;

	lighting_emitface = f;

	lighting_value = f->mat.emit_value;
	lighting_value_square = lighting_value * lighting_value;
	
	lighting_maxdist = sqrt( lighting_value_square / lighting_convergence /*LIGHTING_CONVERGENCE*/ /*0.05*/ ); // 0.01

	for ( i = 0; i < 3; i++ )
	{
		lighting_min[i] = f->min3d[i] - lighting_maxdist;
		lighting_max[i] = f->max3d[i] + lighting_maxdist;
	}

	Vec3dCopy( lighting_color, f->mat.emit_color );

	lighting_nospec = false;

	lighting_state = LightingState_facelight;
//	lighting_usenormal = false;
	lighting_usenormal = true;
}

bool_t Lighting_CullTest( vec3d_t min, vec3d_t max, vec3d_t norm, fp_t dist )
{
	int		i;
	fp_t		d;

	if ( lighting_state == LightingState_pointlight || lighting_state == LightingState_facelight ||
	     lighting_state == LightingState_spotlight )
	{
		// bb test
		for ( i = 0; i < 3; i++ )
		{
			if ( lighting_min[i] > max[i] ||
			     lighting_max[i] < min[i] )
				break;
		}
		if ( i != 3 )
		{
			// bound box test failed
			return false;
		}
	}

	if ( norm )
	{
		if ( lighting_state == LightingState_pointlight || lighting_state == LightingState_spotlight )
		{
			d = Vec3dDotProduct( norm, lighting_origin ) - dist;
			if ( d < 0.0 )
			{
				// backface cull test failed
				return false;
			}
			
			if ( d > lighting_maxdist ) // can that happen after bb test ?
			{
				return false;
			}
			
//		return true;
		}
	}

//	Error( "Lighting_CullTest: unkown state.\n" );
	return true;
}

void Lighting_Patch_pointlight( patch_t *p )
{
	bool_t		hit;
	fp_t		len;
	fp_t		a;
	fp_t		b;
	fp_t		spec;
	vec3d_t		e;
	vec3d_t		v;


#if 1
	Vec3dSub( v, lighting_origin, p->center );
	len = Vec3dLen( v );

	if ( lighting_state == LightingState_facelight )
	{
		if ( len > lighting_maxdist )
			return;
	}
#endif

	hit = TraceLine( p->center, lighting_origin );
	if ( hit )
	{
//		Vec3dInit( p->color, 0, 0, 1.0 );
		return;
	}

//	Vec3dSub( v, lighting_origin, p->center );
	if ( lighting_usenormal )
	{
		Vec3dUnify2( e, v );
		a = Vec3dDotProduct( e, p->norm );
		if ( a < 0 )
			return;
	}
	else
	{
		a = 1.0;
	}
//	len = Vec3dLen( v );

	b = lighting_value_square / (len*len);

	if ( b >= 1.0 && !lighting_nospec )
	{
		// specularity
		spec = lighting_value - len;
		if ( spec <= 0.0 )
			spec = 0.0;
		else
			spec = spec / lighting_value;
		
		if ( spec >= 1.0 )
			spec = 1.0;

		spec = spec_tab[(int)(spec*99.0)];

		Vec3dScale( v, spec, lighting_color );
		Vec3dAdd( p->spec, p->spec, v );

		if ( p->spec[0] > 1.0 )
			p->spec[0] = 1.0;
		if ( p->spec[1] > 1.0 )
			p->spec[1] = 1.0;
		if ( p->spec[2] > 1.0 )
			p->spec[2] = 1.0;
	}

	// diffuse
	b *= a;
	if ( b > 1.0 )
	{
		b = 1.0;
	}

	Vec3dScale( v, b, lighting_color );
	Vec3dAdd( p->color, p->color, v );

	if ( p->color[0] > 1.0 )
		p->color[0] = 1.0;
	if ( p->color[1] > 1.0 )
		p->color[1] = 1.0;
	if ( p->color[2] > 1.0 )
		p->color[2] = 1.0;

}

void Lighting_Patch_spotlight( patch_t *p )
{
	bool_t		hit;
	fp_t		len;
	fp_t		a;
	fp_t		b;
	fp_t		spec;
	vec3d_t		e;
	vec3d_t		v;
	vec3d_t		ray;
	fp_t		dot;

#if 1
	Vec3dSub( v, p->center, lighting_origin );
	len = Vec3dLen( v );

	if ( len == 0 )
	{
		printf( "Warning: len == 0\n" );
		return;
	}

	if ( lighting_state == LightingState_facelight )
	{
		if ( len > lighting_maxdist )
			return;
	}
#endif

	// spot test
	Vec3dScale( ray, 1.0/len, v );
	dot = Vec3dDotProduct( lighting_spotdirection, ray );
	if ( dot < lighting_spotangle )
		return;

	hit = TraceLine( p->center, lighting_origin );
	if ( hit )
	{
//		Vec3dInit( p->color, 0, 0, 1.0 );
		return;
	}

//	Vec3dSub( v, lighting_origin, p->center );
	if ( lighting_usenormal )
	{
		Vec3dUnify2( e, v );
		a = -Vec3dDotProduct( e, p->norm );
//		printf( "%f ", a );
	}
	else
	{
		a = 1.0;
	}
//	len = Vec3dLen( v );

	b = lighting_value_square / (len*len);

	if ( b >= 1.0 && !lighting_nospec )
	{
		// specularity
		spec = lighting_value - len;
		if ( spec <= 0.0 )
			spec = 0.0;
		else
			spec = spec / lighting_value;
		
		if ( spec >= 1.0 )
			spec = 1.0;

		spec = spec_tab[(int)(spec*99.0)];

		Vec3dScale( v, spec, lighting_color );
		Vec3dAdd( p->spec, p->spec, v );

		if ( p->spec[0] > 1.0 )
			p->spec[0] = 1.0;
		if ( p->spec[1] > 1.0 )
			p->spec[1] = 1.0;
		if ( p->spec[2] > 1.0 )
			p->spec[2] = 1.0;
	}

	// diffuse
	b *= a;
	if ( b > 1.0 )
	{
		b = 1.0;
	}

	Vec3dScale( v, b, lighting_color );
	Vec3dAdd( p->color, p->color, v );

	if ( p->color[0] > 1.0 )
		p->color[0] = 1.0;
	if ( p->color[1] > 1.0 )
		p->color[1] = 1.0;
	if ( p->color[2] > 1.0 )
		p->color[2] = 1.0;
	
}

void Lighting_Volume_spotlight( vec3d_t pos, vec3d_t color )
{
	bool_t		hit;
	fp_t		len;
	fp_t		a;
	fp_t		b;
	fp_t		spec;
	vec3d_t		e;
	vec3d_t		v;
	vec3d_t		ray;
	fp_t		dot;

#if 1
	Vec3dSub( v, pos, lighting_origin );
	len = Vec3dLen( v );

	if ( len == 0 )
	{
		printf( "Warning: len == 0\n" );
		return;
	}

	if ( lighting_state == LightingState_facelight )
	{
		if ( len > lighting_maxdist )
			return;
	}
#endif

	// spot test
	Vec3dScale( ray, 1.0/len, v );
	dot = Vec3dDotProduct( lighting_spotdirection, ray );
	if ( dot < lighting_spotangle )
		return;

	hit = TraceLine( pos, lighting_origin );
	if ( hit )
	{
//		Vec3dInit( p->color, 0, 0, 1.0 );
		return;
	}


	b = lighting_value_square / (len*len);

	if ( b > 1.0 )
	{
		b = 1.0;
	}

	Vec3dScale( v, b, lighting_color );
	Vec3dAdd( color, color, v );

	if ( color[0] > 1.0 )
		color[0] = 1.0;
	if ( color[1] > 1.0 )
		color[1] = 1.0;
	if ( color[2] > 1.0 )
		color[2] = 1.0;
	
}



void Lighting_Volume_pointlight( vec3d_t pos, vec3d_t color )
{
	bool_t		hit;
	fp_t		len;
	fp_t		a;
	fp_t		b;
	fp_t		spec;
	vec3d_t		e;
	vec3d_t		v;

	hit = TraceLine( pos, lighting_origin );
	if ( hit )
	{
//		Vec3dInit( p->color, 0, 0, 1.0 );
		return;
	}

	Vec3dSub( v, lighting_origin, pos );
//	len = Vec3dLen( v );
	len = Vec3dLen( v );
	b = lighting_value_square / (len*len);

//	b*=0.5;

	if ( lighting_state == LightingState_facelight )
	{
		Vec3dUnify2( e, v );
		a = -Vec3dDotProduct( e, lighting_emitface->pl->norm );
		b*=a;
	}

	
	if ( b > 1.0 )
	{
		b = 1.0;
	}
	
	Vec3dScale( v, b, lighting_color );
	Vec3dAdd( color, color, v );
	
	if ( color[0] > 1.0 )
		color[0] = 1.0;
	if ( color[1] > 1.0 )
		color[1] = 1.0;
	if ( color[2] > 1.0 )
		color[2] = 1.0;

}


void Lighting_FacePatches( face_t *f )
{
	patch_t		*p, *p2;
	int		i;

	if ( lighting_state == LightingState_pointlight )
	{
		Lighting_SetupSpecTab( f->mat.spec_pow );		
		for ( p = f->patches; p ; p=p->next )
			Lighting_Patch_pointlight( p );

		return;
	}

	else if ( lighting_state == LightingState_spotlight )
	{
		Lighting_SetupSpecTab( f->mat.spec_pow );		
		for ( p = f->patches; p ; p=p->next )
			Lighting_Patch_spotlight( p );
		
		return;		
	}

	else if ( lighting_state == LightingState_facelight )
	{
		Lighting_SetupSpecTab( f->mat.spec_pow );
		
		for ( p = lighting_emitface->patches; p ; p=p->next )
		{	
			for ( i = 0; i < 3; i++ )
			{
				if ( p->center[i] < lighting_min[i] ||
				     p->center[i] > lighting_max[i] )
					break;
			}
			if ( i != 3 )
			{
				// bound box test failed
				continue;
			}

			Vec3dCopy( lighting_origin, p->center );			
			for ( p2 = f->patches; p2 ; p2=p2->next )
			{
				Lighting_Patch_pointlight( p2 );
			}
		}
		return;
	}

	Error( "Lighing_FacePatches: unkown state.\n" );
}


void Lighting_CurvedSurfacePatches( bsurface_t *cs )
{
	patch_t		*p, *p2;
	int		i;

	if ( lighting_state == LightingState_pointlight )
	{
		Lighting_SetupSpecTab( cs->mat.spec_pow );		
		for ( p = cs->patches; p ; p=p->next )
		{
			node_t		*leaf;
			leaf = FindLeafForPoint( p->center );
			leaf->ignore = true;
			Lighting_Patch_pointlight( p );
			leaf->ignore = false;
		}
		
		return;
	}
	else if ( lighting_state == LightingState_facelight )
	{
		bool_t		un;
		Lighting_SetupSpecTab( cs->mat.spec_pow );
	
		un = lighting_usenormal;
		lighting_usenormal = true;
	
		for ( p = lighting_emitface->patches; p ; p=p->next )
		{	
			for ( i = 0; i < 3; i++ )
			{
				if ( p->center[i] < lighting_min[i] ||
				     p->center[i] > lighting_max[i] )
					break;
			}
			if ( i != 3 )
			{
				// bound box test failed
				continue;
			}

			Vec3dCopy( lighting_origin, p->center );			
			for ( p2 = cs->patches; p2 ; p2=p2->next )
			{
				node_t		*leaf;
				leaf = FindLeafForPoint( p2->center );
				leaf->ignore = true;
				Lighting_Patch_pointlight( p2 );
				leaf->ignore = false;				
//				Lighting_Patch_pointlight( p2 );
			}
		}
		lighting_usenormal = un;
		return;
	}


}

#define LIGHT_CELL_SIZE		( 16.0 )

void Lighting_Volume( map3_t *map )
{
	vec3d_t		min, max;
	int		x, y, z;
	patch_t		*p;

	//
	// setup volume 
	//
	
	if ( lighting_state != LightingState_facelight && lighting_state != LightingState_spotlight )
		return;

	min[0] = floor( lighting_min[0]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;
	min[1] = floor( lighting_min[1]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;
	min[2] = floor( lighting_min[2]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;

	max[0] = ceil( lighting_max[0]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;
	max[1] = ceil( lighting_max[1]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;
	max[2] = ceil( lighting_max[2]/LIGHT_CELL_SIZE )*LIGHT_CELL_SIZE;	

	printf( "(%.2f, %.2f, %.2f)\n", max[0]-min[0], max[1]-min[1], max[2]-min[2] );


	for ( x = min[0]; x <= max[0]; x+=LIGHT_CELL_SIZE )
	{
		for ( y = min[1]; y <= max[1]; y+=LIGHT_CELL_SIZE )
		{
			for ( z = min[2]; z <= max[2]; z+=LIGHT_CELL_SIZE )
			{
				veccell_t	*c;
				int		xc, yc, zc;
				vec3d_t		color;

				xc = _UnitSnap(x,LIGHT_CELL_SIZE);
				yc = _UnitSnap(y,LIGHT_CELL_SIZE);
				zc = _UnitSnap(z,LIGHT_CELL_SIZE);
				
				Vec3dInit( color, 0, 0, 0 );

				if ( lighting_state == LightingState_facelight )
				{
					for ( p = lighting_emitface->patches; p ; p=p->next )
					{
						vec3d_t		v;
						
						Vec3dCopy( lighting_origin, p->center );
						
						v[0] = x+LIGHT_CELL_SIZE/2.0;
						v[1] = y+LIGHT_CELL_SIZE/2.0;
						v[2] = z+LIGHT_CELL_SIZE/2.0;
						
						Lighting_Volume_pointlight( v, color );
					}
				}
				
				else if ( lighting_state == LightingState_spotlight )
				{
					vec3d_t		v;
					
					v[0] = x+LIGHT_CELL_SIZE/2.0;
					v[1] = y+LIGHT_CELL_SIZE/2.0;
					v[2] = z+LIGHT_CELL_SIZE/2.0;
					
					Lighting_Volume_spotlight( v, color );	
				}

				if ( color[0] != 0.0 || color[1] != 0.0 || color[2] != 0.0 )
				{
					if ( !(c=Map3FindCell( map, xc, yc, zc ) ) )
					{
						c = NEW( veccell_t );
						c->x = xc;
						c->y = yc;
						c->z = zc;
						Vec3dCopy( c->vec, color );
						Map3InsertCell( map, c );
					}
					else
					{
						Vec3dAdd( c->vec, c->vec, color );
					}
				}
			}
		}
	}
}
