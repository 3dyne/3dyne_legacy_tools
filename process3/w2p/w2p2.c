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



// w2p2.c

// =============================================================================

#include "w2p2.h"
//#include "../shared/process.h"


int		p_planenum = 0;
wplane_t		p_planes[MAX_PLANES];

int		p_wtexturenum = 0;
wtexture_t	p_wtextures[MAX_WTEXTURES];

int		p_wtexdefnum = 0;
wtexdef_t	p_wtexdefs[MAX_WTEXDEFS];

int		p_wbrushnum = 0;
int		p_wsurfacenum = 0;
wsurface_t	p_wsurfaces[MAX_WSURFACES];
wbrush_t	p_wbrushes[MAX_WBRUSHES];

// some statistic ...
static int	stat_wbrush_solidnum = 0;
static int	stat_wbrush_liquidnum = 0;
static int	stat_wbrush_deconum = 0;
static int	stat_wbrush_hintnum = 0;

static int	stat_warche_num = 0;
static int	stat_wkvpair_num = 0;

static int	stat_axialplanes = 0;

static int		stat_cpoly_hack = 0;

/*
  ====================
  TypeOfNormal

  ====================
*/
int TypeOfNormal( vec3d_t norm )
{
	int		i;
	vec3d_t		absnorm;

	if ( norm[0] == 1.0 || norm[0] == -1.0 )
		return PLANE_X;
	if ( norm[1] == 1.0 || norm[1] == -1.0 )
		return PLANE_Y;
	if ( norm[2] == 1.0 || norm[2] == -1.0 )
		return PLANE_Z;

	for ( i = 0; i < 3; i++ )
		absnorm[i] = fabs( norm[i] );

	if ( absnorm[0] >= absnorm[1] && absnorm[0] >= absnorm[2] )
		return PLANE_ANYX;
	else if ( absnorm[1] >= absnorm[0] && absnorm[1] >= absnorm[2] )
		return PLANE_ANYY;
	else if ( absnorm[2] >= absnorm[0] && absnorm[2] >= absnorm[1] )
		return PLANE_ANYZ;

	Error( "TypeOfNormal: can't happen.\n" );
	return -1;
}

/*
  ====================
  ArePlanesEqual

  ====================
*/
bool_t ArePlanesEqual( vec3d_t norm1, fp_t dist1,
		       vec3d_t norm2, fp_t dist2 )
{
	if ( fabs( norm1[0] - norm2[0] ) < PLANE_NORM_EPSILON &&
	     fabs( norm1[1] - norm2[1] ) < PLANE_NORM_EPSILON &&
	     fabs( norm1[2] - norm2[2] ) < PLANE_NORM_EPSILON &&
	     fabs( dist1 - dist2 ) < PLANE_DIST_EPSILON )
		return true;
	return false;
}

bool_t ArePlanesNearlyEqual( vec3d_t norm1, fp_t dist1,
		             vec3d_t norm2, fp_t dist2 )
{
        if ( fabs( norm1[0] - norm2[0] ) < 5*PLANE_NORM_EPSILON &&
             fabs( norm1[1] - norm2[1] ) < 5*PLANE_NORM_EPSILON &&
             fabs( norm1[2] - norm2[2] ) < 5*PLANE_NORM_EPSILON &&
             fabs( dist1 - dist2 ) < 5*PLANE_DIST_EPSILON )
                return true;
        return false;
}

/*
  ====================
  AddPlane

  ====================
*/
unsigned int AddPlane( vec3d_t norm, fp_t dist )
{
	int		type;
	bool_t		swap;

	if ( p_planenum+2 >= MAX_PLANES )
		Error( "AddPlane: reached MAX_PPLANES.\n" );
	
	type = TypeOfNormal( norm );
	
	swap = false;
	if ( type <= PLANE_Z )
	{
		// axial plane
		stat_axialplanes++;
		if ( norm[type] < 0.0 )
		{
			norm[type] = -norm[type];
			dist = -dist;
			swap = true;
		}
	}
	else
	{
		// non-axial plane
		if ( norm[type - PLANE_ANYX] < 0.0 )
		{
			Vec3dFlip( norm, norm );
			dist =- dist;
			swap = true;
		}
	}

	p_planes[p_planenum].type = p_planes[p_planenum+1].type = type;

	Vec3dCopy( p_planes[p_planenum].norm, norm );
	p_planes[p_planenum].dist = dist;	
	p_planes[p_planenum].clsname = HManagerGetFreeID();
	p_planes[p_planenum].type |= PLANE_POS;
	p_planes[p_planenum].planes = NULL;

	Vec3dFlip( p_planes[p_planenum+1].norm, norm );
	p_planes[p_planenum+1].dist = -dist;
	p_planes[p_planenum+1].clsname = HManagerGetFreeID();
	p_planes[p_planenum+1].planes = NULL;

	// link planes
	p_planes[p_planenum].clsref_flipplane = p_planes[p_planenum+1].clsname;
	p_planes[p_planenum+1].clsref_flipplane = p_planes[p_planenum].clsname;

	p_planenum += 2;

	if ( swap )
		return p_planenum-1;
	else
		return p_planenum-2;
}

/*
  ====================
  FindPlane

  ====================
*/ 
void SnapPlane( vec3d_t norm, fp_t *dist )
{
	int		i;

	for ( i = 0; i < 3; i++ )
	{
		if ( fabs( norm[i]-1 ) < PLANE_NORM_EPSILON )
		{
			Vec3dInit( norm, 0.0, 0.0, 0.0 );
			norm[i] = 1.0;
			break;
		}
		if ( fabs( norm[i]+1 ) < PLANE_NORM_EPSILON )
		{
			Vec3dInit( norm, 0.0, 0.0, 0.0 );
			norm[i] = -1.0;
			break;
		}
	}
	
	if ( fabs( *dist-((float)((int)((*dist)+0.5))) ) < PLANE_DIST_EPSILON )
		*dist=((float)((int)((*dist)+0.5)));

}

unsigned int FindPlane( vec3d_t norm, fp_t dist )
{
	int		i;	

	SnapPlane( norm, &dist );

 	for ( i = 0; i < p_planenum; i++ )
	{
		if ( ArePlanesEqual( p_planes[i].norm, p_planes[i].dist, norm, dist ) )
		{
			aplane_t	*pl;
			pl = (aplane_t *) malloc( sizeof( aplane_t) );
			Vec3dCopy( pl->norm, norm );
			pl->dist = dist;
			pl->next = p_planes[i].planes;
			p_planes[i].planes = pl;
			return i;				   
		}
#if 0
		else if ( ArePlanesNearlyEqual( p_planes[i].norm, p_planes[i].dist, norm, dist ) )
			printf( " * FindPlane: planes are nearly equal. *\n" );
#endif
	 
	}

	return AddPlane( norm, dist );
}

/*
  ====================
  FindTexture
  
  ====================
*/
int FindTexture( char *ident )
{
	int		i;
	
	for ( i = 0; i < p_wtexturenum; i++ )
		if ( !strcmp( ident, p_wtextures[i].ident ) )
			return i;

	if ( p_wtexturenum == MAX_WTEXTURES )
		Error( "FindTexture: reached MAX_WTEXTURES.\n" );

	memset( p_wtextures[p_wtexturenum].ident, 0, TEXTURE_IDENT_SIZE );
	strcpy( p_wtextures[p_wtexturenum].ident, ident );
	p_wtextures[p_wtexturenum].clsname = HManagerGetFreeID();

	p_wtexturenum++;

	return p_wtexturenum-1;
}


/*
  ====================
  FindTexdef

  ====================
*/

int FindTexdef( char *ident, fp_t rotate, vec2d_t scale, vec2d_t shift, int plane )
{
	int		i;
	int		type;
	wplane_t		*pl;
	wtexdef_t	td;
	vec2d_t		axis[2] = { {1, 0}, {0, 1} };
	fp_t		angle;
	fp_t		angle_sin, angle_cos;


	memset( &td, 0, sizeof( wtexdef_t ) );

	td.rotate = rotate;
	Vec2dCopy( td.scale, scale );

	//
	// find projection
	//
	pl = &p_planes[plane];
	type = pl->type & PLANE_AXIS_MASK;
	if ( type <= PLANE_Z )
		td.flags = type;
	else if ( type >= PLANE_ANYX )
		td.flags = type - PLANE_ANYX;

	//
	// rotate
	//
	if ( rotate != 0.0 || scale[0] != 1.0 || scale[1] != 1.0 )
	{
		angle = rotate / 180*M_PI;
		angle_sin = sin( angle );
		angle_cos = cos( angle );

		for( i = 0; i < 2; i++ )
		{
			td.vec[i][0] = angle_cos*axis[i][0] - angle_sin*axis[i][1];
			td.vec[i][1] = angle_sin*axis[i][0] + angle_cos*axis[i][1];

			td.vec[i][0] /= scale[0];
			td.vec[i][1] /= scale[1];
		}
			       
		td.flags |= PROJECT_VEC;
	}

	// 
	// shift
	//

	if ( shift[0] != 0.0 || shift[1] != 0.0 )
	{
		td.shift[0] = shift[0];
		td.shift[1] = shift[1];

		td.flags |= PROJECT_SHIFT;
	}
	else
	{
		td.shift[0] = 0.0;
		td.shift[1] = 0.0;
	}

	td.clsref_texture = p_wtextures[FindTexture( ident )].clsname;

	//
	// search texdef
	//
	
	for ( i = 0; i < p_wtexdefnum; i++ )
	{
		if ( p_wtexdefs[i].clsref_texture == td.clsref_texture &&
		     p_wtexdefs[i].flags == td.flags &&
		     p_wtexdefs[i].vec[0][0] == td.vec[0][0] &&
		     p_wtexdefs[i].vec[0][1] == td.vec[0][1] &&
		     p_wtexdefs[i].vec[1][0] == td.vec[1][0] &&
		     p_wtexdefs[i].vec[1][1] == td.vec[1][1] &&
		     p_wtexdefs[i].shift[0] == td.shift[0] &&
		     p_wtexdefs[i].shift[1] == td.shift[1] )
		{
			return i;
		}
 	}

	if ( p_wtexdefnum == MAX_WTEXDEFS )
		Error( "FindTexdef: reached MAX_TEXDEFS.\n" );

	memcpy( &p_wtexdefs[p_wtexdefnum], &td, sizeof( wtexdef_t ) );
	p_wtexdefs[p_wtexdefnum].clsname = HManagerGetFreeID();
	p_wtexdefnum++;
	
	return p_wtexdefnum-1;
}

/*
  ====================
  ConvertBrushClass

  ====================
*/
static int total_faces;

static int	pump_brushes = 0;

hobj_t * ConvertBrushClass( hobj_t *brushes, hobj_t *brush )
{
	hpair_t		*pair;
	hobj_search_iterator_t	iter;
	hobj_t		*face;
	int		num;

	// brush data
	int		bcontent;

	// face data
	vec3d_t		norm, keep_norm;
	fp_t		dist, keep_dist;
	int		fcontent;
	char		ident[TEXTURE_IDENT_SIZE];
	fp_t		rotate;
	vec2d_t		scale;
	vec2d_t		shift;

	bool_t		no_contents;
	bool_t		no_texdef;

	int	pl;
	int	td;

	hobj_t	*tmp;
	hobj_t	*out;
	char		idtext[256];
	
	// brush contents
	pair = FindHPair( brush, "content" );
	if ( !pair )
		Error( "missing brush content.\n" );
	HPairCastToInt_safe( &bcontent, pair );

// hack moved to the script ...
#if 0
	// hack: fix some design leaks ...
	if ( bcontent == 6 )
		bcontent = 4;
#endif

	sprintf( idtext, "#%u", HManagerGetFreeID() );
	out = NewClass( "bspbrush", idtext );

	sprintf( idtext, "%u", bcontent );
	pair = NewHPair2( "int", "content", idtext );
	InsertHPair( out, pair );

	pair = NewHPair2( "ref", "original", brush->name );
	InsertHPair( out, pair );

	

	// brush faces
	InitClassSearchIterator( &iter, brush, "face" );

	for ( num = 0; ( face = SearchGetNextClass( &iter ) ) ; num++ )
	{
		//
		// face plane
		//

		// plane dist
		pair = FindHPair( face, "dist" );
		if ( !pair )
			Error( "missing face plane distance.\n" );
		HPairCastToFloat_safe( &dist, pair );
		keep_dist = dist;	// keep dist, cause FindPlane trash it up

		// plane norm
		pair = FindHPair( face, "norm" );
		if ( !pair )
			Error( "missing face plane normal.\n" );
		HPairCastToVec3d_safe( norm, pair );
		Vec3dCopy( keep_norm, norm );	// keep norm, dto

		//
		// face content
		//
		pair = FindHPair( face, "content" );
		if ( !pair )
		{
//			Error( "missing face content.\n" );
			no_contents = true;
		}
		else
		{
			HPairCastToInt_safe( &fcontent, pair );
			no_contents = false;
		}

		//
		// face texdef
		//

		// texture ident
		pair = FindHPair( face, "ident" );
		if ( !pair )
		{
			no_texdef = true;
		}
		else
		{
			HPairCastToString_safe( ident, pair );
			no_texdef = false;
			

			// texdef rotate
			pair = FindHPair( face, "rotate" );
			if ( !pair )
				Error( "missing face texdef rotate.\n" );
			HPairCastToFloat_safe( &rotate, pair );
			
			// texdef shift
			pair = FindHPair( face, "shift" );
			if  ( !pair )
				Error( "missing face texdef shift.\n" );
			HPairCastToVec2d_safe( shift, pair );
			
			// texdef scale
			pair = FindHPair( face, "scale" );
			if ( !pair )
				Error( "missing face texdef scale.\n" );
			HPairCastToVec2d_safe( scale, pair );
		}

		//
		// finish face
		//
		pl = FindPlane( norm, dist );


		sprintf( idtext, "#%u", HManagerGetFreeID() );
		tmp = NewClass( "surface", idtext );
		
		// clsref_plane
		sprintf( idtext, "#%u", p_planes[pl].clsname );
		pair = NewHPair2( "ref", "plane", idtext );
		InsertHPair( tmp, pair );

		if ( !no_texdef )
		{
			// clsref_texdef
			td = FindTexdef( ident, rotate, scale, shift, pl );
			sprintf( idtext, "#%u", p_wtexdefs[td].clsname );
			pair = NewHPair2( "ref", "texdef", idtext );
			InsertHPair( tmp, pair );

			// special cpoly hack
			if ( !strcmp( ident, "devel/cpoly" ))
			{
				pair = NewHPair2( "int", "bsp_dont_split", "1" );
				InsertHPair( tmp, pair );
				stat_cpoly_hack++;
			}
		}

		if ( !no_contents )
		{
			// content
			sprintf( idtext, "%d", fcontent );
			pair = NewHPair2( "int", "content", idtext );
			InsertHPair( tmp, pair );
		}

		// insert surface into bspbrush
		InsertClass( out, tmp );



	}


	total_faces+=num;

//	return out;
	InsertClass( brushes, out );


	return out;
}

/*
  ====================
  ConvertSBrushes

  ====================
*/
void ConvertSBrushes( hmanager_t *hm, char *name )
{
	FILE	*h;
	hobj_t		*sbrushcls;
	hobj_search_iterator_t	iter;
	hobj_t		*brush;
	int		num;
	hobj_t		*bspbrushcls;


	bspbrushcls = NewClass( "bspbrushes", "bspbrushes0" );

	sbrushcls = HManagerGetRootClass( hm );
	if ( !sbrushcls )
		Error( "can't find class 'sbrush'\n" );

	InitClassSearchIterator( &iter, sbrushcls, "brush" );
	
	total_faces = 0;
	for ( num = 0; ( brush = SearchGetNextClass( &iter ) ) ; num++ )
	{
		ConvertBrushClass( bspbrushcls, brush );
	}
	printf( " ok\n" );
	printf( "brushes: %d\n", num ); 
	printf( "faces: %d\n", total_faces );
	printf( "generated pump brushes: %d\n", pump_brushes );

	h = fopen( name, "w" );
	WriteClass( bspbrushcls, h );
	fclose( h );

}





/*
  ====================
  WritePlaneClass

  ====================
*/
void WritePlaneClass( char *name )
{
	FILE		*h;
	int		i;

	hobj_t		*planecls;
	hobj_t		*tmp;
	hpair_t		*pair;
	char		idtext[256];

#if 0
	printf( "averraging planes ...\n" );
	for ( i = 0; i < p_planenum; i+=2 )
	{
		int		num;
		vec3d_t		norm;
		fp_t		dist;
		aplane_t	*pl;
		Vec3dCopy( norm, p_planes[i].norm );
		dist = p_planes[i].dist;

		for ( num = 1, pl = p_planes[i].planes; pl ; pl=pl->next, num++ )
		{
			Vec3dAdd( norm, pl->norm, norm );
			dist+=pl->dist;
		}
		Vec3dUnify2( p_planes[i].norm, norm );
		p_planes[i].dist = dist / ((fp_t)(num));

		SnapPlane( p_planes[i].norm, &p_planes[i].dist );
		Vec3dFlip( p_planes[i+1].norm, p_planes[i].norm );
		p_planes[i+1].dist = -p_planes[i].dist;
	}
#endif	

	planecls = NewClass( "planes", "planes0" );

	for ( i = 0; i < p_planenum; i++ )
	{
		sprintf( idtext, "#%u", p_planes[i].clsname );
		tmp = NewClass( "plane", idtext );
		
		sprintf( idtext, "%f %f %f", p_planes[i].norm[0], p_planes[i].norm[1], p_planes[i].norm[2] );
		pair = NewHPair2( "vec3d", "norm", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%f", p_planes[i].dist );
		pair = NewHPair2( "float", "dist", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%d", p_planes[i].type );
		pair = NewHPair2( "int", "type", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "#%u", p_planes[i].clsref_flipplane );
		pair = NewHPair2( "ref", "flipplane", idtext );
		InsertHPair( tmp, pair );

		InsertClass( planecls, tmp );
	}

	h = fopen( name, "w" );
	WriteClass( planecls, h );
	fclose( h );
}

/*
  ====================
  WriteTexdefClass

  ====================
*/
void WriteTexdefClass( char *name )
{
	FILE		*h;
	int		i;

	hobj_t		*texdefcls;
	hobj_t		*tmp;
	hpair_t		*pair;
	char		idtext[256];

	texdefcls = NewClass( "texdefs", "texdefs0" );

	for ( i = 0; i < p_wtexdefnum; i++ )
	{
		sprintf( idtext, "#%u", p_wtexdefs[i].clsname );
		tmp = NewClass( "texdef", idtext );

		sprintf( idtext, "#%u", p_wtexdefs[i].clsref_texture );
		pair = NewHPair2( "ref", "texture", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%u", p_wtexdefs[i].flags );
		pair = NewHPair2( "int", "flags", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%f %f", p_wtexdefs[i].vec[0][0], p_wtexdefs[i].vec[0][1] );
		pair = NewHPair2( "vec2d", "vec0", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%f %f", p_wtexdefs[i].vec[1][0], p_wtexdefs[i].vec[1][1] );
		pair = NewHPair2( "vec2d", "vec1", idtext );
		InsertHPair( tmp, pair );

		sprintf( idtext, "%f %f", p_wtexdefs[i].shift[0], p_wtexdefs[i].shift[1] );
		pair = NewHPair2( "vec2d", "shift", idtext );
		InsertHPair( tmp, pair );

		EasyNewVec2d( tmp, "scale", p_wtexdefs[i].scale );
		EasyNewFloat( tmp, "rotate", p_wtexdefs[i].rotate );
		
		InsertClass( texdefcls, tmp );
	}

	h = fopen( name, "w" );
	WriteClass( texdefcls, h );
	fclose( h );
}

/*
  ====================
  WriteTextureClass

  ====================
*/
void WriteTextureClass( char *name )
{
	FILE		*h;
	int		i;

	hobj_t		*texturecls;
	hobj_t		*tmp;
	hpair_t		*pair;
	char		idtext[256];

	texturecls = NewClass( "textures", "textures0" );

	for ( i = 0; i < p_wtexturenum; i++ )	
	{
		sprintf( idtext, "#%u", p_wtextures[i].clsname );
		tmp = NewClass( "texture", idtext );	

		sprintf( idtext, "%s", p_wtextures[i].ident );
		pair = NewHPair2( "string", "ident", idtext );
		InsertHPair( tmp, pair );

		InsertClass( texturecls, tmp );
	}

	h = fopen( name, "w" );
	WriteClass( texturecls, h );
	fclose( h );
}


/*
  ====================
  MakeBigBox

  ====================
*/
void MakeBigBox( fp_t size )
{
	vec3d_t		norm;
	fp_t		dist;
	int		i;
	hobj_t		*brushes;
	hobj_t		*brush;
	hobj_t		*surface;
	int		plane;
	hpair_t		*pair;
	FILE		*h;

	brushes = NewClass( "bspbrushes", "bigbox0" );

	brush = NewClass( "bspbrush", "bigbox" );
	InsertClass( brushes, brush );

	pair = NewHPair2( "ref", "original", "null" );
	InsertHPair( brush, pair );
	pair = NewHPair2( "int" , "content", "0" );
	InsertHPair( brush, pair );

	for ( i = 0; i < 3; i++ )
	{
		char	tt[256];
		surface = NewClass( "surface", "noname" );
		InsertClass( brush, surface );
		pair = NewHPair2( "int", "content", "0" );
		InsertHPair( surface, pair );
		Vec3dInit( norm, 0.0, 0.0, 0.0 );
		norm[i] = 1.0;
		dist = size; //tree->max[i] + 64.0;
		plane = FindPlane( norm, dist );
		sprintf( tt, "#%d", p_planes[plane].clsname );
		pair = NewHPair2( "ref", "plane", tt );
		InsertHPair( surface, pair );

		surface = NewClass( "surface", "noname" );
		InsertClass( brush, surface );
		pair = NewHPair2( "int", "content", "0" );
		InsertHPair( surface, pair );
		Vec3dInit( norm, 0.0, 0.0, 0.0 );
		norm[i] = -1.0;
		dist = size; //- (tree->min[i] - 64.0);
		plane = FindPlane( norm, dist );
		sprintf( tt, "#%d", p_planes[plane].clsname );
		pair = NewHPair2( "ref", "plane", tt );
		InsertHPair( surface, pair );

	}	

	h = fopen( "_bigbox.hobj", "w" );
	if ( !h )
		Error( "can't write bigbox class.\n" );
	WriteClass( brushes, h );
	fclose( h );
}

/*
  ==============================
  AddPlaneClasses

  ==============================
*/
void AddPlaneClasses( hmanager_t *planehm )
{
	hobj_search_iterator_t		iter;
	hobj_t		*plane;

	InitClassSearchIterator( &iter, HManagerGetRootClass( planehm ), "*" );
	for ( ; ( plane = SearchGetNextClass( &iter ) ) ; )
	{
		vec3d_t		norm;
		fp_t		dist;
		int		planenum;
		hpair_t		*pair;
		char		tt[256];

		EasyFindVec3d( norm, plane, "norm" );
		EasyFindFloat( &dist, plane, "dist" );

		planenum = FindPlane( norm, dist );
//		printf( "%d ", planenum );
		sprintf( tt, "#%d", p_planes[planenum].clsname );
		pair = NewHPair2( "ref", "plane", tt );
		InsertHPair( plane, pair );
	}
}

void PrintHelp( void )
{
	puts( "usage:\n" );
	puts( "input:" );
	puts( " sbrush \t- Wired output file of static brushes (fixed name)" );
	puts( "output:" );
	puts( " planes.asc \t- Plane file" );
	puts( " textures.asc \t- Texture file" );
	puts( " texdefs.asc \t- Texdef file" );
	puts( " wsurfaces.asc \t- Surfaces of the original brushes" );
	puts( " wbrushes.asc \t- Original brushes" );
	puts( " initbspbrushes.asc \t- Original brushes in bspbrush format, input for csg/bsp" );
}

int main( int argc, char *argv[] )
{
	char			*in_name;
	char			*out_name_prefix;
	char			*in_plane_name;

	tokenstream_t		*ts;
	hobj_t			*sbrushcls;
	hmanager_t		*hm;

	fp_t			pump_brush_dist;
	unsigned int		pump_brush_contents;

	printf( "===== w2p2 - preprocess wired sbrush class =====\n" );

	SetCmdArgs( argc, argv );

	if ( argc == 2 )
	{
		if ( !strcmp( argv[1], "-h" ) || !strcmp( argv[1], "--help" ) )
		{
			PrintHelp();
			exit(-1);
		}
	}


	in_name = GetCmdOpt2( "-i" );
	if ( !in_name )
	{
		in_name = "sbrush.hobj";
		printf( " default input brush class: %s\n", in_name );
	}
	else
	{
		printf( " input brush class: %s\n", in_name );
	}

	printf( "in_name: %s\n", in_name );
	ts = BeginTokenStream( in_name ); 
	sbrushcls = ReadClass( ts );
	EndTokenStream( ts );
//	DeepDumpClass( sbrushcls );

	hm = NewHManager();
	HManagerSetRootClass( hm, sbrushcls );
	HManagerRebuildHash( hm );
	if ( ! HManagerCheckClassConsistancy( hm ) )
	{
		Error( "please fix class !\n" );
	}
//	DumpHManager( hm, false );


	fprintf( stderr, "building _bspbrush.hobj ..." );


	ConvertSBrushes( hm, "_bspbrush.hobj" );
	printf( "add BIG_BOX planes ...\n" );
	MakeBigBox( BIG_BOX );	// shared/defs.h

	in_plane_name = GetCmdOpt2( "-ipl" );
	if ( in_plane_name )
	{
		hmanager_t	*inplanehm;
		FILE		*h;
		if ( ! (inplanehm = NewHManagerLoadClass( in_plane_name ) ) )
			Error( "inplane load failed\n" );
		printf( "do additional planes ...\n" );
		AddPlaneClasses( inplanehm );
		h = fopen( in_plane_name, "w" );
		if ( !h )
			Error( "can't write inplane\n" );
		WriteClass( HManagerGetRootClass( inplanehm ), h );
		fclose( h );
	}

#if 0
	printf( " write files ...\n" );
	printf( "  planes.asc\n" );
	Write_Planes( p_planes, p_planenum, "planes.asc", "w2p" );
	printf( "  textures.asc\n" );
	Write_WTextures( p_wtextures, p_wtexturenum, "textures.asc", "w2p" );
	printf( "  texdefs.asc\n" );
	Write_WTexdefs( "texdefs.asc" );
	printf( "  wsurfaces.asc\n" );
	Write_WSurfaces( "wsurfaces.asc" );
	printf( "  wbrushes.asc\n" );
	Write_WBrushes( "wbrushes.asc" );
	printf( "  initbspbrushes.asc\n" );
	Write_InitialBspbrushes( "initbspbrushes.asc" );

	printf( " - WBrush Statistic -\n" );
	printf( " %d solid, ", stat_wbrush_solidnum );
	printf( "%d liquid, ", stat_wbrush_liquidnum );
	printf( "%d deco, ", stat_wbrush_deconum );
	printf( "%d hint\n", stat_wbrush_hintnum );

#endif

 	printf( " planes: %d, %d are axial\n", p_planenum, stat_axialplanes );

	printf( " textures: %d\n", p_wtexturenum );
	printf( " texdefs: %d\n", p_wtexdefnum );
	printf( " %d surfaces are set to 'bsp_dont_split'\n", stat_cpoly_hack );       

	printf( "_plane.hobj\n" );
	WritePlaneClass( "_plane.hobj" );
	printf( "_texdef.hobj\n" );
	WriteTexdefClass( "_texdef.hobj" );
	printf( "_texture.hobj\n" );
	WriteTextureClass( "_texture.hobj" );

	HManagerSaveID();

#if 0
	printf( " - WArcheType Statistic -\n" );
	printf( " archetypes: %d\n", stat_warche_num );
	printf( " pairs: %d\n", stat_wkvpair_num );	
#endif
}
