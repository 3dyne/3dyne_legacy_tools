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



// field_gl.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <sys/time.h>
#include <unistd.h>
                                                                                
#include "cmdpars.h"
#include "wire.h"
#include "lib_token.h"
#include "lib_error.h"
#include "lib_math.h"
#include "lib_poly.h"
#include "lib_unique.h"
#include "lib_hobj.h"
#include "tga.h"

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include "r_math.h"

#define RND	( (random()%1000)/1000.0 )
#define DEG2RAD	( M_PI/180.0 )

#define	SIZE_X		( 1024 )
#define SIZE_Y		( 768 )

int	r_argc;
char	**r_argv;

vec3d_t	r_flatcolors[256];
vec3d_t		r_origin;
matrix3_t	r_matrix;
matrix3_t	r_invmatrix;
fp_t		r_speed = 0;
fp_t	r_yaw = 0.0;
fp_t	r_roll = 0.0;
fp_t	r_pitch = 0.0;

bool_t	key_pressmap[256];

void SetupFieldMap( char *name );
void RunParticle( void );

void GlutKeyPress( unsigned char key, int x, int y )
{
	key_pressmap[key] = true;
}

void GlutKeyRelease( unsigned char key, int x, int y )
{
	key_pressmap[key] = false;
}

void HandlePressedKeys(void )
{

//	c = cos( r_pitch * (M_PI/180.0) ) * 10.0;
//	s = sin( r_pitch * (M_PI/180.0) ) * 10.0;

	if ( key_pressmap[27] )
		exit(-1);

	if ( key_pressmap['w'] )
	{
		r_speed+=5.0; //r_speed*2.5;
		if ( r_speed > 50.0 )
			r_speed = 50.0;
	}

	if ( key_pressmap['z'] )
	{
		r_speed-=5.0;
		if ( r_speed < -50.0 )
			r_speed = -50.0;
	}

	if ( key_pressmap['j'] )
	{
		r_speed=2.0; //r_speed*2.5;
	}

	if ( key_pressmap['n'] )
	{
		r_speed-=2.0;
	}


	if ( key_pressmap['e'] )
	{
		r_origin[1] += 5.0;
	}
	if ( key_pressmap['d'] )
	{
		r_origin[1] -= 5.0;
	}
	if ( key_pressmap['x'] )
	{
		r_yaw -= 5.0;
		if ( r_yaw < 0.0 )
			r_yaw+=360.0;
	}
	if ( key_pressmap['c'] )
	{
		r_yaw += 5.0;
		if ( r_yaw > 360.0 )
			r_yaw-=360.0;
	}

	if ( key_pressmap['f'] )
	{
		r_pitch -= 5.0;
		if ( r_pitch < 0.0 )
			r_pitch+=360.0;
	}
	if ( key_pressmap['r'] )
	{
		r_pitch += 5.0;
		if ( r_pitch > 360.0 )
			r_pitch-=360.0;
	}

	if ( key_pressmap['a'] )
	{
		r_yaw -= 5.0;
		if ( r_yaw < 0.0 )
			r_yaw+=360.0;
	}
	if ( key_pressmap['s'] )
	{
		r_yaw += 5.0;
		if ( r_yaw > 360.0 )
			r_yaw-=360.0;
	}
}

void GlutResize( int width, int height )
{
	glutPostRedisplay();
}

void CalcVertex( vec4d_t out, vec3d_t in )
{
        vec3d_t         tmp; 
	Vec3dScale( tmp, 1.0/16.0, r_origin );
        Vec3dSub( tmp, in, tmp /*r_origin*/ );
        Matrix3Vec3dRotate( tmp, tmp, r_matrix );
        out[0] = tmp[0];
        out[1] = tmp[1]*1.33;
        out[3] = 1.0 + tmp[2] / 1.0;
        out[2] = 1.0; // !!!
}

int GetMilliSec( void )
{
	int		msec;
	struct timeval	tv;

	gettimeofday( &tv, NULL );
	msec = tv.tv_usec / 1000;
	msec+= tv.tv_sec * 1000;
	
	return msec;
}


void GlutDisplay( void )
{
	int		i, j, k;
	int		time1, time2;

	time1 = GetMilliSec();
	
	glClearDepth( 0.0 );
	glClearColor( 0.3, 0.3, 0.3, 0.0);
	glClearColor( 0.0, 0.0, 0.0, 0.0);
	glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//	glPolygonMode( GL_FRONT, GL_LINE );
	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );


	RunParticle();

	glFlush();
	glutSwapBuffers();

	time2 = GetMilliSec();
	fprintf( stderr, "f/s: %f ", 1000.0/(time2-time1) );
}


void GlutIdle( void )
{
	fp_t	c, s;

	HandlePressedKeys();

	c = cos( r_yaw * (M_PI/180.0) ) * r_speed;
	s = sin( r_yaw * (M_PI/180.0) ) * r_speed;
	r_origin[2] += c;
	r_origin[0] += s;
	r_speed *= 0.9;
	if ( fabs(r_speed) < 0.1 )
		r_speed = 0.0;

	Matrix3SetupRotate( r_matrix, r_roll*DEG2RAD, r_pitch*DEG2RAD, r_yaw*DEG2RAD );
	Matrix3Transpose( r_invmatrix, r_matrix );

	GlutDisplay();

	usleep( 10000 );
}


void InitGL( void )
{
	glEnable(GL_CULL_FACE);
	glFrontFace( GL_CW );
	glHint( GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST );

	glEnable(GL_DEPTH_TEST);
	glDepthMask(GL_TRUE);	
	glDepthFunc( GL_GEQUAL );
	glDepthRange( 0.0, 1.0 );	

	glViewport( 0, 0, (GLint)SIZE_X, (GLint)SIZE_Y );

	glMatrixMode( GL_PROJECTION );
	glLoadIdentity();

	glMatrixMode( GL_MODELVIEW );
       	glLoadIdentity();	

	{
		int		i;
		// init flat colors
		for ( i = 0; i < 256; i++ )
			Vec3dInit( r_flatcolors[i], RND, RND, RND );
	}

	Vec3dInit( r_origin, 0.0, 0.0, 0.0 );	
}

void GlutMain( void )
{
	glutInit( &r_argc, r_argv );
	glutInitDisplayMode( GLUT_RGB|GLUT_DOUBLE|GLUT_DEPTH );
	glutInitWindowSize( SIZE_X, SIZE_Y );
	glutCreateWindow( "field_gl" );

	InitGL();
	SetupFieldMap( "_field.bin" );

	glutDisplayFunc( GlutDisplay );
	glutReshapeFunc( GlutResize );
	glutKeyboardFunc( GlutKeyPress );
	glutKeyboardUpFunc( GlutKeyRelease );
	glutIdleFunc( GlutIdle );
	
	glutMainLoop();		
}

GLuint Misc_GenTexture_TGA_8888( char *name )
{
	FILE		*h;
	tga_t		*tga;
	int		i;
	GLuint		texobj;
	unsigned char            *tmp, *ptr;

	h = fopen( name, "r" );
	if ( !h )
		Error( "Misc_GenTexture_TGA: can't open file.\n" );
	tga = TGA_Read( h );
	TGA_Dump( tga );
	fclose( h );
	if ( !tga )
		Error( "Misc_GenTexture_TGA: tga failed.\n" );

	glGenTextures( 1, &texobj );
	glBindTexture( GL_TEXTURE_2D, texobj );
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
//	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);

	tmp = (unsigned char *) malloc( tga->image_width*tga->image_height*4 );
	ptr = tmp;
	for ( i = 0; i < tga->image.pixels; i++ )
	{

		*ptr++ = tga->image.red[i];
		*ptr++ = tga->image.green[i];
		*ptr++ = tga->image.blue[i];
		*ptr++ = tga->image.alpha[i];
	}
	TGA_Free( tga );

	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGBA, tga->image_width, tga->image_height, GL_RGBA, GL_UNSIGNED_BYTE, tmp );
	free( tmp );

	return texobj;
}


/*
  =============================================================================
  field stuff

  =============================================================================
*/

#define MAX_FIELDPOINTS		( 400000 )

typedef struct fpoints_s
{
	ivec3d_t		pos;
	vec3d_t			vec;
} fpoint_t;

int		r_fpointnum = 0;
fpoint_t	r_fpoints[MAX_FIELDPOINTS];

void SetupFieldMap( char *name )
{
	int	i;
	FILE		*h;
	h = fopen( name, "r" );
	if ( !h )
		Error( "can't open fieldmap binary.\n" );

	fread( &r_fpointnum, 4, 1, h );
	if ( r_fpointnum > MAX_FIELDPOINTS )
		Error( "reached MAX_FIELDPOINTS\n" );

	for ( i = 0; i < r_fpointnum; i++ )
	{
		short		s;
		fread( &s, 2, 1, h );
		r_fpoints[i].pos[0] = (int) s;
		fread( &s, 2, 1, h );
		r_fpoints[i].pos[1] = (int) s;
		fread( &s, 2, 1, h );
		r_fpoints[i].pos[2] = (int) s;

		fread( &r_fpoints[i].vec[0], 4, 1, h );
		fread( &r_fpoints[i].vec[1], 4, 1, h );
		fread( &r_fpoints[i].vec[2], 4, 1, h );
	}

	printf( " %d fieldcells\n", r_fpointnum );	
}

#define MAX_CACHEDFC		( 8192 )
int	xs, ys, zs;
ivec3d_t	r_fmin;
ivec3d_t	r_fmax;
int		r_cachedfcnum;
//vec3d_t		r_cachedfc[MAX_CACHEDFC];
vec3d_t		r_cachedfp[MAX_CACHEDFC];

typedef struct
{
	vec3d_t		pos;		// for debug
	vec3d_t		vec;	// stream
	vec3d_t		norm;	// reflect

	vec3d_t		vec2;
	int		start;
	int		fade;
} cfpoint_t;

typedef enum 
{
	FieldComponent_stream,
	FieldComponent_reflection
} fieldComponent;

cfpoint_t	r_cachedfpts[MAX_CACHEDFC];

int		r_framecount = 0;

void CacheFieldPointsOfBB( ivec3d_t min, ivec3d_t max )
{
	int		i, j;
	int		num;

	for ( i = 0; i < 3; i++ )
	{
		r_fmin[i] = min[i];
		r_fmax[i] = max[i];
	}

	xs = max[0] - min[0];
	ys = max[1] - min[1];
	zs = max[2] - min[2];

	r_cachedfcnum = xs*ys*zs;

	if ( r_cachedfcnum > MAX_CACHEDFC )
		Error( "reached MAX_CACHEDFC\n" );

	memset( r_cachedfpts, 0, xs*ys*zs*sizeof(cfpoint_t) );
	
	num = 0;
	for ( i = 0; i < r_fpointnum; i++ )
	{
		for ( j = 0; j < 3; j++ )
		{
			if ( r_fpoints[i].pos[j] < min[j] || r_fpoints[i].pos[j] > max[j] )
			{
				break;
			}			
		}
		if ( j != 3 )
			continue;

		{
			ivec3d_t	v;
			v[0] = r_fpoints[i].pos[0] - min[0];
			v[1] = r_fpoints[i].pos[1] - min[1];
			v[2] = r_fpoints[i].pos[2] - min[2];
			Vec3dCopy( r_cachedfpts[v[0]+v[1]*xs+v[2]*xs*ys].vec, r_fpoints[i].vec );
			Vec3dInit( r_cachedfp[v[0]+v[1]*xs+v[2]*xs*ys], r_fpoints[i].pos[0]*1.0, r_fpoints[i].pos[1]*1.0, r_fpoints[i].pos[2]*1.0 );
		}
		num++;
	}
	printf( " %d fieldpoints cached\n", num );
}

void FindFieldVectorForPoint( vec3d_t point, vec3d_t vec )
{
	int		i;
	ivec3d_t		v;
	int		addr;

	v[0] = (int)rint(point[0]/1.0);
	v[1] = (int)rint(point[1]/1.0);
	v[2] = (int)rint(point[2]/1.0);

	for ( i = 0; i < 3; i++ )
	{
		if ( v[i] < r_fmin[i] || v[i] > r_fmax[i] )
			break;
	}
	if ( i != 3 )
	{
		// point out of cached area
		printf( "*" );
		Vec3dInit( vec, 0, 0, 0 );
		return;		
	}

	v[0]-=r_fmin[0];
	v[1]-=r_fmin[1];
	v[2]-=r_fmin[2];

	addr = v[0]+v[1]*xs+v[2]*xs*ys;

	Vec3dCopy( vec, r_cachedfpts[addr].vec );

	Vec3dAdd( vec, r_cachedfpts[addr].vec2, vec );
#if 0
	if ( r_cachedfpts[addr].start < r_framecount )
	{
		for ( ; r_cachedfpts[addr].start < r_framecount; r_cachedfpts[addr].start++ )
			Vec3dScale( r_cachedfpts[addr].vec2, 0.8, r_cachedfpts[addr].vec2 );
	}
#endif
}


void ShockCachedFieldPoints( vec3d_t shockvec, int fade )
{
	int		i;

	for ( i = 0; i < r_cachedfcnum; i++ )
	{
		Vec3dCopy( r_cachedfpts[i].vec2, shockvec );
		r_cachedfpts[i].fade = fade;	
		r_cachedfpts[i].start = r_framecount;
	}
}

void RunShock( void )
{
	int		i;

	for ( i = 0; i < r_cachedfcnum; i++ )
	{
		Vec3dScale( r_cachedfpts[i].vec2, 0.95, r_cachedfpts[i].vec2 );
	}	
}

void DrawCachedFieldPoints( void )
{
	int		i;

	glDisable( GL_TEXTURE_2D );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


	for( i = 0; i < r_cachedfcnum; i++ )
	{
		vec3d_t		v1, v2, tmp;
		vec4d_t		w;

		Vec3dCopy( v1, r_cachedfp[i] );
		Vec3dMA( tmp, 0.5, r_cachedfpts[i].vec, v1 );
		Vec3dMA( v2, 0.5, r_cachedfpts[i].vec2, tmp );

		glBegin( GL_LINES );
		glColor3f( 1.0, 0, 0 );
		CalcVertex( w, v1 );
		glVertex4fv( w );
		glColor3f( 0, 0, 1.0 );
		CalcVertex( w, v2 );
		glVertex4fv( w );
		glEnd();
	}
}


/*
  ==================================================
  field manipulation

  ==================================================
*/
typedef struct
{
	int		xs, ys, zs;
	int		pointnum;
	ivec3d_t		min, max;
	cfpoint_t	pts[4*4*4];	// variable
} field_t;

field_t * NewField( ivec3d_t min, ivec3d_t max )
{
	field_t		*f;
	int		size;
	int		xs, ys, zs;
	int		pointnum;
	int		i;

	xs = 1+max[0]-min[0];
	ys = 1+max[1]-min[1];
	zs = 1+max[2]-min[2];
	pointnum = xs*ys*zs;

	size = (int)&(((field_t *)0)->pts[pointnum]);
	f = (field_t *) malloc( size );
	memset( f, 0, size );

	f->xs = xs;
	f->ys = ys;
	f->zs = zs;
	f->pointnum = pointnum;
	for ( i = 0; i < 3; i++ )
	{
		f->min[i]=min[i];
		f->max[i]=max[i];
	}

	// pos for debug draw
	{
		int		x, y, z;
		for ( x = f->min[0]; x <= f->max[0]; x++ )
		{
			for ( y = f->min[1]; y <= f->max[1]; y++ )
			{
				for ( z = f->min[2]; z <= f->max[2]; z++ )
				{
					int	addr;
					addr = x+y*xs+z*xs*ys;
					f->pts[addr].pos[0] = (fp_t)x;
					f->pts[addr].pos[1] = (fp_t)y;
					f->pts[addr].pos[2] = (fp_t)z;
				}
			}
		}
	}

	return f;
}

field_t * LoadField2Binary( char *name, fieldComponent type )
{
	int		i;
	FILE		*h;
	field_t		*f;
	int		pointnum;
	int		size;

	h = fopen( name, "r" );
	if ( !h )
		Error( "can't open fieldmap binary.\n" );
	
	// read pointnum
	fread( &pointnum, 4, 1, h );

	size = (int)&(((field_t *)0)->pts[pointnum]);
	f = (field_t *) malloc( size );
	memset( f, 0, size );
	
	f->pointnum = pointnum;

	// read bounds
	fread( &f->min[0], 4, 1, h );
	fread( &f->min[1], 4, 1, h );
	fread( &f->min[2], 4, 1, h );

	fread( &f->max[0], 4, 1, h );
	fread( &f->max[1], 4, 1, h );
	fread( &f->max[2], 4, 1, h );

	f->xs = 1+f->max[0]-f->min[0];
	f->ys = 1+f->max[1]-f->min[1];
	f->zs = 1+f->max[2]-f->min[2];

	// read points
	for ( i = 0; i < pointnum; i++ )
	{
		int		addr;
		int		x, y, z;
		short		s;
		fread( &s, 2, 1, h );
		x = (int) s;

		fread( &s, 2, 1, h );
		y = (int) s;

		fread( &s, 2, 1, h );
		z = (int) s;
		
		addr = x+ y*f->xs+ z*f->xs*f->ys;
		
		f->pts[addr].pos[0] = (fp_t) x;
		f->pts[addr].pos[1] = (fp_t) y;
		f->pts[addr].pos[2] = (fp_t) z;

		if ( type == FieldComponent_stream )
		{
			fread( &f->pts[addr].vec[0], 4, 1, h );
			fread( &f->pts[addr].vec[1], 4, 1, h );
			fread( &f->pts[addr].vec[2], 4, 1, h );		
		}
		else
		{
			fread( &f->pts[addr].norm[0], 4, 1, h );
			fread( &f->pts[addr].norm[1], 4, 1, h );
			fread( &f->pts[addr].norm[2], 4, 1, h );		
		}
	}
	return f;
}

cfpoint_t * Field_GetFieldPoint( field_t *f, ivec3d_t pos )
{
	int		i;
	int		addr;
	ivec3d_t		v;

      

	for ( i = 0; i < 3; i++ )
	{
		v[i] = pos[i];		// copy pos

		if ( v[i] < f->min[i] || v[i] > f->max[i] )
			break;
	}
	if ( i != 3 )
	{
		// point out of cached area
		return NULL;		
	}

	v[0]-=f->min[0];
	v[1]-=f->min[1];
	v[2]-=f->min[2];
	
	addr = v[0]+v[1]*f->xs+v[2]*f->xs*f->ys;
	return &f->pts[addr];
}

void Field_OverlayOtherField( field_t *f, field_t *m, ivec3d_t ofs )
{
	ivec3d_t		v, v2;
	cfpoint_t	*source;
	cfpoint_t		*dest;

	for ( v[0] = m->min[0]; v[0] <= m->max[0]; v[0]++ )
	{
		for ( v[1] = m->min[1]; v[1] <= m->max[1]; v[1]++ )
		{
			for ( v[2] = m->min[2]; v[2] <= m->max[2]; v[2]++ )
			{
				source = Field_GetFieldPoint( m, v );
				if ( !source )
					continue;
				
				v2[0] = v[0] + ofs[0];
				v2[1] = v[1] + ofs[1];
				v2[2] = v[2] + ofs[2];
				
				dest = Field_GetFieldPoint( f, v2 );
				if ( !dest )
					continue;

				Vec3dCopy( dest->vec, source->vec );
				Vec3dCopy( dest->norm, source->norm );
			}
		}
	}
}

void Field_WalkThrough( field_t *f, ivec3d_t from, ivec3d_t to, field_t *m )
{
	int		dx, dy, dz;
	fp_t		sx, sy, sz;
	fp_t		x, y, z;
	int		max = 0;
	int		count;

	dx = to[0]-from[0];
	dy = to[1]-from[1];
	dz = to[2]-from[2];

	if ( dx >= dy && dx >= dz )
		max = dx;
	else if ( dy >= dx && dy >= dz )
		max = dy;
	else if ( dz >= dx && dz >= dy )
		max = dz;

	if ( !max )
		Error( "max == 0\n" );

	sx = (fp_t)dx/(fp_t)max;
	sy = (fp_t)dy/(fp_t)max;
	sz = (fp_t)dz/(fp_t)max;

	x = from[0];
	y = from[1];
	z = from[2];
	for ( count = 0; count <= max; count++ )
	{
		cfpoint_t	*p;
		ivec3d_t	v;

		v[0] = (int)rint(x);
		v[1] = (int)rint(y);
		v[2] = (int)rint(z);

		Field_OverlayOtherField( f, m, v );
#if 0
		p = Field_GetFieldPoint( f, v );
		if ( p )
		{
			Vec3dInit( p->vec, 0, 1, 0 );
		}
#endif
		x+=sx;
		y+=sy;
		z+=sz;
	}
		
}

void Field_DrawFieldPoints( field_t *f )
{
	int		i;

	glDisable( GL_TEXTURE_2D );
	glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );


	for( i = 0; i < f->pointnum; i++ )
	{
		vec3d_t		v1, v2, tmp;
		vec4d_t		w;
		
		Vec3dCopy( v1, f->pts[i].pos );
		Vec3dMA( tmp, 0.5, f->pts[i].vec, v1 );
		Vec3dMA( v2, 0.5, f->pts[i].norm, tmp );
       
	
		glBegin( GL_LINES );
		glColor3f( 0, 0, 1.0 );
		CalcVertex( w, v2 );
		glVertex4fv( w );

		glColor3f( 1.0, 0, 0 );
		CalcVertex( w, v1 );
		glVertex4fv( w );
		glEnd();

		glBegin( GL_POINTS );
		glColor3f( 1, 1, 1 );
		glVertex4fv( w );
		glEnd();
	}
}


void Field_FindVectorForPoint( field_t *f, vec3d_t point, vec3d_t vec )
{
	int		i;
	ivec3d_t		v;
	int		addr;

	v[0] = (int)rint(point[0]/1.0);
	v[1] = (int)rint(point[1]/1.0);
	v[2] = (int)rint(point[2]/1.0);
	
	for ( i = 0; i < 3; i++ )
	{
		if ( v[i] < f->min[i] || v[i] > f->max[i] )
			break;
	}
	if ( i != 3 )
	{
		// point out of cached area
		printf( "*" );
		Vec3dInit( vec, 0, 0, 0 );
		return;		
	}

	v[0]-=f->min[0];
	v[1]-=f->min[1];
	v[2]-=f->min[2];
	
	addr = v[0]+v[1]*f->xs+v[2]*f->xs*f->ys;
	Vec3dCopy( vec, f->pts[addr].vec );
	Vec3dAdd( vec, vec, f->pts[addr].vec2 );
}

cfpoint_t * Field_FindFieldPointForPoint( field_t *f, vec3d_t point )
{
	int		i;
	ivec3d_t		v;
	int		addr;

	v[0] = (int)rint(point[0]/1.0);
	v[1] = (int)rint(point[1]/1.0);
	v[2] = (int)rint(point[2]/1.0);
	
	for ( i = 0; i < 3; i++ )
	{
		if ( v[i] < f->min[i] || v[i] > f->max[i] )
			break;
	}
	if ( i != 3 )
	{
		// point out of cached area
		return NULL;
	}

	v[0]-=f->min[0];
	v[1]-=f->min[1];
	v[2]-=f->min[2];	

	addr = v[0]+v[1]*f->xs+v[2]*f->xs*f->ys;
	return &f->pts[addr];
}

void Field_ShockFieldPoints( field_t *f, vec3d_t shockvec )
{
	int		i;

	for ( i = 0; i < f->pointnum; i++ )
	{
		Vec3dCopy( f->pts[i].vec2, shockvec );
//		f->pts[i].start = r_framecount;
	}
}

void Field_RunShock( field_t *f )
{
	int		i;

	for ( i = 0; i < f->pointnum; i++ )
	{
		Vec3dScale( f->pts[i].vec2, 0.95, f->pts[i].vec2 );
	}	
}


//void Field_

/*
  =============================================================================
  particle stuff

  =============================================================================
*/

typedef struct particle_s
{
	vec3d_t		pos;
	vec3d_t		dir;
	vec3d_t		brown;	// random component

	fp_t		speed;
	int		count;
} particle_t;

#define MAX_PARTICLES	( 100 )

particle_t	r_particles[MAX_PARTICLES];


void RunParticle( void )
{
	int		i;
	vec3d_t		v3;
	vec4d_t		v4, v;
	vec3d_t		vec;
	static GLuint	texobj;
	static fp_t	rotate[64][4][2];
	static int	r_count = 0;

	fp_t		shrink;

	static field_t	*field;
	static field_t	*field2;
	static field_t	*field3;

	if ( r_count == 0 )
	{
		ivec3d_t	min = { 0, 0, 0 };
		ivec3d_t	max = { 7, 7, 7 };
		ivec3d_t	ofsvec = { 3, 3, 3 };

//		texobj = Misc_GenTexture_TGA_8888( "/home/mcb/art/fx/smoke2.tga" );
//		texobj = Misc_GenTexture_TGA_8888( "/home/mcb/art/fx/blip2.tga" );

		CacheFieldPointsOfBB( min, max );	

		field = NewField( min, max );//LoadField2Binary( "_field2.bin" );
		field2 = LoadField2Binary( "_field2.bin", FieldComponent_stream );
		field3 = LoadField2Binary( "_field_obstacle1.bin", FieldComponent_reflection );
		
//		Field_WalkThrough( field, min, max, field2 );
//		Field_OverlayOtherField( field, field2, ofsvec );

		{
			fp_t	pts[4][2]={ {1.0, -1.0}, {-1.0,-1.0}, { -1.0, 1.0 }, { 1.0, 1.0 } };
			int	j, k;
			fp_t	s, c, angle;
			for ( j = 0; j < 64; j++ )
			{
				angle = (j/64.0) * M_PI*2.0; //(j*(360.0/16.0)) / (180.0*M_PI);
				s = sin( angle );
				c = cos( angle );
				for ( k = 0; k < 4; k++ )
				{
					rotate[j][k][0] = c*pts[k][0] - s*pts[k][1];
					rotate[j][k][1] = s*pts[k][0] + c*pts[k][1];
				}
			}
		}

		texobj = Misc_GenTexture_TGA_8888( "/home/mcb/art/fx/blip2.tga" );
	}

	Field_RunShock( field );
	if ( !(r_count & 127 ) )
	{
		vec3d_t		shockvec = { 0, 2, 0 };
//		Field_ShockFieldPoints( field, shockvec );
		printf( "shock\n" );
	}
	if ( !((r_count+64) & 127 ) )
	{
		vec3d_t		shockvec = { 0.0, -1.0, 0.0 };
//		Field_ShockFieldPoints( field, shockvec );
		printf( "shock2\n" );
	}

	if ( !(r_count & 15 ) )
	{
		ivec3d_t	from = { 0, 0, 0 };
		ivec3d_t	to = { 7, 0, 0 };
		ivec3d_t	ofsvec = { 4, 0, 0 };

//		to[0] = 7;
//		to[1] = (random()%7000)/1000;
//		to[2] = (random()%7000)/1000;;
		Field_WalkThrough( field, from, to, field2 );
		Field_OverlayOtherField( field, field3, ofsvec );
	}

//	DrawCachedFieldPoints();
//	Field_DrawFieldPoints( field );

	if ( r_count == 0 )
	{
		for ( i = 0; i < MAX_PARTICLES; i++ )
		{
			r_particles[i].count = i;
			r_particles[i].speed = 1.0/16.0;
			Vec3dInit( r_particles[i].pos, 0.0, 0.0, 0.0 );
			Vec3dInit( r_particles[i].dir, 0, 1, 0 );
		}
	}

	glColor3f( 1, 1, 1 );


	glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
	glBindTexture( GL_TEXTURE_2D, texobj );
	glEnable( GL_TEXTURE_2D );
	glEnable ( GL_BLEND );                                          
//	glBlendFunc( GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA );
	glBlendFunc( GL_ONE, GL_ONE );
	glDepthMask(GL_FALSE);

	for ( i = 0; i < MAX_PARTICLES; i++ )
	{
		cfpoint_t		*p;
		
		if ( r_particles[i].count <= 0 )
		{
			r_particles[i].count = MAX_PARTICLES;
			r_particles[i].speed = 1.0/16.0;
//			Vec3dInit( r_particles[i].pos, 0.5, 1.5, 1.5 );
//			Vec3dInit( r_particles[i].pos, 1.45+(random()%1000)/10000.0, 0.5, 1.45+(random()%1000)/10000.0 );
			Vec3dInit( r_particles[i].pos, 1.5, 0.5+(random()%1000)/1000.0, 0.5+(random()%1000)/1000.0 );
			Vec3dInit( r_particles[i].dir, 0, 0, 0 );
		}
		
//		Field_FindVectorForPoint( field, r_particles[i].pos, vec );		

		if ( ! ( r_particles[i].count & 15 ) )
		{
			Vec3dInit( r_particles[i].brown, -0.0005+(random()%1000)/100000.0, -0.0005+(random()%1000)/100000.0, -0.0005+(random()%1000)/100000.0 );	
		}
		
		Vec3dAdd( r_particles[i].pos, r_particles[i].pos, r_particles[i].brown );

		p = Field_FindFieldPointForPoint( field, r_particles[i].pos );
		if ( p )
		{		       
			
			// stream ?
			if ( p->vec[0] != 0.0 || p->vec[1] != 0.0 || p->vec[1] != 0.0 )
			{
				Vec3dCopy( r_particles[i].dir, p->vec );
			}

			Vec3dMA( r_particles[i].pos, r_particles[i].speed, r_particles[i].dir, r_particles[i].pos );	

			if ( p->norm[0] != 0.0 || p->norm[1] != 0.0 || p->norm[2] != 0.0 )
			{

				Vec3dMA( r_particles[i].pos, r_particles[i].speed, p->norm, r_particles[i].pos );	
			}
		}
		else
		{
			Vec3dMA( r_particles[i].pos, r_particles[i].speed, r_particles[i].dir, r_particles[i].pos );	
		}

//		Vec3dMA( r_particles[i].pos, r_particles[i].speed, vec, r_particles[i].pos );
		r_particles[i].count--;

		shrink = 0.7;
		if ( r_particles[i].count > 95 )
		{
			glColor3f( (95-r_particles[i].count)/5.0, (95-r_particles[i].count)/5.0, (95-r_particles[i].count)/5.0 );
		}
		else
		{
			glColor3f( r_particles[i].count/95.0, r_particles[i].count/95.0, r_particles[i].count/95.0 );
//			shrink = r_particles[i].count/10.0;
		}
		if ( r_particles[i].count < 5 )
		{
			shrink*=0.85;
		}
//		shrink = r_particles[i].count/10.0;

		CalcVertex( v, r_particles[i].pos );
		{
			int	frame;
			
			frame = /*r_particles[i].count*/ (r_count*2) & 63;
			if ( i&1 )
				frame = 63-frame;
			
			glBegin( GL_TRIANGLE_FAN );
			glTexCoord2f( 0,0 );
			glVertex3f( (v[0]+rotate[frame][0][0]*shrink)/v[3], (v[1]+rotate[frame][0][1]*shrink)/v[3], 1.0/v[3] );
			glTexCoord2f( 1,0 );
			glVertex3f( (v[0]+rotate[frame][1][0]*shrink)/v[3], (v[1]+rotate[frame][1][1]*shrink)/v[3], 1.0/v[3] );
			glTexCoord2f( 1,1 );
			glVertex3f( (v[0]+rotate[frame][2][0]*shrink)/v[3], (v[1]+rotate[frame][2][1]*shrink)/v[3], 1.0/v[3] );
			glTexCoord2f( 0,1 );
			glVertex3f( (v[0]+rotate[frame][3][0]*shrink)/v[3], (v[1]+rotate[frame][3][1]*shrink)/v[3], 1.0/v[3] );
			glEnd();
		}

	}
	
	r_count++;
	r_framecount++;
}

int main( int argc, char *argv[] )
{
	printf( "===== field_gl - field test engine =====\n" );

	r_argc = argc;
	r_argv = argv;

	GlutMain();
}
