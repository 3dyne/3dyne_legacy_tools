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



// w2p.c

// =============================================================================

#include "w2p.h"
//#include "../shared/process.h"


int		p_planenum = 0;
plane_t		p_planes[MAX_PLANES];

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

	Vec3dFlip( p_planes[p_planenum+1].norm, norm );
	p_planes[p_planenum+1].dist = -dist;

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
			return i;				   
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
	plane_t		*pl;
	wtexdef_t	td;
	vec2d_t		axis[2] = { {1, 0}, {0, 1} };
	fp_t		angle;
	fp_t		angle_sin, angle_cos;


	memset( &td, 0, sizeof( wtexdef_t ) );

	//
	// find projection
	//
	pl = &p_planes[plane];
	if ( pl->type <= PLANE_Z )
		td.flags = pl->type;
	else if ( pl->type >= PLANE_ANYX )
		td.flags = pl->type - PLANE_ANYX;

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

	td.texture = FindTexture( ident );
	//
	// search texdef
	//
	
	for ( i = 0; i < p_wtexdefnum; i++ )
		if ( !memcmp( &p_wtexdefs[i], &td, sizeof( wtexdef_t ) ) )
			return i;
	
	
	if ( p_wtexdefnum == MAX_WTEXDEFS )
		Error( "FindTexdef: reached MAX_TEXDEFS.\n" );

	memcpy( &p_wtexdefs[p_wtexdefnum], &td, sizeof( wtexdef_t ) );
	p_wtexdefnum++;
	
	return p_wtexdefnum-1;
}

/*
  ====================
  LoadSBrush

  ====================
*/
void LoadSBrushes( char *name )
{
	tokenstream_t		*ts;

	vec3d_t		norm;
	fp_t		dist;

	int		pl; // FindPlane

	// texdef
 	char		ident[TEXTURE_IDENT_SIZE];
	fp_t		rotate;
	vec2d_t		shift;
	vec2d_t		scale;

	printf( "LoadSBrushes: loading %s ...\n", name );

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( " can't open file.\n" );

	GetToken( ts );
	if ( strcmp( ts->token, "WireType" ) )
		Error( "token 'WireType' not found.\n" );

	GetToken( ts );
//	if ( strcmp( ts->token, WIRETYPE_VERSION ) )
//		Error( "'WireType' wrong version.\n" );

	// expect '{' of world
	GetToken( ts );
	if ( ts->token[0] != '{' )
		Error( "expected '{' of world.\n" );

	//
	// brush loop
	//
	
	for(;;)
	{

		GetToken( ts );
		if ( ts->token[0] == '}' )	// wwm end
			break;
		if ( ts->token[0] != '{' )	// brush start
			Error( "expected '{' of brush start.\n" );

		//
		// begin new wbrush
		//
		if ( p_wbrushnum == MAX_WBRUSHES )
			Error( "LoadSBrushes: reached MAX_WBRUSHES.\n" );

		p_wbrushes[p_wbrushnum].firstsurface = p_wsurfacenum;
		
		GetToken( ts );
		if ( ts->token[0] =='*'  )
		{
			p_wbrushes[p_wbrushnum].id = (unique_t) atoi( &ts->token[1] );
		}
		else
		{
			p_wbrushes[p_wbrushnum].id = UNIQUE_INVALIDE;
			KeepToken( ts );
		}
		
		GetToken( ts );
		if ( ts->token[0] == 'b' )
		{
			// brush contents
			p_wbrushes[p_wbrushnum].contents = atoi( &ts->token[1] );
			switch( p_wbrushes[p_wbrushnum].contents )
			{
			case BRUSH_CONTENTS_SOLID:
				stat_wbrush_solidnum++;
				break;
			case BRUSH_CONTENTS_LIQUID:
				stat_wbrush_liquidnum++;
				break;
			case BRUSH_CONTENTS_DECO:
				stat_wbrush_deconum++;
				break;
			case BRUSH_CONTENTS_HINT:
				stat_wbrush_hintnum++;
				break;
			default:
				Error( "LoadSBrushes: unkowen brush contents" );
			}
		}
		else
		{
			KeepToken( ts );
			printf( "LoadSBrushes: no brush contents set, use solid.\n" );
			p_wbrushes[p_wbrushnum].contents = BRUSH_CONTENTS_SOLID;
		}


		// face loop

		for(;;)
		{
			GetToken( ts );


			if ( ts->token[0] == '}' ) // brush end
				break;
			if ( ts->token[0] != '{' ) // face start
				Error( "expected '{' of face start.\n" );

			if ( p_wsurfacenum == MAX_WSURFACES )
				Error( "LoadSBrushes: reached MAX_WSURFACES.\n" );

			// plane norm start '('
			GetToken( ts );
			if ( ts->token[0] != '(' )
				Error( "expected '(' of plane norm start.\n" );

			// x norm 'float'
			GetToken( ts );
			norm[0] = atof( ts->token );
			// y norm 'float'
			GetToken( ts );
			norm[1] = atof( ts->token );
			// z norm 'float'
			GetToken( ts );
			norm[2] = atof( ts->token );

			// plane norm end ')'
			GetToken( ts );
			if ( ts->token[0] != ')' )
				Error( "expected ')' of plane norm end.\n" );
			
			// plane dist 'float'
			GetToken( ts );
			dist = atof( ts->token );

			// string 'ident'
			GetToken( ts );
			strcpy( ident, ts->token );

			// rotate 'float'
			GetToken( ts );
			rotate = atof( ts->token );

			// x scale 'float'
			GetToken( ts );
			scale[0] = atof( ts->token );

			// y scale 'float'
			GetToken( ts );
			scale[1] = atof( ts->token );

			// x shift 'float'
			GetToken( ts );
			shift[0] = atof( ts->token );

			// y shift 'float'
			GetToken( ts );
			shift[1] = atof( ts->token );

			GetToken( ts );
			if ( ts->token[0] == 's' )
			{
				// surface contents
				p_wsurfaces[p_wsurfacenum].contents = atoi( &ts->token[1] );
			}
			else
			{
				KeepToken( ts );
				printf( "LoadSBrushes: no surface contents set, use close+texture.\n" );
				p_wsurfaces[p_wsurfacenum].contents=SURFACE_CONTENTS_CLOSE|SURFACE_CONTENTS_TEXTURE;
			}
						
	
			// face end '}'
			GetToken( ts );
			if ( ts->token[0] != '}' )
				Error( "expected '}' of face end.\n" );

			//
			// finish surface
			// 

			pl = FindPlane( norm, dist );
			p_wsurfaces[p_wsurfacenum].plane = pl;

			p_wsurfaces[p_wsurfacenum].texdef = FindTexdef( ident, rotate, scale, shift, pl );
			p_wsurfacenum++;
		}

		//
		// finish brush
		//

		p_wbrushes[p_wbrushnum].surfacenum = p_wsurfacenum - p_wbrushes[p_wbrushnum].firstsurface;

		if ( p_wbrushes[p_wbrushnum].surfacenum < 4 )
		{
			printf( " * only %d faces in brush id: %d, ignored *\n", 
				p_wbrushes[p_wbrushnum].surfacenum,
				p_wbrushes[p_wbrushnum].id );
			
			p_wsurfacenum = p_wbrushes[p_wbrushnum].firstsurface;
		}
		else
		{
			p_wbrushnum++;
		}
	}		
	EndTokenStream( ts );

}

#if 0
/*
  ====================
  Write_Planes

  ====================
*/
void Write_Planes( char *name )
{
	FILE		*h;
	int		i;

	printf( "Write_Planes ...\n" );

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_Planes: can't open file '%s'\n", name );

	fprintf( h, "# plane file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <plane num> <norm> <dist> <type>\n" );

	for ( i = 0; i < p_planenum; i++ )
	{
		fprintf( h, "%d ", i );
		Write_Vec3d( h, p_planes[i].norm );
		Write_fp( h, p_planes[i].dist );
		fprintf( h, "%u\n", p_planes[i].type );
	}

	fprintf( h, "end\n" );

	fclose( h );
}

/*
  ====================
  Write_WTextures

  ====================
*/
void Write_WTextures( char *name )
{
	FILE		*h;
	int		i;

	printf( "Write_WTextures ...\n" );
	
	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file '%s'\n", name );

	fprintf( h, "# texture file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <texture num> <texture ident>\n" );

	for ( i = 0; i < p_wtexturenum; i++ )
	{
		fprintf( h, "%d %s\n", i, p_wtextures[i].ident );
	}
	fprintf( h, "end\n" );

	fclose( h );
}
#endif

/*
  ====================
  Write_WTexdefs

  ====================
*/
void Write_WTexdefs( char *name )
{
	FILE		*h;
	int		i;


	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file '%s'\n", name );

	fprintf( h, "# texdef file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <texdef num> { <texture> <flags> <vec2d_t vec[2]> <vec2d_t shift> } \n" );

	for ( i = 0; i < p_wtexdefnum; i++ )
	{
		fprintf( h, "%d { %d %u ", i, 
			 p_wtexdefs[i].texture,
			 p_wtexdefs[i].flags );
		Write_Vec2d( h, p_wtexdefs[i].vec[0] );
		Write_Vec2d( h, p_wtexdefs[i].vec[1] );
		Write_Vec2d( h, p_wtexdefs[i].shift );
		fprintf( h, "}\n" );
	}
	fprintf( h, "end\n" );

	fclose( h );
}

/*
  ====================
  Write_WSurfaces

  ====================
*/
void Write_WSurfaces( char *name )
{
	FILE		*h;
	int		i;


	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file '%s'\n", name );

	fprintf( h, "# wsurface file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <surface num> <plane> <contents> <texdef>\n" );
	
	for ( i = 0; i < p_wsurfacenum; i++ )
	{
		fprintf( h, "%d %d %u %d\n", i,
			 p_wsurfaces[i].plane,
			 p_wsurfaces[i].contents,
			 p_wsurfaces[i].texdef );
	}
	
	fprintf( h, "end\n" );
	fclose( h );
}

/*
  ====================
  Write_WBrushes

  ====================
*/

void Write_WBrushes( char *name )
{
	FILE		*h;
	int			i;


	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file '%s'\n", name );

	fprintf( h, "# wbrush file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <wbrush num> <unique id> <contents> <first wsurface> <wsurfacenum> \n" );
	
	for ( i = 0; i < p_wbrushnum; i++ )
	{
		fprintf( h, "%d %d %u %d %d\n", i,
			 p_wbrushes[i].id,
			 p_wbrushes[i].contents,
			 p_wbrushes[i].firstsurface,
			 p_wbrushes[i].surfacenum );
	}

	fprintf( h, "end\n" );

	fclose( h );
}

void Write_InitialBspbrushes( char *name )
{
	FILE		*h;
	int		i, j;

//	printf( "Write_InitialBspbrushes ...\n" );

	h = fopen( name, "w" );
	if ( !h )
		Error( "can't open file '%s'\n", name );

	fprintf( h, "# initial bspbrush file\n" );
	fprintf( h, "# generated by w2p !!! DON'T EDIT !!!\n" );
	fprintf( h, "# <bspbrush num> <wbrushnum> <contents> <surfacenum> [ <plane> <contents> <state> ]\n" );
	
	for ( i = 0; i < p_wbrushnum; i++ )
	{
		fprintf( h, "%d %d %u %d\n", i, i,
			 p_wbrushes[i].contents,					// brush contents
			 p_wbrushes[i].surfacenum );					// number of surfaces
		for ( j = 0; j < p_wbrushes[i].surfacenum; j++ )
		{
			fprintf( h, " %d %u %u\n", 
				 p_wsurfaces[p_wbrushes[i].firstsurface+j].plane,	// surface plane
				 p_wsurfaces[p_wbrushes[i].firstsurface+j].contents,	// surface contents
				 0 );							// surface state
		}
	}

	fprintf( h, "end\n" );
	fclose( h );
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
	printf( "===== w2p - preprocess wired sbrush file =====\n" );

	SetCmdArgs( argc, argv );


	if ( argc == 2 )
	{
		if ( !strcmp( argv[1], "-h" ) || !strcmp( argv[1], "--help" ) )
		{
			PrintHelp();
			exit(-1);
		}
	}
	
	LoadSBrushes( "sbrush" );

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

	printf( " planes: %d, %d are axial\n", p_planenum, stat_axialplanes );

	printf( " textures: %d\n", p_wtexturenum );
	printf( " texdefs: %d\n", p_wtexdefnum );

#if 0
	printf( " - WArcheType Statistic -\n" );
	printf( " archetypes: %d\n", stat_warche_num );
	printf( " pairs: %d\n", stat_wkvpair_num );	
#endif
}
