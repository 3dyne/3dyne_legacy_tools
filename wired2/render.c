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



// render.cc

#include <math.h>
#include <values.h>

#include "lib_token.h"
#include "shock.h"
#include "brush.h"
#include "render.h"
#include "texture.h"
#include "arr.h"
#include "tga.h"

#define FRUSTUM_CLIP_ON_EPSILON	( 0.001 )

// local functions


// local globals
static float	r_matrix[9];
static vec3d_t	r_origin;
static float	r_scale;
static float	r_invscale;

static float	r_width;
static float	r_width_2;
static float	r_height;
static float	r_height_2;

static plane_t	r_frustum[5];

static void	*r_scrptr = NULL;
static float	*r_zbuffer = NULL;

static unsigned char	r_flat_color_red;
static unsigned char	r_flat_color_green;
static unsigned char	r_flat_color_blue;


 // for R_RasterizeFace()
static int	t_pointnum;
static vec5d_t	t_points[256];

// span function
void R_NO_DrawSpan_Flat( int y, int xleft, float zleft, float uleft, float vleft,
			 int xright, float zright, float uright, float vright );

void R_NO_DrawSpanTGA888NoDepth( int y, int xleft, float zleft, float uleft, float vleft,
			   int xright, float zright, float uright, float vright );

void R_NO_DrawSpanTGA888( int y, int xleft, float zleft, float uleft, float vleft,
			   int xright, float zright, float uright, float vright );

void R_NO_DrawSpanRGBA( int y, int xleft, float zleft, float uleft, float vleft,
			int xright, float zright, float uright, float vright );

void (*span_func)( int y, int xleft, float zleft, float uleft, float vleft,
		   int xright, float zright, float uright, float vright );

// texture
static int	t_width;
static int	t_height;
static int	t_widthmask;
static int	t_heightmask;
static void	*t_data;

static char	*t_data_tgared;
static char	*t_data_tgagreen;
static char	*t_data_tgablue;

static unsigned char	*t_data_rgba;

// statistic
static int	stat_brushes;
static int	stat_faces;
static int	stat_cull;
static int	stat_fcull;
static int	stat_vis;

void R_CalcMatrix( float alpha, float beta, float gamma )
{
        float   alphasin, alphacos;
        float   betasin, betacos;
        float   gammasin, gammacos;
                                                                                
        alphasin = sin( alpha ); // six
	alphacos = cos( alpha ); // cox
        betasin  = sin( beta );  // siy
        betacos  = cos( beta );  // coy
        gammasin = sin( gamma ); // siz
        gammacos = cos( gamma ); // coz

        r_matrix[0] =   betacos * gammacos;
        r_matrix[1] = -(betacos * gammasin);
        r_matrix[2] =   betasin;
        r_matrix[3] =   alphasin * betasin * gammacos  -  alphacos * gammasin;
        r_matrix[4] = -(alphasin * betasin * gammasin  +  alphacos * gammacos);
        r_matrix[5] = -(alphasin * betacos);
        r_matrix[6] = -(alphacos * betasin * gammacos  +  alphasin * gammasin);
        r_matrix[7] =   alphacos * betasin * gammasin  -  alphasin * gammacos;
        r_matrix[8] =   alphacos * betacos;
}

void R_SetOrigin( vec3d_t origin )
{
	Vec3dCopy( r_origin, origin );
}

void R_SetZoom( float scale )
{
	r_scale = scale;
	r_invscale = 1.0/scale;
}

void R_SetView( float width, float height )
{
	int		size;

	if ( r_width == width && r_height == height )
		return;
	
	r_width = width;
	r_width_2 = width / 2.0;
	
	r_height = height;
	r_height_2 = height / 2.0;
	
	size = (int)(width*height);

	if ( r_zbuffer )
		free( r_zbuffer );
	r_zbuffer = (float*) malloc( size * sizeof(float) );
	printf(" alloc zbuffer at %p\n", r_zbuffer );
	
	R_CalcFrustum();
	
}

void Vec3dInitPlane_old( plane_t *p, vec3d_t v0, vec3d_t v1, vec3d_t v2 )           
{                                                                               
        vec3d_t         tv1, tv2;                                               
                                                                                
        Vec3dSub( tv1, v1, v0 );                                                
        Vec3dSub( tv2, v2, v0 );                                                
                                                                                
        Vec3dCrossProduct( p->norm, tv1, tv2 );                                 
        Vec3dUnify( p->norm );                                                  
                                                                                
        p->dist = Vec3dDotProduct( v0, p->norm );                               
}                                                                               


void R_CalcFrustum( void )
{
	vec3d_t	v[5];

	Vec3dInit( v[0], -r_width_2, -r_height_2, 0 );
        Vec3dInit( v[1],  r_width_2, -r_height_2, 0 );
        Vec3dInit( v[2],  r_width_2,  r_height_2, 0 );
        Vec3dInit( v[3], -r_width_2,  r_height_2, 0 );
        Vec3dInit( v[4],    0,    0, -r_width_2 ); 

	Vec3dInitPlane_old( &r_frustum[0], v[0], v[3], v[1] ); // z
        Vec3dInitPlane_old( &r_frustum[1], v[4], v[0], v[3] ); // left
        Vec3dInitPlane_old( &r_frustum[2], v[4], v[2], v[1] ); // right
        Vec3dInitPlane_old( &r_frustum[3], v[4], v[1], v[0] ); // top
        Vec3dInitPlane_old( &r_frustum[4], v[4], v[3], v[2] ); // bot
}

void R_Vec3dRot( vec3d_t out, vec3d_t in )
{
	vec3d_t		v;

	Vec3dSub( v, in, r_origin );
	Vec3dScale( v, r_scale, v );
	out[0] = v[0]*r_matrix[0] + v[1]*r_matrix[1] + v[2]*r_matrix[2];
        out[1] = v[0]*r_matrix[3] + v[1]*r_matrix[4] + v[2]*r_matrix[5];
	out[2] = v[0]*r_matrix[6] + v[1]*r_matrix[7] + v[2]*r_matrix[8];
}


// rotate for skybox
void R_Vec3dRotSky( vec3d_t out, vec3d_t in )
{
	vec3d_t		v;

	Vec3dCopy( v, in );
//	Vec3dSub( v, in, r_origin );
//	Vec3dScale( v, r_scale, v );
	out[0] = v[0]*r_matrix[0] + v[1]*r_matrix[1] + v[2]*r_matrix[2];
        out[1] = v[0]*r_matrix[3] + v[1]*r_matrix[4] + v[2]*r_matrix[5];
	out[2] = v[0]*r_matrix[6] + v[1]*r_matrix[7] + v[2]*r_matrix[8];
}

void R_Vec3dInverseRot( vec3d_t out, vec3d_t in )
{
	vec3d_t		v;
	
	v[0] = in[0]*r_matrix[0] + in[1]*r_matrix[3] + in[2]*r_matrix[6];
        v[1] = in[0]*r_matrix[1] + in[1]*r_matrix[4] + in[2]*r_matrix[7];
	v[2] = in[0]*r_matrix[2] + in[1]*r_matrix[5] + in[2]*r_matrix[8];

	Vec3dScale( v, r_invscale, v );
	
	Vec3dAdd( out, v, r_origin );
}

void R_Vec3dPer( vec3d_t out, vec3d_t in )
{
	float   scale;                                                          
                                                                                
        scale = 1 - in[2]/-r_width_2;                                                
        scale = 1 / scale;                                                      

        out[0] = in[0]*scale + (float)r_width_2;                                   
        out[1] = in[1]*scale + (float)r_height_2;                                  
        out[2] = scale;
}



int R_FrustumClipLine( vec3d_t from, vec3d_t to, int planenum )
{
	int	i;
	float	d, d0, d1;
	vec3d_t		mid;

	d0 = Vec3dDotProduct( from, r_frustum[planenum].norm ) - r_frustum[planenum].dist;
	d1 = Vec3dDotProduct( to, r_frustum[planenum].norm ) - r_frustum[planenum].dist;
	
	if ( d0 > FRUSTUM_CLIP_ON_EPSILON && d1 > FRUSTUM_CLIP_ON_EPSILON )
		return 0;
	if ( d0 <= FRUSTUM_CLIP_ON_EPSILON && d1 <= FRUSTUM_CLIP_ON_EPSILON )
		return 1;

	if ( d0 > FRUSTUM_CLIP_ON_EPSILON && d1 < FRUSTUM_CLIP_ON_EPSILON ) {
		// clip from
		d = d0 / ( d0 - d1 );
		for ( i=0; i<3; i++ ) {
			mid[i]=from[i] + d*(to[i]-from[i]);
		}
		Vec3dCopy( from, mid );
	}

	if ( d0 < FRUSTUM_CLIP_ON_EPSILON && d1 > FRUSTUM_CLIP_ON_EPSILON ) {
		// clip to
		d = d0 / ( d0 - d1 );
		for ( i=0; i<3; i++ ) {
			mid[i]=from[i] + d*(to[i]-from[i]);
		}
		Vec3dCopy( to, mid );
	}	
	return 2;
}

void R_DumpStat( void )
{
	printf("R_DumpStat:\n");
	printf(" stat_brushes = %d\n", stat_brushes );
	printf(" stat_faces = %d\n", stat_faces );
	printf(" stat_cull = %d\n", stat_cull );
	printf(" stat_fcull = %d\n", stat_fcull );
	printf(" stat_vis = %d\n", stat_vis );
}

void R_SetFrameBuffer( void *ptr, int depth )
{
	if ( depth != 32 )
		__error("only colordepth rendering supported yet. sorry.\n");
	
	r_scrptr = ptr;
}

void R_InitFrame( void )
{
	int	i;
	int	pixels;
	unsigned int	*pdata;

	stat_brushes = 0;
	stat_faces = 0;
	stat_cull = 0;
	stat_fcull = 0;
	stat_vis = 0;

	pdata = (unsigned int*) r_scrptr;
	pixels = (int) (r_width * r_height);
	
	for ( i = 0; i < pixels; i++ ) {
		
		(*pdata) = 0x00707070; // blue-green-red
//		(*pdata) = t_pal->rgb_set[i&255].red | (t_pal->rgb_set[i&255].green << 8) | (t_pal->rgb_set[i&255].blue << 16);
		pdata++;
	}

	memset( r_zbuffer, 0, pixels*4 );

}

void Vec5dInit( vec5d_t v, float a, float b, float c, float d, float e )        
{                                                                               
        v[0] = a;                                                               
        v[1] = b;                                                               
        v[2] = c;                                                               
        v[3] = d;                                                               
        v[4] = e;                                                               
}                                                                               


#define M       ( -256 )                                                        
#define P       ( 256 )                                                         
#define T       ( 256 )                                                         

void R_RenderSky( void )
{
	vec5d_t		cube[6][4];
	const char		*skyname[6];

	polygon_t	*p;
	tga_t		*tga;

	int		i,j;

	// left 
	Vec5dInit( cube[0][0], M, P, M, 0, 0 );                                 
        Vec5dInit( cube[0][1], M, P, P, T, 0 );                                 
        Vec5dInit( cube[0][2], M, M, P, T, T );                                 
        Vec5dInit( cube[0][3], M, M, M, 0, T );                                 
	skyname[0] = "sky/sky4_left";

                                                                                
        // right                                                                
        Vec5dInit( cube[1][0], P, P, P, 0, 0 );                                 
        Vec5dInit( cube[1][1], P, P, M, T, 0 );                                 
        Vec5dInit( cube[1][2], P, M, M, T, T );                                 
        Vec5dInit( cube[1][3], P, M, P, 0, T );                                 
	skyname[1] = "sky/sky4_right";                                                                          
      
        // front                                                                
        Vec5dInit( cube[2][0], M, P, P, 0, 0 );                                 
        Vec5dInit( cube[2][1], P, P, P, T, 0 );                                 
        Vec5dInit( cube[2][2], P, M, P, T, T );                                 
        Vec5dInit( cube[2][3], M, M, P, 0, T );                                 
	skyname[2] = "sky/sky4_front";                                                                                

        // back                                                                 
        Vec5dInit( cube[3][0], P, P, M, 0, 0 );                                 
        Vec5dInit( cube[3][1], M, P, M, T, 0 );                                 
        Vec5dInit( cube[3][2], M, M, M, T, T );                                 
        Vec5dInit( cube[3][3], P, M, M, 0, T );                                 
	skyname[3] = "sky/sky4_back";                                                                                
                                                                                
        // top                                                                  
        Vec5dInit( cube[4][0], M, P, M, 0, 0 );                                 
        Vec5dInit( cube[4][1], P, P, M, T, 0 );                                 
        Vec5dInit( cube[4][2], P, P, P, T, T );                                 
        Vec5dInit( cube[4][3], M, P, P, 0, T );                                 
	skyname[4] = "sky/sky4_top";                                                                                
                                                                                
        // bot                                                                  
        Vec5dInit( cube[5][0], M, M, P, 0, 0 );                                 
        Vec5dInit( cube[5][1], P, M, P, T, 0 );                                 
        Vec5dInit( cube[5][2], P, M, M, T, T );                                 
        Vec5dInit( cube[5][3], M, M, M, 0, T );                                 
	skyname[5] = "sky/sky4_bottom";

	span_func = R_NO_DrawSpanTGA888NoDepth;
	
	for ( i = 0; i < 6; i++ )
	{
		printf( "sky %d\n",i );

		p = NewPolygon( 4 );
		p->pointnum = 4;

		for ( j = 0; j < 4; j++ )
		{
			R_Vec3dRotSky( p->p[j], cube[i][j] );
			p->p[j][3] = cube[i][j][3];
			p->p[j][4] = cube[i][j][4];
		}

		for ( j = 0; j < 5; j++ )
		{
			p = ClipPolygonByPlane( p, &r_frustum[j] );
			if ( !p )
				break;
		}

		if ( !p )
			continue;

		// project
		t_pointnum = p->pointnum;
		for ( j = 0; j < t_pointnum; j++ ) {
			R_Vec3dPer( t_points[j], p->p[j] );
			t_points[j][3] = p->p[j][3]*t_points[j][2];
			t_points[j][4] = p->p[j][4]*t_points[j][2];
		}
		
		
		tga = T_GetTGA888ByName( skyname[i] );
		if (!tga) {
			printf("WARNING: R_RenderTextureFace can't get TGA888.\n");
			// fix me
		}
		else {
			
			t_width = tga->image_width;
			t_height = tga->image_height;
			t_data_tgared = (char *)tga->image.red;
			t_data_tgagreen = (char *)tga->image.green;
			t_data_tgablue = (char *)tga->image.blue;
			t_widthmask = t_width - 1;
			t_heightmask = t_height - 1;
			
			
			R_RasterizeFace();
		}
		
		FreePolygon( p );
	}   		       
}

void R_RenderTextureFrame( brush_t *brushes )
{
	brush_t		*b;

//	R_RenderSky();

	span_func = R_NO_DrawSpanRGBA; //R_NO_DrawSpan;

	for ( b = brushes; b ; b=b->next ) {

		if ( b->select&256 ) { // SELECT_VISIBLE = 256 ( can't include c++ )
			stat_brushes++;
			R_RenderTextureFace( b->faces );
		}
		
	}
	
}

void R_RenderDebugCSGFaces( const char *file )
{
	int		i, j;
	face_t		f;
	polygon_t	*p;

	tokenstream_t	*ts;

	span_func = R_NO_DrawSpan_Flat;

	ts = BeginTokenStream( file );

	if ( !ts )
	{
		printf( "R_RenderDebugCSGFaces: can't open debug-csg-faces.\n" );
		return;
	}

	p = NewPolygon( 32 );
	f.polygon = p;
	f.next = NULL;
	strcpy( f.texdef.ident, "default" );

	for( j = 0;; j++)
	{
		GetToken( ts );
		if ( !strcmp( ts->token, "end" ) )
			break;

		p->pointnum = atoi( ts->token );

		if ( p->pointnum > 32 )
		{
			printf( "pointnum > 32\n" );
			goto csgfaces_error;
		}

		for ( i = 0; i < p->pointnum; i++ )
		{
			GetToken( ts );
			p->p[i][0] = atof( ts->token );
			GetToken( ts );
			p->p[i][1] = atof( ts->token );
			GetToken( ts );
			p->p[i][2] = atof( ts->token );
			p->p[i][3] = 1.0;
			p->p[i][4] = 1.0;
		}

		Vec3dInitPlane_old( &f.plane, p->p[0], p->p[1], p->p[2] );

		r_flat_color_red = 123*j;
		r_flat_color_green = 321*j;
		r_flat_color_blue = 213*j;

		R_RenderTextureFace( &f );
	}

	EndTokenStream( ts );
	return;

csgfaces_error:
	EndTokenStream( ts );
	return;
	
}



/*
  ====================
  R_SetTexture

  ====================
*/
static texident_t		*r_cur_ti = NULL;

void	R_SetTexture( const char *ident )
{
	r_cur_ti = TexIdent_GetByIdent( ident );
}



/*
  ====================
  R_RenderPolygon

  ====================
*/
void	R_RenderPolygon( polygon_t *poly )
{
	
	fp_t		dist;
	vec3d_t		norm;
	polygon_t	*p;
	int		i;
	texident_t	*ti;
	
	if ( !r_cur_ti )
	{
		printf( "WARNING: no valid 'r_cur_arr' in R_RenderPolygon.\n" ); 
		return;
	}

	ti = r_cur_ti;

	if ( !ti->image )
	{
		printf( "WARNING: (null) image\n" );
		return;
	}
	
	Vec3dInitPlane( norm, &dist, poly->p[0], poly->p[1], poly->p[2] );
	
	// backface cull
//	if ( Vec3dDotProduct( r_origin, norm ) <= dist )
//		return;

	p = NewPolygon( poly->pointnum );
	p->pointnum = poly->pointnum;

	for ( i = 0; i < poly->pointnum; i++ ) {
		R_Vec3dRot( p->p[i], poly->p[i] );
		p->p[i][3] = poly->p[i][3];
		p->p[i][4] = poly->p[i][4];
	}

	// frustum clip
	for ( i = 0; i < 5; i++ ) {
		p = ClipPolygonByPlane( p, &r_frustum[i] );
		if (!p)
			break;
	}
	if (!p) {
		return;
	}
	
	t_pointnum = p->pointnum;
	for ( i = 0; i < t_pointnum; i++ ) {
		R_Vec3dPer( t_points[i], p->p[i] );
		t_points[i][3] = p->p[i][3]*t_points[i][2];
		t_points[i][4] = p->p[i][4]*t_points[i][2];
	}

	t_width = ti->width;
	t_height = ti->height;
	t_data_rgba = (unsigned char*) ti->image;
	t_widthmask = t_width - 1;
	t_heightmask = t_height - 1;	

	R_RasterizeFace();	

	FreePolygon( p );
}


void R_RenderTextureFace( face_t *faces )
{       
	int		i;
	face_t		*f;
//	float		d;
	polygon_t	*p;
//	tga_t		*tga;
	texident_t	*ti;

	for ( f = faces; f ; f=f->next ) {
		stat_faces++;

		// ignore all sky
//		if ( strstr( f->texdef.ident, "sky" ) )
//			continue;

		if ( Vec3dDotProduct( r_origin, f->plane.norm ) <= f->plane.dist ) 
		{
			stat_cull++;
			continue;
		}
		
		p = NewPolygon( f->polygon->pointnum );
		p->pointnum = f->polygon->pointnum;

		// rotate and copy polygon
		for ( i = 0; i < f->polygon->pointnum; i++ ) {
			R_Vec3dRot( p->p[i], f->polygon->p[i] );
			p->p[i][3] = f->polygon->p[i][3];
			p->p[i][4] = f->polygon->p[i][4];
		}

		// frustum clip
		for ( i = 0; i < 5; i++ ) {
			p = ClipPolygonByPlane( p, &r_frustum[i] );
			if (!p)
				break;
		}
		if (!p) {
			stat_fcull++;
			continue;
		}

		stat_vis++;		

		// project
		t_pointnum = p->pointnum;
		for ( i = 0; i < t_pointnum; i++ ) {
			R_Vec3dPer( t_points[i], p->p[i] );
			t_points[i][3] = p->p[i][3]*t_points[i][2];
			t_points[i][4] = p->p[i][4]*t_points[i][2];
		}



		
		ti = TexIdent_GetByIdent( f->texdef.ident );
		if (!ti) {
			printf("WARNING: R_RenderTextureFace can't get texture.\n");
			continue;
		}
		else if ( !ti->image )
		{
			printf( "WARNING: (null) image\n" );
			continue;
		}
		else {
			
			t_width = ti->width;
			t_height = ti->height;
			t_data_rgba = (unsigned char*) ti->image;
			t_widthmask = t_width - 1;
			t_heightmask = t_height - 1;	
			
			R_RasterizeFace();
//			R_LightFace( f );
		}
		
		FreePolygon( p );
	}       
}


void R_RasterizeFace( void )
{
	int		i;
	int		y;
	int		top, bot;
	int		ileft, iright;

	int		sy[256];
	float		height;

	float		xleft, xleftstep;
	float		xright, xrightstep;
	float		zleft, zleftstep;
	float		zright, zrightstep;

	float		u[2], ustep[2];
	float		v[2], vstep[2];
	
	top = MAXINT; // values.h
	bot = MININT;
	ileft = 0;

//	__named_message("\n"); //"top = %d, bot = %d\n", top, bot );
//	printf(" t_pointnum = %d\n", t_pointnum );
	for ( i = 0; i < t_pointnum; i++ ) {

		sy[i] = (int) t_points[i][1];
//		printf(" y = %d\n", y );

		if ( sy[i] < top ) {
			top = sy[i];
			ileft = i;
		}

		if ( sy[i] > bot )
			bot = sy[i];
	}
//	printf(" top = %d, ileft = %d, bot = %d \n", top, ileft, bot );

	if ( top >= bot )
		return;

	iright = ileft;

	xleft = t_points[ileft][0];
	xright = t_points[iright][0];

	xleftstep = 0;
	xrightstep = 0;

	zleft = t_points[ileft][2];
	zright = t_points[iright][2];

	zleftstep = 0;
	zrightstep = 0;

	u[0] = t_points[ileft][3];
	v[0] = t_points[ileft][4];

	u[1] = t_points[iright][3];
	v[1] = t_points[iright][4];

	ustep[0] = 0;
	ustep[1] = 0;
	vstep[0] = 0;
	vstep[1] = 0;

	y = top;
	while( y < bot ) {
		
		if ( y >= sy[ileft] ) {
//			printf(" ileft = %d\n", ileft );
			do {
				xleft = t_points[ileft][0];
				zleft = t_points[ileft][2];
				u[0] = t_points[ileft][3];
				v[0] = t_points[ileft][4];
//				printf("u = %f, v = %f\n", u[0], v[0] );
				ileft--;
				if ( ileft == -1 )
					ileft = t_pointnum - 1;
			} while( sy[ileft] <= y );
			height = sy[ileft] - y;
			xleftstep = ( t_points[ileft][0] - xleft ) / height;
			zleftstep = ( t_points[ileft][2] - zleft ) / height;
			ustep[0] = ( t_points[ileft][3] - u[0] ) / height;
			vstep[0] = ( t_points[ileft][4] - v[0] ) / height;
		}
			
		if ( y >= sy[iright] ) {
//			printf(" iright = %d\n", iright );
			do {
				xright = t_points[iright][0];
				zright = t_points[iright][2];
				u[1] = t_points[iright][3];
				v[1] = t_points[iright][4];
				iright++;
				if ( iright == t_pointnum )
					iright = 0;
			} while( sy[iright] <= y );
			height = sy[iright] - y;
			xrightstep = ( t_points[iright][0] - xright ) / height;
			zrightstep = ( t_points[iright][2] - zright ) / height;
			ustep[1] = ( t_points[iright][3] - u[1] ) / height;
			vstep[1] = ( t_points[iright][4] - v[1] ) / height;
		}

		span_func( y, (int)xleft, zleft, u[0], v[0], (int)xright, zright, u[1], v[1] );

		xleft += xleftstep;
		xright += xrightstep;
		zleft += zleftstep;
		zright += zrightstep;
		y++;

		u[0] += ustep[0];
		v[0] += vstep[0];
		u[1] += ustep[1];
		v[1] += vstep[1];			
	}
	

}

void R_NO_DrawSpan( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	int		txFix, tyFix;

	unsigned short		pixel;
	int		zbofs;

	unsigned int		r, g, b;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		if ( r_zbuffer[zbofs] <= z ) {
			scale = 1.0 / z;
			r_zbuffer[zbofs] = z;
			
			txFix = (int) (tx * scale );
			tyFix = (int) (ty * scale );
//		(*scr) = 0x00404040; //(* ((char*)t_data + (txFix&255) + ((tyFix&255)*256) ));
//			pixel = (unsigned int)(* ((unsigned char*)t_data + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));

			pixel = *(unsigned short*)(t_data + ((txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width))*2);
			r = pixel;
			r >>= 11;
			r &= 31;
			r <<= 3;
			g = pixel;
			g >>= 5;
			g &= 63;
			g <<= 2;
		 	b = pixel;
			b &= 31;
			b <<= 3;

			(*scr) = r | g<<8 | b<<16;

//			(*scr) = (t_pal->rgb_set[pixel].blue << 16) |
//				(t_pal->rgb_set[pixel].green << 8) |
//				(t_pal->rgb_set[pixel].red ); 
		}
		scr++;
		zbofs++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}

void R_NO_DrawSpan_Flat( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
//	int		txFix, tyFix;

//	unsigned short		pixel;
	int		zbofs;

	unsigned int		r, g, b;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		if ( r_zbuffer[zbofs] <= z ) {
			scale = 1.0 / z;
			r_zbuffer[zbofs] = z;
			
			r = r_flat_color_red;
			g = r_flat_color_green;
			b = r_flat_color_blue;

			(*scr) = r | g<<8 | b<<16;

//			(*scr) = (t_pal->rgb_set[pixel].blue << 16) |
//				(t_pal->rgb_set[pixel].green << 8) |
//				(t_pal->rgb_set[pixel].red ); 
		}
		scr++;
		zbofs++;
		z+=zstep;
	}
}

void R_NO_DrawSpanTGA888( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	int		txFix, tyFix;

	unsigned char		red,green,blue;
	int		zbofs;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		if ( r_zbuffer[zbofs] <= z ) 
		{
			scale = 1.0 / z;
			r_zbuffer[zbofs] = z;

			txFix = (int) (tx * scale );
			tyFix = (int) (ty * scale );
//		(*scr) = 0x00404040; //(* ((char*)t_data + (txFix&255) + ((tyFix&255)*256) ));
			red = (unsigned char)(* ((unsigned char*)t_data_tgared + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
			green = (unsigned char)(* ((unsigned char*)t_data_tgagreen + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
			blue = (unsigned char)(* ((unsigned char*)t_data_tgablue + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
			(*scr) = (blue << 16) |
				(green << 8) |
				(red ); 
			
		}
		scr++;
		zbofs++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}

void R_NO_DrawSpanRGBA( int y, int xleft, float zleft, float uleft, float vleft,
			int xright, float zright, float uright, float vright )
{
	
//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	int		txFix, tyFix;

	unsigned char		red,green,blue;
	int		zbofs;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		if ( r_zbuffer[zbofs] <= z ) 
		{
			scale = 1.0 / z;
			r_zbuffer[zbofs] = z;

			txFix = (int) (tx * scale );
			tyFix = (int) (ty * scale );
//		(*scr) = 0x00404040; //(* ((char*)t_data + (txFix&255) + ((tyFix&255)*256) ));
			red = (unsigned char)t_data_rgba[((txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width))*4];
			green = (unsigned char)t_data_rgba[((txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width))*4+1];
			blue = (unsigned char)t_data_rgba[((txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width))*4+2];

			(*scr) = (blue << 16) |
				(green << 8) |
				(red ); 
			
		}
		scr++;
		zbofs++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}


void R_NO_DrawSpanTGA888NoDepth( int y, int xleft, float zleft, float uleft, float vleft,
				 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	int		txFix, tyFix;

	unsigned char		red,green,blue;
	int		zbofs;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		scale = 1.0 / z;
		
		txFix = (int) (tx * scale );
		tyFix = (int) (ty * scale );
//		(*scr) = 0x00404040; //(* ((char*)t_data + (txFix&255) + ((tyFix&255)*256) ));
		red = (unsigned char)(* ((unsigned char*)t_data_tgared + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
		green = (unsigned char)(* ((unsigned char*)t_data_tgagreen + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
		blue = (unsigned char)(* ((unsigned char*)t_data_tgablue + (txFix&t_widthmask) + ((tyFix&t_heightmask)*t_width) ));
		(*scr) = (blue << 16) |
			(green << 8) |
			(red ); 

		scr++;
		zbofs++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}


//
// alpha light stuff
//

typedef union {
	unsigned char	ch[4];
	unsigned int	dw;
} size32_u;

vec3d_t		l_origin = { -640, -320, -640 };
float		l_red = 1.0;
float		l_green = 0.5;
float		l_blue = 0.0;

float		l_val = 0.5;

vec3d_t		l_texorg; 

vec3d_t		l_pnorm;
vec3d_t		l_ldist;

void R_LightFace( face_t *f )
{
	float		d;
	vec3d_t		lvec;
	vec3d_t		xaxis, yaxis;
	
	float		angle, sinv, cosv, s, t, ns, nt;

	d = Vec3dDotProduct( l_origin, f->plane.norm ) - f->plane.dist;

	if ( d <= 0 )
	{
		printf(" backface culled light.\n");
		return;
	}

	// fusspunkt lvec 
	printf(" d = %f\n", d );
	Vec3dScale( lvec, -d, f->plane.norm );
	Vec3dAdd( lvec, lvec, l_origin );
	Vec3dPrint( lvec );

	// auf texture 
	TextureAxisFromPlane( &f->plane, xaxis, yaxis );

	angle = f->texdef.rotate / 180 * M_PI;
	sinv = sin( angle );
	cosv = cos( angle );

	s = Vec3dDotProduct( lvec, xaxis );
	t = Vec3dDotProduct( lvec, yaxis );

	ns = cosv*s - sinv*t;
	nt = sinv*s + cosv*t;
	
	l_texorg[0] = ns/f->texdef.scale[0] + f->texdef.shift[0];
	l_texorg[1] = nt/f->texdef.scale[1] + f->texdef.shift[1];	
	l_texorg[2] = -d;	
	
	R_RasterizeLight();
}

void R_RasterizeLight( void )
{
	int		i;
	int		y;
	int		top, bot;
	int		ileft, iright;

	int		sy[8];
	float		height;

	float		xleft, xleftstep;
	float		xright, xrightstep;
	float		zleft, zleftstep;
	float		zright, zrightstep;

	float		u[2], ustep[2];
	float		v[2], vstep[2];
	
	top = MAXINT; // values.h
	bot = MININT;
	ileft = 0;

//	__named_message("\n"); //"top = %d, bot = %d\n", top, bot );
//	printf(" t_pointnum = %d\n", t_pointnum );
	for ( i = 0; i < t_pointnum; i++ ) {

		sy[i] = (int) t_points[i][1];
//		printf(" y = %d\n", y );

		if ( sy[i] < top ) {
			top = sy[i];
			ileft = i;
		}

		if ( sy[i] > bot )
			bot = sy[i];
	}
//	printf(" top = %d, ileft = %d, bot = %d \n", top, ileft, bot );

	iright = ileft;

	xleft = t_points[ileft][0];
	xright = t_points[iright][0];

	xleftstep = 0;
	xrightstep = 0;

	zleft = t_points[ileft][2];
	zright = t_points[iright][2];

	zleftstep = 0;
	zrightstep = 0;

	u[0] = t_points[ileft][3];
	v[0] = t_points[ileft][4];

	u[1] = t_points[iright][3];
	v[1] = t_points[iright][4];

	ustep[0] = 0;
	ustep[1] = 0;
	vstep[0] = 0;
	vstep[1] = 0;

	y = top;
	while( y < bot ) {
		
		if ( y >= sy[ileft] ) {
//			printf(" ileft = %d\n", ileft );
			do {
				xleft = t_points[ileft][0];
				zleft = t_points[ileft][2];
				u[0] = t_points[ileft][3];
				v[0] = t_points[ileft][4];
//				printf("u = %f, v = %f\n", u[0], v[0] );
				ileft--;
				if ( ileft == -1 )
					ileft = t_pointnum - 1;
			} while( sy[ileft] <= y );
			height = sy[ileft] - y;
			xleftstep = ( t_points[ileft][0] - xleft ) / height;
			zleftstep = ( t_points[ileft][2] - zleft ) / height;
			ustep[0] = ( t_points[ileft][3] - u[0] ) / height;
			vstep[0] = ( t_points[ileft][4] - v[0] ) / height;
		}
			
		if ( y >= sy[iright] ) {
//			printf(" iright = %d\n", iright );
			do {
				xright = t_points[iright][0];
				zright = t_points[iright][2];
				u[1] = t_points[iright][3];
				v[1] = t_points[iright][4];
				iright++;
				if ( iright == t_pointnum )
					iright = 0;
			} while( sy[iright] <= y );
			height = sy[iright] - y;
			xrightstep = ( t_points[iright][0] - xright ) / height;
			zrightstep = ( t_points[iright][2] - zright ) / height;
			ustep[1] = ( t_points[iright][3] - u[1] ) / height;
			vstep[1] = ( t_points[iright][4] - v[1] ) / height;
		}

		R_NO_LightSpan( y, (int)xleft, zleft, u[0], v[0], (int)xright, zright, u[1], v[1] );

		xleft += xleftstep;
		xright += xrightstep;
		zleft += zleftstep;
		zright += zrightstep;
		y++;

		u[0] += ustep[0];
		v[0] += vstep[0];
		u[1] += ustep[1];
		v[1] += vstep[1];			
	}
	

}

void R_NO_LightSpan( int y, int xleft, float zleft, float uleft, float vleft,
		 int xright, float zright, float uright, float vright )
{

//
// Draw span without optimization
//
	int		x;
	int		width;
	unsigned int		*scr;

	float		z, zstep;
	float		scale;

	float tx, ty;
	float sx, sy;
	
	vec3d_t		ray;
	float		lx, ly;
	float		len;

//	int		pixel;
	int		zbofs;

	int		add;
	size32_u	u;
	int		red,green,blue;

//	printf(" xleft = %d, xright = %d, y = %d\n", xleft, xright, y );

	zbofs = (y*(int)(r_width) + xleft);
	scr = (unsigned int*)(r_scrptr) + zbofs;
	
//	printf(" r_scrptr = %p, scr = %p\n", r_scrptr, scr );

	width = xright - xleft;
	
	z = zleft;
	zstep = ( zright - zleft ) / width;

	tx = uleft;
	ty = vleft;

	sx = ( uright - uleft ) / width;
	sy = ( vright - vleft ) / width;

	for ( x = xleft; x < xright; x++ ) {

		if ( r_zbuffer[zbofs] == z ) {
			scale = 1.0 / z;
//			r_zbuffer[zbofs] = z;
			
			lx = ( tx * scale );
			ly = ( ty * scale );

			ray[0] = lx - l_texorg[0];
			ray[1] = ly - l_texorg[1];
			ray[2] = 0 - l_texorg[2];

//			Vec3dUnify( ray );
			len = Vec3dLen( ray );

//			add = len;

			// only draw lights near enough
			if ( len < 256 )
			{

				len = 255-len;
				len *= l_val;
				add = (int)(len);

				u.dw = (*scr);
				
				red = (int)(u.ch[0] + add*l_red);
				green = (int)(u.ch[1] + add*l_green);
				blue = (int)(u.ch[2] + add*l_blue);
				
				if ( red > 255 )
					red = 255;
				if ( green > 255 )
					green = 255;
				if ( blue > 255 )
					blue = 255;
				
				u.ch[0] = red;
				u.ch[1] = green;
				u.ch[2] = blue;
				
				(*scr) = u.dw;
			}
		}
		scr++;
		zbofs++;
		tx+=sx;
		ty+=sy;
		z+=zstep;
	}
}
