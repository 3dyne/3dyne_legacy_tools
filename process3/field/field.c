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



// field.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"

#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )
#define LINK( x, head ) ( x->next = head, head = x )

typedef enum
{
	SingularType_source,
	SingularType_sink
} singularType;

typedef struct singularity_s
{
	vec3d_t		pos;
	singularType	type;
	fp_t		value;

	struct singularity_s	*next;
} singularity_t;

typedef struct homogen_s
{
	vec3d_t		norm;
	fp_t		dist;

	struct homogen_s	*next;
} homogen_t;

typedef struct fieldpoint_s
{
	vec3d_t		pos;
	vec3d_t		vec;

	struct fieldpoint_s	*next;
} fieldpoint_t;

singularity_t * NewSingularity( fp_t x, fp_t y, fp_t z, fp_t value, singularType type )
{
	singularity_t	*s;

	s = NEW( singularity_t );
	s->pos[0] = x;
	s->pos[1] = y;
	s->pos[2] = z;
	s->value = value;
	s->type = type;

	return s;
}

homogen_t * NewHomogen( fp_t x, fp_t y, fp_t z, fp_t dist, fp_t value )
{
	homogen_t	*h;

	h = NEW( homogen_t );
	h->norm[0] = x;
	h->norm[1] = y;
	h->norm[2] = z;
	h->dist = dist;

	return h;
}

fieldpoint_t * NewFieldPoint( fp_t x, fp_t y, fp_t z )
{
	fieldpoint_t	*f;

	f = NEW( fieldpoint_t );
	f->pos[0] = x;
	f->pos[1] = y;
	f->pos[2] = z;

	return f;
}

void FieldPointsToStdout( fieldpoint_t *list )
{
	fieldpoint_t		*f;

	for ( f = list; f ; f=f->next )
	{
		printf( "%f %f %f %f %f %f\n", f->pos[0], f->pos[1], f->pos[2],
			f->vec[0], f->vec[1], f->vec[2] );
	}
}

void FieldPointsToBinary( fieldpoint_t *list )
{
	fieldpoint_t		*f;
	int		num;
	FILE		*h;

	// count
	for ( f = list, num = 0; f ; f=f->next, num++ )
	{ }
		
	h = fopen( "_field.bin", "w" );
	if ( !h )
		Error( "can't write field binary.\n" );

	// write fieldpoint num
	fwrite( &num, 4, 1, h );
	
	for ( f = list; f ; f=f->next )
	{
		short		s;
		
		s = (short)rint(f->pos[0]);
		fwrite( &s, 2, 1, h );
		s = (short)rint(f->pos[1]);
		fwrite( &s, 2, 1, h );
		s = (short)rint(f->pos[2]);
		fwrite( &s, 2, 1, h );
		
		fwrite( &f->vec[0], 4, 1, h );
		fwrite( &f->vec[1], 4, 1, h );
		fwrite( &f->vec[2], 4, 1, h );		
	}

	fclose( h );
}

void FieldPointsToBinary2( char *name, fieldpoint_t *list, ivec3d_t min, ivec3d_t max )
{
	fieldpoint_t		*f;
	int		num;
	FILE		*h;
	char		tt[256];

	// count
	for ( f = list, num = 0; f ; f=f->next, num++ )
	{ }
		
	h = fopen( name, "w" );
	if ( !h )
		Error( "can't write field binary.\n" );

	// write fieldpoint num
	fwrite( &num, 4, 1, h );

	// write bounds
	fwrite( &min[0], 4, 1, h );
	fwrite( &min[1], 4, 1, h );
	fwrite( &min[2], 4, 1, h );
	fwrite( &max[0], 4, 1, h );
	fwrite( &max[1], 4, 1, h );
	fwrite( &max[2], 4, 1, h );

	for ( f = list; f ; f=f->next )
	{
		short		s;
		
		s = (short)rint(f->pos[0]);
		fwrite( &s, 2, 1, h );
		s = (short)rint(f->pos[1]);
		fwrite( &s, 2, 1, h );
		s = (short)rint(f->pos[2]);
		fwrite( &s, 2, 1, h );
		
		fwrite( &f->vec[0], 4, 1, h );
		fwrite( &f->vec[1], 4, 1, h );
		fwrite( &f->vec[2], 4, 1, h );		
	}

	fclose( h );
}


void CalcFieldPointVector( fieldpoint_t *f, singularity_t *sgu, homogen_t *hgn )
{
	singularity_t	*s;
	homogen_t		*h;
	vec3d_t		vec;
	fp_t		len;
	fp_t		len_square;
	fp_t		scale;

	for ( s = sgu; s ; s=s->next )
	{
		if ( s->pos[0] == f->pos[0] &&
		     s->pos[1] == f->pos[1] &&
		     s->pos[2] == f->pos[2] )
			continue;

		if ( s->type == SingularType_source )
		{
			// vector goes source to point
			Vec3dSub( vec, f->pos, s->pos );
		}
		else
		{
			// vector goes point to sink
			Vec3dSub( vec, s->pos, f->pos );
		}
		
		Vec3dScale( vec, s->value, vec );
		len = Vec3dLen( vec );	       

		len_square = len* len;
		scale = 1.0 / len_square;

		Vec3dScale( vec, scale, vec );	     
		
		Vec3dAdd( f->vec, f->vec, vec );
	}

	for ( h = hgn; h ; h=h->next )
	{
		fp_t		d;

		d = Vec3dDotProduct( f->pos, h->norm ) - h->dist;
		if ( d <= 0.0 )
			continue;

		len_square = d * d;
		scale = 1.0 / len_square;

		Vec3dScale( vec, scale, h->norm );
		Vec3dAdd( f->vec, f->vec, vec );
	}

	Vec3dUnify( f->vec );
}


int main( int argc, char *argv[] )
{
	char		*in_field_name;
	char		*out_field_name;
	
	fieldpoint_t	*pts;
	fieldpoint_t	*f;
				
	singularity_t	*sgu;

	homogen_t	*hgn;

	int		x, y, z;
	int		pointnum;
	int		sgunum;
	int		hgnnum;

	hmanager_t	*fieldhm;
	hobj_t		*root;
	hobj_t		*obj;
	hpair_t		*pair;
	hobj_search_iterator_t		iter;

	vec3d_t		v;
	ivec3d_t	min = { 0, 0, 0 };
	ivec3d_t	max = { 3, 7, 3 };

	
	SetCmdArgs( argc, argv );

	in_field_name = GetCmdOpt2( "-i" );
	out_field_name = GetCmdOpt2( "-o" );

	if ( !in_field_name )
	{
		Error( "no input field class.\n" );
	}

	if ( !out_field_name )
	{
		Error( "no output field name.\n" );
	}

	fieldhm = NewHManagerLoadClass( in_field_name );
	if ( !fieldhm )
		Error( "can't open input class.\n" );
	root = HManagerGetRootClass( fieldhm );
	
	//
	// setup fieldpoints
	//
	
	obj = FindClassType( root, "fieldpoints" );
	if ( !obj )
		Error( "missing class 'fieldpoints'.\n" );

	// get min
	pair = FindHPair( obj, "min" );
	if ( !pair )
		Error( "missing 'min' in class '%s'.\n", obj->name );
	HPairCastToVec3d_safe( v, pair );
	min[0] = (int)rint(v[0]);
	min[1] = (int)rint(v[1]);
	min[2] = (int)rint(v[2]);
		
	// get max
	pair = FindHPair( obj, "max" );
	if ( !pair )
		Error( "missing 'max' in class '%s'.\n", obj->name );
	HPairCastToVec3d_safe( v, pair );
	max[0] = (int)rint(v[0]);
	max[1] = (int)rint(v[1]);
	max[2] = (int)rint(v[2]);		
	
	// create fieldpoints 
	pts = NULL;
	pointnum = 0;
	for ( x = min[0]; x <=max[0]; x++ )
	{
		for ( y = min[1]; y <=max[1]; y++ )
		{
			for ( z = min[2]; z <= max[2] ; z++ )
			{
				f = NewFieldPoint( (fp_t)x, (fp_t)y, (fp_t)z );
				LINK( f, pts );
				pointnum++;
			}
		}
	}
	printf( "field bounds ( %d %d %d ) - ( %d %d %d ), %d fieldpoints\n", min[0], min[1], min[2], max[0], max[1], max[2], pointnum );


	//
	// setup singularities
	//

	InitClassSearchIterator( &iter, root, "singularity" );
	sgu = NULL;
	sgunum = 0;
	for ( ; ( obj = SearchGetNextClass( &iter ) ) ; )
	{
		singularity_t	*s;		
		vec3d_t		origin;
		singularType	type = 0;
		fp_t		value;

		pair = FindHPair( obj, "origin" );
		if ( !pair )
			Error( "missing 'origin' in singularity '%s'.\n", obj->name );
		HPairCastToVec3d_safe( origin, pair );

		pair = FindHPair( obj, "type" );
		if ( !pair )
			Error( "missing 'type' in singularity '%s'.\n", obj->name );
		if ( !strcmp( "sink", pair->value ) )
		{
			type = SingularType_sink;
		}
		else if ( !strcmp( "source", pair->value ) )
		{
			type = SingularType_source;
		}
		else
		{
			Error( "unkown singularity type in '%s'.\n", obj->name );
		}

		pair = FindHPair( obj, "value" );
		if ( !pair )
			Error( "missing 'value' in singularity '%s'.\n", obj->name );
		HPairCastToFloat_safe( &value, pair );


		
		s = NewSingularity( origin[0], origin[1], origin[2], value, type );
		LINK( s, sgu );
		sgunum++;
	}

	printf( " %d singularities\n", sgunum );

	//
	// setup homogens
	//

	hgn = NULL;
	hgnnum = 0;
	InitClassSearchIterator( &iter, root, "homogen" );
	for( ; ( obj = SearchGetNextClass( &iter ) ) ; )
	{
		vec3d_t		norm;
		fp_t		dist;
		fp_t		value;
		homogen_t	*h;

		pair = FindHPair( obj, "norm" );
		if ( !pair )
			Error( "missing 'norm' in homogen '%s'.\n", obj->name );
		HPairCastToVec3d_safe( norm, pair );

		pair = FindHPair( obj, "dist" );
		if ( !pair )
			Error( "missing 'dist' in homogen '%s'.\n", obj->name );
		HPairCastToFloat_safe( &dist, pair );

		pair = FindHPair( obj, "value" );
		if ( !pair )
			Error( "missing 'value in homogen '%s'.\n", obj->name );
		HPairCastToFloat_safe( &value, pair );
		
		h = NewHomogen( norm[0], norm[1], norm[2], dist, value );
		LINK( h, hgn );
		hgnnum++;
	}

	printf( " %d homogens\n", hgnnum );

	//
	// calc fieldpoints
	//

	for ( f = pts; f ; f=f->next )
	{
		CalcFieldPointVector( f, sgu, hgn );
	}	

//	FieldPointsToStdout( pts );
//	FieldPointsToBinary( pts );
	FieldPointsToBinary2( out_field_name, pts, min, max );
}
