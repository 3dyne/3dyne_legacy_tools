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



// envopt.c

#include "envopt.h"

#include <stdlib.h>

#include "lib_poly.h"

#define	EDGEMAP_MAXWIDTH	( 128 )
#define EDGEMAP_MAXHEIGHT	( 128 )

#define HEIGHTMAP_MAXWIDTH	( 128 )
#define HEIGHTMAP_MAXHEIGHT	( 128 )

int	ewidth, eheight;
int	emap[EDGEMAP_MAXWIDTH][EDGEMAP_MAXHEIGHT];

int	hwidth, hheight;
fp_t	hmap[HEIGHTMAP_MAXWIDTH][HEIGHTMAP_MAXHEIGHT];

#define MAX_EDGES_PER_VERTEX	( 64 )
#define MAX_VERTICES		( 65000 )

typedef struct vertex_s
{
	int		edgenum;
	int		edges[MAX_EDGES_PER_VERTEX];
	
	int		trinum;
	int		tris[MAX_EDGES_PER_VERTEX][2];
} vertex_t;


int		vertexnum;
vec3d_t		vertices[MAX_VERTICES];
vertex_t	verts[MAX_VERTICES];

vec3d_t		vmin, vmax;

FILE		*handle;

#define RND ( (random()%1000000)/1000000.0)

#define SCL ( 128.0 * 40.0 )


typedef struct line_s
{
	vec3d_t		p1;
	vec3d_t		p2;
} line_t;

line_t * NewLine( void )
{
	return malloc( sizeof( line_t ) );
}

void FreeLine( line_t *l )
{
	free( l );
}

line_t * BaseLineForPlane( vec3d_t norm, fp_t dist )
{
	vec3d_t		p1, p2;
	vec3d_t		q1, q2;
	fp_t		d1, d2, scl;
	int		i;

	line_t		*l;

	l = NewLine();

	if ( fabs(norm[0]) >= fabs(norm[2]) )
	{
		// major x
		Vec3dInit( p1, -8192.0, 0.0, -8192.0 );
		Vec3dInit( p2,  8192.0, 0.0, -8192.0 );
		Vec3dInit( q1, -8192.0, 0.0,  8192.0 );
		Vec3dInit( q2,  8192.0, 0.0,  8192.0 );
	}
	else
	{
		// major z
		Vec3dInit( p1, -8192.0, 0.0, -8192.0 );
		Vec3dInit( p2, -8192.0, 0.0,  8192.0 );
		Vec3dInit( q1,  8192.0, 0.0, -8192.0 );
		Vec3dInit( q2,  8192.0, 0.0,  8192.0 );
	}

	d1 = Vec3dDotProduct( p1, norm ) - dist;
	d2 = Vec3dDotProduct( p2, norm ) - dist;
	scl = d1 / (d1-d2);
	for ( i = 0; i < 3; i++ )
	{
		l->p1[i] = p1[i] + scl*(p2[i]-p1[i]);
	}

	d1 = Vec3dDotProduct( q1, norm ) - dist;
	d2 = Vec3dDotProduct( q2, norm ) - dist;
	scl = d1 / (d1-d2);
	for ( i = 0; i < 3; i++ )
	{
		l->p2[i] = q1[i] + scl*(q2[i]-q1[i]);
	}
	
	return l;
}

void ClipLineInPlace( line_t **inout, vec3d_t norm, fp_t dist )
{
	fp_t		d1, d2;
	int		i;
	fp_t		scl;
	vec3d_t		clip;
	line_t		*l;

	l = *inout;

	d1 = Vec3dDotProduct( norm, l->p1 ) - dist;
	d2 = Vec3dDotProduct( norm, l->p2 ) - dist;

	if ( d1 > 0.0 && d2 > 0.0 )
	{
		FreeLine( l );
		*inout = NULL;
		return;
	}

	if ( d1 <= 0.0 && d2 <= 0.0 )
	{
		return;
	}

	scl = d1 / (d1-d2);
	for ( i = 0; i < 3; i++ )
	{
		clip[i] = l->p1[i] + scl*(l->p2[i]-l->p1[i]);
	}

	l = NewLine();
	if ( d1 <= 0.0 )
	{
	       Vec3dCopy( l->p1, (*inout)->p1 );
	       Vec3dCopy( l->p2, clip );
	       FreeLine( *inout );
	       *inout = l;
	}
	else
	{
		Vec3dCopy( l->p2, (*inout)->p2 );
		Vec3dCopy( l->p1, clip );
		FreeLine( *inout );
		*inout = l;
	}

}

void InitEdgeMap( tga_t *edgemap )
{
	int	w;
	int	h;
	int	x, y, i;
	int	count[4];
	fp_t	e;
	int	d;
	int	num;

	Vec3dInitBB( vmin, vmax, 999999.9 );

	w = edgemap->image_width;
	h = edgemap->image_height;

	printf( "edgemap size: %d x %d\n", w, h );

	ewidth = w;
	eheight = h;

	if ( w > EDGEMAP_MAXWIDTH || h > EDGEMAP_MAXHEIGHT )
		Error( "edgemap to big.\n" );

//	for ( i = 0; i < 4; i++ )
//		count[i] = 0.0;

	i = 0;
	num = 0;
	for ( y = 0; y < h; y++ )
	{
		for ( x = 0; x < w; x++ )
		{
			e = ((fp_t)(edgemap->image.red[i++])) / 255.0;
//			emap[x][y] = e;
			
//			e-= 159.0/255.0;

			if ( e*0.8 > RND )
			{
				emap[x][y] = 1;
				Vec3dInit( vertices[num], x*40.0, 0.0, y*40.0 );
				Vec3dAddToBB( vmin ,vmax, vertices[num] );
				num++;
			}
			else
				emap[x][y] = 0;

		}
	}

//	for ( 

	printf( "num: %d\n", num );
//	getchar();
	vertexnum = num;
//	for ( i = 0; i < 4; i++ )
//		printf( "count %d: %d\n", i, count[i] );
	
}

void InitHeightMap( tga_t *heightmap )
{
	int	w;
	int	h;
	int	x, y, i;
	fp_t	e;

	w = heightmap->image_width;
	h = heightmap->image_height;

	printf( "heightmap size: %d x %d\n", w, h );

	hwidth = w;
	hheight = h;

	if ( w > HEIGHTMAP_MAXWIDTH || h > HEIGHTMAP_MAXHEIGHT )
		Error( "heightmap to big.\n" );

	i = 0;
	for ( y = 0; y < h; y++ )
	{
		for ( x = 0; x < w; x++ )
		{
			e = ((fp_t)(heightmap->image.red[i++])) / 255.0;
			hmap[x][y] = e;
		}
	}	
}


void SetupCells( void )
{
	int	w, h;
	int	x, y;
	int	x1, y1;
	int	x2, y2;

	int	tri1;
	int	tri2;
	int	e;
	
	tri1 = tri2 = 0;

	w = hwidth / ewidth;
	h = hheight / eheight;

	printf( "max vertices per cell: %d x %d\n", w, h );

	for ( x = 0; x < ewidth; x++ )
	{
		for ( y = 0; y < eheight; y++ )
		{
			e = 1 << emap[x][y];
			e = e * e;
//			printf( "%d ", e );
			
			tri2+=16;
			tri1+=e;

			x1 = x * w;
			x2 = (x+1) * w;
			y1 = y * h;
			y2 = (y+1) * h;
			printf( "(%d,%d)-(%d,%d)\n", x1, y1, x2, y2 );
		}
	}
	
	printf( "%d to %d\n", tri1, tri2 );
}

void FixPolygon2( polygon_t *inout, vec3d_t center )
{
	vec3d_t		vright = { 1, 0, 0 };
	int		i, j;
	vec3d_t		v;
	fp_t		t;
	fp_t		dx, dy, ax, ay;

	int		best;
	fp_t		min;
	vec3d_t		tmp;

	for ( j = 0; j < inout->pointnum; j++ )
	{

		min = 999999.9;
		best = -1;
		for ( i = j; i < inout->pointnum; i++ )
		{
			Vec3dSub( v, inout->p[i], center );
			Vec3dUnify( v );
			
			dx = v[0];
			dy = v[2];
			ax = fabs( dx );
			ay = fabs( dy );
			t = (ax+ay == 0) ? 0 : (dy/(ax+ay));
			if ( dx < 0.0 ) 
				t = 2-t;
			else if ( dy < 0 )
				t = 4+t;
			t *= 90.0;

			printf( "%f\n", t );

			if ( t < min )
			{
				min = t;
				best = i;
			}
		}
		
		printf( "best: %d %f\n ", best, min );

		// SWAP
		Vec3dCopy( tmp, inout->p[j] );
		Vec3dCopy( inout->p[j], inout->p[best] );
		Vec3dCopy( inout->p[best], tmp );

	}
	printf( "\n" );
	
}

void FixPolygon( polygon_t **inout, vec3d_t center, vec3d_t norm )
{
	polygon_t	*p;
	int		num, num2;
	int		flags[256];
	int		i, j;
	vec3d_t		v1, v2;
	vec3d_t		tmp;
	fp_t		min, l;
	int		best;

	num = (*inout)->pointnum;
	p = NewPolygon( num );
	p->pointnum = num;

	memset( flags, 255, 256*4 );


	flags[0] = 0;
	Vec3dCopy( p->p[0], (*inout)->p[0] );
	i = 0;
	for ( num2 = 1; num2 < num; )
	{
		min = 999999.9;
		best = -1;
		for ( j = 1; j < num; j++ )
		{
			if ( !flags[j] )
				continue;

//			printf( "%d %d: ", i, j );
			Vec3dSub( v2, (*inout)->p[j], (*inout)->p[i] );
			Vec3dSub( v1, center, (*inout)->p[i] );
			Vec3dCrossProduct( tmp, v1, v2 );
			Vec3dPrint( tmp );
//			getchar();
			Vec3dUnify( tmp );
			Vec3dAdd( tmp, norm, tmp );
//			printf( "best %f\n", Vec3dLen( tmp ) );

			if ( Vec3dLen( tmp ) < 1.9 )	// clockwise ?
				continue;
			Vec3dSub( tmp, (*inout)->p[j], (*inout)->p[i] );
			l = Vec3dLen( tmp );
			if ( l < min )
			{
				min = l;
				best = j;
			}

		}

		if ( best == -1 )
			Error("error\n");

		Vec3dCopy( p->p[num2], (*inout)->p[best] );
		flags[best] = 0;
		i = best;
		num2++;
	}

	FreePolygon( *inout );
	*inout = p;
}

void Init()
{
	fp_t	*v1, *v2;
	vec3d_t		delta, norm, pos;		
	fp_t		dist;
	int		i, j, k, num;

	vec3d_t		norms[65000];
	fp_t		dists[65000];
	polygon_t	*polys[65000];
	int		out[256];
	int		num2;

	fp_t		min, l;
	int		best;
	int		tmp;

	vec3d_t		vup = { 0.0, 1.0, 0.0 };
	polygon_t	*poly;
	

	for ( i = 0; i < vertexnum; i++ )
	{
		v1 = vertices[i];

		//
		// setup planes
		//
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( polys[j] )
			{
				FreePolygon( polys[j] );
				polys[j] = NULL;
			}

			if ( i == j )
				continue;
			v2 = vertices[j];
			
			Vec3dSub( delta, v2, v1 );
			Vec3dUnify2( norm, delta );
			Vec3dScale( delta, 0.5, delta );
			Vec3dAdd( pos, v1, delta );

			dist = Vec3dInitPlane2( norm, pos );

			Vec3dCopy( norms[j], norm );
			dists[j] = dist;

			polys[j] = BasePolygonForPlane( norm, dist );
		}

		//
		// clip each by each
		//
#if 0
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( j == i )
				continue;

			for ( k = 0; k < vertexnum; k++ )
			{			     
				if ( k == i || j == k )
					continue;

				if ( !polys[k] )
					continue;

				if ( polys[j] )
				{
					ClipPolygonInPlace( &polys[j], norms[k], dists[k] );
				}
				else
					break;
			}
		}

#else
		
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( j == i )
				continue;

			if ( !polys[j] )
				continue;
			

			for ( k = 0; k < j; k++ )
			{
				if ( k == i )
					continue;

				if ( !polys[k] )
					continue;

				if ( polys[j] )
					ClipPolygonInPlace( &polys[j], norms[k], dists[k] );
				else
					goto skip1;
			}

			for ( k = 0; k < j; k++ )
			{
				if ( k == i )
					continue;
				if ( !polys[k] )
					continue;
				ClipPolygonInPlace( &polys[k], norms[j], dists[j] );
			}
			
		skip1:

		}
#endif

		printf( "v%d: ", i );
		verts[i].edgenum = verts[i].trinum = 0;
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( i == j )
				continue;
			if ( polys[j] )
			{
				if ( verts[i].edgenum == MAX_EDGES_PER_VERTEX )
					Error( "edge overflow.\n" );
				verts[i].edges[verts[i].edgenum++] = j;
				printf( "%d ", j );			
			}
		}
		printf( ": %d\n", verts[i].edgenum );

	}

	//
}

void Init2()
{
	fp_t	*v1, *v2;
	vec3d_t		delta, norm, pos;		
	fp_t		dist;
	int		i, j, k;

	vec3d_t		norms[65000];
	fp_t		dists[65000];
	line_t		*lines[65000];
	int		out[256];



//	polygon_t	*poly;
	

	for ( i = 0; i < vertexnum; i++ )
	{
		v1 = vertices[i];

		//
		// setup planes
		//
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( lines[j] )
			{
//				FreePolygon( polys[j] );
				FreeLine( lines[j] );
				lines[j] = NULL;
			}

			if ( i == j )
				continue;
			v2 = vertices[j];
			
			Vec3dSub( delta, v2, v1 );
			Vec3dUnify2( norm, delta );
			Vec3dScale( delta, 0.5, delta );
			Vec3dAdd( pos, v1, delta );

			dist = Vec3dInitPlane2( norm, pos );

			Vec3dCopy( norms[j], norm );
			dists[j] = dist;

			lines[j] = BaseLineForPlane( norm, dist );
		}

		//
		// clip each by each
		//
#if 0
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( j == i )
				continue;

			for ( k = 0; k < vertexnum; k++ )
			{			     
				if ( k == i || j == k )
					continue;

				if ( !polys[k] )
					continue;

				if ( polys[j] )
				{
					ClipPolygonInPlace( &polys[j], norms[k], dists[k] );
				}
				else
					break;
			}
		}

#else
		
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( j == i )
				continue;

			if ( !lines[j] )
				continue;
			

			for ( k = 0; k < j; k++ )
			{
				if ( k == i )
					continue;

				if ( !lines[k] )
					continue;

				if ( lines[j] )
					ClipLineInPlace( &lines[j], norms[k], dists[k] );
				else
					goto skip1;
			}

			for ( k = 0; k < j; k++ )
			{
				if ( k == i )
					continue;
				if ( !lines[k] )
					continue;
				ClipLineInPlace( &lines[k], norms[j], dists[j] );
			}
			
		skip1:

		}
#endif

		printf( "v%d: ", i );
		verts[i].edgenum = verts[i].trinum = 0;
		for ( j = 0; j < vertexnum; j++ )
		{
			if ( i == j )
				continue;
			if ( lines[j] )
			{
				if ( verts[i].edgenum == MAX_EDGES_PER_VERTEX )
					Error( "edge overflow.\n" );
				verts[i].edges[verts[i].edgenum++] = j;
				printf( "%d ", j );			
			}
		}
		printf( ": %d\n", verts[i].edgenum );

	}

	//
}


bool_t TestEdge( int v, int vtest )
{
	int		i;
	for ( i = 0; i < verts[v].edgenum; i++ )
		if ( verts[v].edges[i] == vtest )
			return true;

	return false;
}

bool_t TestTri( int v, int vtest1, int vtest2 )
{
	int		i;
	for ( i = 0; i < verts[v].trinum; i++ )
	{
		if ( (verts[v].tris[i][0] == vtest1 && verts[v].tris[i][1] == vtest2 ) ||
		     (verts[v].tris[i][0] == vtest2 && verts[v].tris[i][1] == vtest1 ) )
			return true;
	}
	return false;
}

void AddTri( int v, int v1, int v2 )
{
	if ( verts[v].trinum == MAX_EDGES_PER_VERTEX )
		Error( "tri overflow.\n" );
	verts[v].tris[verts[v].trinum][0] = v1;
	verts[v].tris[verts[v].trinum][1] = v2;
	verts[v].trinum++;
}

void Test2( void )
{
	int		i, j, k;
	int		v1, v2;
	int		tri;


	for ( i = 0; i < vertexnum; i++ )
	{
		tri = 0;
		for ( j = 0; j < verts[i].edgenum; j++ )
		{
			v1 = verts[i].edges[j];
#if 0

			fprintf( handle, "%f %f %f\n", vertices[i][0]/SCL, vertices[i][1]/SCL, vertices[i][2]/SCL );
			fprintf( handle, "%f %f %f\n", vertices[v1][0]/SCL, vertices[v1][1]/SCL, vertices[v1][2]/SCL );
			fprintf( handle, "%f %f %f\n", vertices[v1][0]/SCL, vertices[v1][1]/SCL, vertices[v1][2]/SCL );
			fprintf( handle, "\n" );

#else		       
			for ( k = 0; k < verts[i].edgenum; k++ )
			{
				if ( j == k )
					continue;
				
				v2 = verts[i].edges[k];

				if ( TestEdge( v1, v2 ) || TestEdge( v2, v1 ) )
				{
					if ( TestTri( i, v1, v2 ) ||
					     TestTri( v1, i, v2 ) ||
					     TestTri( v2, i, v1 ) )
						continue;

					AddTri( i, v1, v2 );
					AddTri( v1, i, v2 );
					AddTri( v2, i, v1 );
#if 0
					fprintf( handle, "%f %f %f\n", vertices[i][0]/SCL, vertices[i][1]/SCL, vertices[i][2]/SCL );
					fprintf( handle, "%f %f %f\n", vertices[v1][0]/SCL, vertices[v1][1]/SCL, vertices[v1][2]/SCL );
					fprintf( handle, "%f %f %f\n", vertices[v2][0]/SCL, vertices[v2][1]/SCL, vertices[v2][2]/SCL );
#else
					fprintf( handle, "%d %d %d\n", i, v1, v2 );
#endif
					fprintf( handle, "\n" );	

					tri++;
				}
			}
#endif	
		}

		printf( "v%d: %d tris\n", i, tri );	
	}

}

void WriteVertices( void )
{
	int		i;

	for ( i = 0; i < vertexnum; i++ )
	{
		fprintf( handle, "%f %f %f\n", vertices[i][0]/SCL, vertices[i][1]/SCL, vertices[i][2]/SCL );
	}
}

int main( int argc, char *argv[] )
{
	char	*height_name;
	char	*edge_name;

	tga_t	*heightmap;
	tga_t	*edgemap;

	int		i, j;
	int		num;


	FILE	*h;

	printf( "===== envpot - build and optimize a environment data =====\n" );
	SetCmdArgs( argc, argv );


	height_name = GetCmdOpt2( "-h" );
	edge_name = GetCmdOpt2( "-e" );

	if ( !height_name )
		Error( "no heightmap.\n" );
	if ( !edge_name )
		Error( "no edgemap.\n" );

	h = fopen( height_name, "r" );
	if ( !h )
		Error( "can't open heightmap.\n" );
	heightmap = TGA_Read( h );
	fclose( h );

	h = fopen( edge_name, "r" );
	if ( !h )
		Error( "can't open edgemap.\n" );
	edgemap = TGA_Read( h );
	fclose( h );

	InitEdgeMap( edgemap );
//	InitHeightMap( heightmap );
//	SetupCells();

#if 0	
	num = 0;
	for ( i = 0; i < 10; i++ )
	{
		for ( j = 0; j < 10; j++ )
		{
//			Vec3dInit( vertices[num++], (random()%1000)/1000.0, 0.0, (random()%1000)/1000.0 );
			Vec3dInit( vertices[num++], j/10.0, 0.0, i/10.0 );
		}
	}

	for ( i = 0; i < 5; i++ )
	{
		for ( j = 0; j < 5; j++ )
		{
//			Vec3dInit( vertices[num++], (random()%1000)/1000.0, 0.0, (random()%1000)/1000.0 );
			Vec3dInit( vertices[num++], 0.05+j/10.0, 0.0, 0.05+i/10.0 );
		}
	}

	vertexnum = num;
#endif

//	Vec2dInit( vertices[0].v, 0.0, 0.0 );
//	Vec2dInit( vertices[1].v, 0.0, 1.0 );
//	Vec2dInit( vertices[2].v, -1.0, 0.5 );
//	Vec2dInit( vertices[3].v, 1.0, 0.5 );
//	Vec2dInit( vertices[4].v, 2.0, 0.5 );
//	vertexnum = 5;


	Init2();
	handle = fopen( "env_tris", "w" );
	Test2();
	fprintf( handle, "end" );
	fclose( handle );

	handle = fopen( "env_vertices", "w" );
	WriteVertices();
	fclose( handle );

}
