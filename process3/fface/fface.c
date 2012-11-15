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



// fface.c

#include "fface.h"

fp_t		equal_dist;
int		p_facenum;
face_t		p_faces[MAX_FACES];

int		out_vertexnum;
vec3d_t		out_vertices[MAX_VERTICES];

int		out_facevertexnum;
int		out_facevertices[MAX_FACEVERTICES];

vec3d_t		p_min, p_max;

bool_t CheckCollinearEdge( vec3d_t p1, vec3d_t p2, vec3d_t t )
{
	vec3d_t		v;
	fp_t		d, d1, d2;

	Vec3dSub( v, p2, p1 );
	d = Vec3dLen( v );

	Vec3dSub( v, t, p1 );
	d1 = Vec3dLen( v );

	Vec3dSub( v, t, p2 );
	d2 = Vec3dLen( v );

	if ( fabs( (d1+d2) - d ) < 0.1 )
		return true;
	
	return false;
}

bool_t CheckSame( vec3d_t p1, vec3d_t p2 )
{
	int		i;

	for ( i = 0; i < 3; i++ )
	{
		if ( fabs( p1[i]-p2[i] ) > equal_dist )
			break;
	}
	if ( i != 3 )
		return false;
	return true;
}

polygon_t * PolygonRemoveEqualPoints( polygon_t *in )
{
	polygon_t		*p;
	int			i;

	p = NewPolygon( in->pointnum );
	for ( i = 0; i < in->pointnum; i++ )
	{
		if ( CheckSame( in->p[i], in->p[(i+1)%in->pointnum] ) )
		{
			continue;
		}
		Vec3dCopy( p->p[p->pointnum], in->p[i] );
		p->pointnum++;
	}
	return p;
}


/*
  ====================
  Read_Faces

  ====================
*/
void Read_Faces( char *name )
{
	tokenstream_t		*ts;
	int		i, pointnum;
	polygon_t	*poly;

	printf( "Read faces ..." );

	p_facenum = 0;
	Vec3dInitBB( p_min, p_max, 999999.9 );

	ts = BeginTokenStream( name );
	if ( !ts )
		Error( "Read_Faces: can't open file.\n" );

	for (;;)
	{
		GetToken( ts );
		if ( !strcmp( "end", ts->token ) )
			break;

		if ( p_facenum == MAX_FACES )
			Error( "reached MAX_FACES.\n" );

		if ( ts->token[0] != 'f' )
			Error( "expect f<face num>.\n" );

		if ( p_facenum != atoi( &ts->token[1] ) )
			Error( "faces not sorted.\n" );

		// ignore texdef
		GetToken( ts );
//		p_faces[p_facenum].texdef = atoi( ts->token );
		
		// expect point num
		GetToken( ts );
		pointnum = atoi( ts->token );
		p_faces[p_facenum].p = NewPolygon( pointnum );
		p_faces[p_facenum].p->pointnum = pointnum;

		// read points and add to bb
		Vec3dInitBB( p_faces[p_facenum].min, p_faces[p_facenum].max, 999999.9 );
		for ( i = 0; i < pointnum; i++ )
		{
			// expect '('
			GetToken( ts );
			if ( ts->token[0] != '(' )
				Error( "expect '('\n" );

			// read 3 floats
			GetToken( ts );
			p_faces[p_facenum].p->p[i][0] = atof( ts->token );
			GetToken( ts );
			p_faces[p_facenum].p->p[i][1] = atof( ts->token );
			GetToken( ts );
			p_faces[p_facenum].p->p[i][2] = atof( ts->token );

			Vec3dAddToBB( p_faces[p_facenum].min, p_faces[p_facenum].max, p_faces[p_facenum].p->p[i] );
			Vec3dAddToBB( p_min, p_max, p_faces[p_facenum].p->p[i] );

			// expect ')'
			GetToken( ts );
			if ( ts->token[0] != ')' )
				Error( "expect ')'\n" );
		}
		
		//
		// finish face
		//
		poly = PolygonRemoveEqualPoints( p_faces[p_facenum].p );
		if ( poly->pointnum != p_faces[p_facenum].p->pointnum )
		{
			printf( "removed equal points !\n" );
			FreePolygon( p_faces[p_facenum].p );
			p_faces[p_facenum].p = poly;
		}
		else
		{
			FreePolygon( poly );
		}

		// calc plane
		PlaneFromPolygon( p_faces[p_facenum].p, p_faces[p_facenum].norm, &p_faces[p_facenum].dist );

		// expand bb by equal_dist
		for ( i = 0; i < 3; i++ )
		{
			p_faces[p_facenum].min[i] -= equal_dist;
			p_faces[p_facenum].max[i] += equal_dist;
		}

		p_facenum++;
	}

	EndTokenStream( ts );
	printf( " %d faces.\n", p_facenum );		
	printf( " min " ); Vec3dPrint( p_min );
	printf( " max " ); Vec3dPrint( p_max );
}



/*
  ========================================
  unify vertices

  ========================================
*/

#define		HASHTAB_SIZE		( 100 )

typedef struct hashvec3d_s {

	int			outvertexnum;
	struct hashvec3d_s	*next;

} hashvec3d_t;

hashvec3d_t		hashtab[ HASHTAB_SIZE ];

int			hashvec3dnum;
hashvec3d_t		hashvec3dheap[ MAX_VERTICES ];

void InitHashtab( void )
{
	int		i;

	printf( " init hash table\n" );

	for ( i = 0; i < HASHTAB_SIZE; i++ ) {
		hashtab[i].outvertexnum = -1;
		hashtab[i].next = NULL;
	}

	hashvec3dnum = 0;
}


int FindVertex( vec3d_t v )
{
	int		hashkey;
	hashvec3d_t	*hv;

//	hashkey = ((((int)(v[2]))+((int)(v[1]))+((int)(v[0]))) & ((HASHTAB_SIZE-1) << 3 )) >> 3;
	hashkey = (int)(((v[2]-p_min[2]) / (p_max[2]-p_min[2])) * (float)(HASHTAB_SIZE-1));
	hv = hashtab[ hashkey ].next;

	// search in list
	for ( ; hv ; hv=hv->next ) {
		
		if ( CheckSame( out_vertices[hv->outvertexnum], v ) )
			break;

	}

	if ( !hv ) {
		// vertex not found
		if ( out_vertexnum == MAX_VERTICES ) {
			Error("FindVertex: reached MAX_VERTICES.\n");
		}

		// new hashvec3d from heap

		hashvec3dheap[hashvec3dnum].next = hashtab[hashkey].next;
		hashvec3dheap[hashvec3dnum].outvertexnum = out_vertexnum;
		hashtab[hashkey].next = &hashvec3dheap[hashvec3dnum];
		hashvec3dnum++;

		Vec3dCopy( out_vertices[out_vertexnum], v );
		out_vertexnum++;

		return out_vertexnum - 1;
	}
	else {
		return hv->outvertexnum;
	}	
}

void UnifyPolygonVertices( polygon_t *p )
{
	int		i;
	for( i = 0; i < p->pointnum; i++ )
	{
		if ( out_facevertexnum >= MAX_FACEVERTICES )
			Error( "UnifyPolygonVertices: reached MAX_FACEVERTICES\n" );

		out_facevertices[out_facevertexnum] = FindVertex( p->p[i] );		
		out_facevertexnum++;
	}
}

void UnifyVertices( void )
{
	int		i;
	face_t		*f;

	printf( " - UnifyVertices -\n" );
	
	out_facevertexnum = 0;
	out_vertexnum = 0;
	InitHashtab();
	for ( i = 0; i < p_facenum; i++ )
	{
		f = &p_faces[i];
		f->startfacevertex = out_facevertexnum;
		UnifyPolygonVertices( f->fix );
		f->facevertexnum = out_facevertexnum - f->startfacevertex;
		if ( f->fix2 )
		{
			f->startfacevertex2 = out_facevertexnum;
			UnifyPolygonVertices( f->fix2 );
			f->facevertexnum2 = out_facevertexnum - f->startfacevertex2;			
		}
	}
	
	printf( " %d unique vertices\n", out_vertexnum );
	printf( " %d face vertices\n", out_facevertexnum );
}

/*
  ========================================
  FixTjunction test stuff

  ========================================
*/

static int		jpnum;
static vec3d_t		jpoly[1024];
static int		jtype;
static vec3d_t		jcenter;

static int		splitvertex1;
static int		splitvertex2;

void InitJPolyFromPolygon( polygon_t *p )
{
	int		i;

	PolygonCenter( p, jcenter );

	for ( i = 0; i < p->pointnum; i++ )
	{
		Vec3dCopy( jpoly[i], p->p[i] );
	}
	jpnum = i;
}

void InsertPointAt( int at, vec3d_t p )
{
	int		i;
	
	if ( at > jpnum-1 || at == 1024 )
		Error( "can't insert in jpoly.\n" );

	for ( i = jpnum-1; i >= at; i-- )
		Vec3dCopy( jpoly[i+1], jpoly[i] );
	jpnum++;

	Vec3dCopy( jpoly[at], p );
}

void CheckPoint( vec3d_t p )
{
	int		i;
	fp_t		*p1, *p2;
	
	for ( i = 0; i < jpnum; i++ )
	{
		p1 = jpoly[i];
		p2 = jpoly[(i+1 == jpnum) ? 0 : (i+1)];

		if ( CheckSame( p1, p ) || CheckSame( p2, p ) )
			return;
		
		if ( CheckCollinearEdge( p1, p2, p ) )
		{
			InsertPointAt( (i+1 == jpnum) ? 0 : (i+1), p );
			return;
		}
	}
}

void ClassifyJPoly( void )
{
	bool_t		clflag[1024]; // collinear flag
	int		i;
	fp_t		*p, *pprev, *pnext;
	int		clnum;
	bool_t		cl, clprev, clnext;

	fp_t		*pp, *ppp;
	fp_t		*ps, *pn, *pnn;
	int		num;

#if 1
	if ( jpnum == 3 )
		if ( CheckCollinearEdge( jpoly[0], jpoly[1], jpoly[2] ) )
		{
			printf( "degenerated triangle.\n" );
			jtype = -1;
			return;
		}
#endif		        
	
	clnum = 0;
	for ( i = 0; i < jpnum; i++ )
	{
		pprev = jpoly[(i-1)%jpnum];
		p = jpoly[i];
		pnext = jpoly[(i+1)%jpnum];

		if ( CheckCollinearEdge( pprev, pnext, p ) == true )
		{
			clflag[i] = true;
			clnum++;
		}
		else
			clflag[i] = false;
		
	}

	if ( clnum == 0 )
	{
//		printf( " no fix.\n" );
		jtype = -1;
		return;
	}

	// search a vertex that is not cl and that
	// has 2 neighbors which are not cl too

//	printf( "start fix.\n" );
	for ( i = 0; i < jpnum; i++ )
	{
#if 0
		clprev = clflag[((i-1) < 0) ? (jpnum-1) : (i-1)];
		cl = clflag[i];
		clnext = clflag[((i+1) == jpnum) ? 0 : (i+1)];

		if ( clprev == false && cl == false && clnext == false )
		{
			jtype = i;
			return;
		}
#else
		ppp = jpoly[((i-2)%jpnum)];
		pp = jpoly[((i-1)%jpnum)];
//		pprev = jpoly[((i-1) < 0) ? (jpnum-1) : (i-1)];
		p = jpoly[i];
//		pnext = jpoly[((i+1) == jpnum) ? 0 : (i+1)];
		pn = jpoly[((i+1)%jpnum)];
		pnn = jpoly[((i+2)%jpnum)];
		if ( CheckCollinearEdge( pp, pn, p ) == false )
		{
			if ( CheckCollinearEdge( ppp, p, pp ) == false &&
			     CheckCollinearEdge( p, pnn, pn ) == false )
			{
				jtype = i;
				return;	
			}
		}
		else
		{
			if ( CheckCollinearEdge( ppp, p, pp ) == false &&
			     CheckCollinearEdge( p, pnn, pn ) == false )
			{
				jtype = i;
				return;	
			}	
		}
#endif
	}


	// face has to be split	
	jtype = 1024;

	return;

	printf( "jpnum %d\n", jpnum );
	
	// 
	// search first split vertex
	// 
	for ( i = 0; i < jpnum; i++ )
	{
		p = jpoly[i];
		pn = jpoly[((i+1) == jpnum ) ? 0 : (i+1)];
		pnn = jpoly[((i+2) % jpnum )];
		
		if ( CheckCollinearEdge( p, pnn, pn ) == false )
		{
			splitvertex1 = i;
			break;
		}
	}
	if ( i == jpnum )
		Error( "can't find first split point.\n" );

	//
	// search end vertex
	//
	for ( i = splitvertex1, num = 0; num < jpnum; num++, i++ )
	{
		i = ((i+1)==jpnum) ? 0 : (i+1);
		
		ps = jpoly[splitvertex1];
		pn = jpoly[((i+1)==jpnum) ? 0 : (i+1)];
		pnn = jpoly[((i+2) % jpnum )];

#if 1
		if ( pn == ps )
		{
			printf( "*" );
			splitvertex2 = (i-1)%jpnum;
			break;
		}
#endif  
		if ( CheckCollinearEdge( pn, ps, pnn ) == true )
		{
			splitvertex2 = i;
			break;
		}
	}
	if ( num == jpnum )
		Error( "can't find second split point.\n" );

	printf( "sv1 %d, sv2 %d\n", splitvertex1, splitvertex2 );
}

polygon_t* PolygonFromJPoly( void )
{
	polygon_t	*p;
	int		start;
	int		i;


	if ( jtype == -1 )
	{
		start = 0;
	}
	else if ( jtype == 1024 )
	{
		start = 0;
		InsertPointAt( 0, jcenter );	
		// the first point of the polygon
		// is its center
	}
	else 
	{
		start = jtype;
	}

	p = NewPolygon( jpnum );
	p->pointnum = jpnum;

	for ( i = 0; i < jpnum; start++, i++ )
	{
		start = (start == jpnum ) ? 0 : start;
		Vec3dCopy( p->p[i], jpoly[start] );
	}
	
	return p;
}

void PolygonsFromJPoly( polygon_t **p1, polygon_t **p2 )
{
	int		i, num;

	if ( jtype != 1024 )
		Error( "only one polygon.\n" );
	
	// count points of p1
	for ( i = splitvertex1, num = 0; i != splitvertex2; (i = ((i+1)==jpnum) ? 0 : (i+1)), num++ )
	{ }
	num++;

	printf( "num1 %d\n", num );	
	*p1 = NewPolygon( num );
	(*p1)->pointnum = num;
	// build p1
	for ( i = splitvertex1, num = 0; ; i = ((i+1)==jpnum) ? 0 : (i+1), num++ )
	{ 
		Vec3dCopy( (*p1)->p[num], jpoly[i] );

		if ( i == splitvertex2 )
			break;
	}
		
	// count points of p2
	
	for ( i = splitvertex2, num = 0; i != splitvertex1; (i = ((i+1)==jpnum) ? 0 : (i+1)), num++ )
	{ }
	num++;

	printf( "num2 %d\n", num );	
	*p2 = NewPolygon( num );
	(*p2)->pointnum = num;
	// build p2
	for ( i = splitvertex2, num = 0; ; i = ((i+1)==jpnum) ? 0 : (i+1), num++ )
	{ 
		Vec3dCopy( (*p2)->p[num], jpoly[i] );

		if ( i == splitvertex1 )
			break;
	}
}

void Test_FixTjunctions( void )
{
	int		i, j, m;
	int		k;
	int		count, count2;
	face_t		*f, *f2;

	int		trinum, trinum2;
	int		addnum;

	int		nofixnum = 0, splitnum = 0, startnum = 0;

	printf( " - Test_FixTjunctions -\n" );

	count2 = count = 0;
	trinum2 = trinum = 0;
	for ( i = 0; i < p_facenum; i++ )
	{
		f = &p_faces[i];

//		printf( "in num: %d\n", f->p->pointnum );
		InitJPolyFromPolygon( f->p );
		
//		count2 += f->p->pointnum;
		count++;
		trinum += ( jpnum - 2 );

		addnum = 0;
		for ( j = 0; j < p_facenum; j++ )
		{			
			if ( i == j )
				continue;

			f2 = &p_faces[j];
			for ( k = 0; k < 3; k++ )
			{
				if ( f->min[k] > f2->max[k] || f->max[k] < f2->min[k] )
					break;
			}
			if ( k != 3 )
				continue;
			

			for ( m = 0; m < f2->p->pointnum; m++ )
			{
				CheckPoint( f2->p->p[m] );
			}
		}
//		printf( "jpoly: %d\n", jpnum );

		trinum2 += ( jpnum-2 );
		if ( jpnum == f->p->pointnum ) count2++;

		ClassifyJPoly();
		f->type = jtype;
//		if ( jtype != 1024 )
//		{
			f->fix = PolygonFromJPoly();
//			f->fix2 = NULL;
//		}		
//		else
//		{
//			PolygonsFromJPoly( &f->fix, &f->fix2 );
//		}

		if ( jtype == -1 )
			nofixnum++;
		else if ( jtype == 1024 )
			splitnum++;
		else
			startnum++;
	}

	printf( " fixed faces %d of %d\n", count2, count );
	printf( " trinum from %d to %d\n", trinum, trinum2 );
	printf( " %d nofixnum, %d splitnum, %d startnum\n", nofixnum, splitnum, startnum );
}


/*
  ========================================
  write

  ========================================
*/
void Write_Vertices( char *name )
{
	FILE		*h;
	int			i;

	printf( " write vertices ...\n" );

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_Vertices: can't open file.\n" );

	fprintf( h, "# unified vertex file. generated by fface.\n" );
	fprintf( h, "# <vertexnum> <vec3d_t>\n" );

	for ( i = 0; i < out_vertexnum; i++ )
	{
		fprintf( h, "%d ( %f %f %f )\n", i, out_vertices[i][0], out_vertices[i][1], out_vertices[i][2] );
	}
	fprintf( h, "end" );
	fclose( h );
}

void Write_Faces( char *name )
{
	FILE		*h;
	int		i;
	face_t		*f;
	int		num;

	printf( " write faces ...\n" );

	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_Faces: can't open file.\n" );

	fprintf( h, "# fixface file. generated by fface.\n" );
	fprintf( h, "# <facenum> <jtype> <startfacevertex> <facevertexnum>\n" );
	
	for ( num = 0, i = 0; i < p_facenum; i++ )
	{
		f = &p_faces[i];

		fprintf( h, "%d %d %d %d\n", num++, -1, f->startfacevertex, f->facevertexnum );
		
		if ( f->fix2 )
		{
			fprintf( h, "%d %d %d %d\n", num++, -1, f->startfacevertex2, f->facevertexnum2 );	
		}

	}
	fprintf( h, "end" );
	fclose( h );
}

void Write_Facevertices( char *name )
{
	FILE		*h;
	int		i;

	printf( " write facevertices ...\n" );
	
	h = fopen( name, "w" );
	if ( !h )
		Error( "Write_Facevertices: can't open file.\n" );

	fprintf( h, "# facevertex file. generated by fface.\n" );
	fprintf( h, "# <facevertexnum> <vertex ref>\n" );

	for ( i = 0; i < out_facevertexnum; i++ )
	{
		fprintf( h, "%d %d\n", i, out_facevertices[i] );
	}
	fprintf( h, "end" );
	fclose( h );
}

void ShowHelp( void )
{
	puts( "usage:" );
	puts( "-i input file, lightfaces.asc from process2" );
	puts( "-vertex output file with unique vertices" );
	puts( "-fixface output file with fixed faces" );
	puts( "-facevertex output file with face vertices" );
	puts( "[-dist float] max distance two vertices are merged" );	
}

int main( int argc, char *argv[] )
{
	char	*in_face;
	char	*out_vertex;
	char	*out_face;
	char	*out_facevertex;

	printf( "===== fface - find unique vertieces and fix tjunctions =====\n" );

	SetCmdArgs( argc, argv );

	in_face = GetCmdOpt2( "-i" );
	out_vertex = GetCmdOpt2( "-vertex" );
	out_face = GetCmdOpt2( "-fixface" );
	out_facevertex = GetCmdOpt2( "-facevertex" );

	if ( CheckCmdSwitch2( "--help" ) )
	     ShowHelp();

	if ( CheckCmdSwitch2( "-dist" ) )
	{
		equal_dist = fabs( atof( GetCmdOpt2( "-dist" ) ) );
	}
	else
	{
		equal_dist = 0.5;
	}

	if ( !in_face )
		Error( "no input face file.\n" );
	if ( !out_vertex )
		Error( "no output vertex file.\n" );
	if ( !out_face )
		Error( "no output face file.\n" );
	if ( !out_facevertex ) 
		Error( "no output face vertex file.\n" );

	printf( " input face file: %s\n", in_face );
	printf( " output vertex file: %s\n", out_vertex );
	printf( " output fixface file: %s\n", out_face );
	printf( " output face vertex file: %s\n", out_facevertex );
	printf( " merge dist: %f\n", equal_dist );

	Read_Faces( in_face );
	Test_FixTjunctions();
	UnifyVertices();
	Write_Vertices( out_vertex );
	Write_Faces( out_face );
	Write_Facevertices( out_facevertex );

}
