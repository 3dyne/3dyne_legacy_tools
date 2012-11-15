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



// shapetjfix.c

#include "shapetjfix.h"

fp_t		equal_dist;

int		out_vertexnum = 0;

int		out_facevertexnum = 0;

vec3d_t		p_min, p_max;

bool_t CheckCollinearEdge_old( vec3d_t p1, vec3d_t p2, vec3d_t t )
{
	vec3d_t		v;
	fp_t		d, d1, d2;

	Vec3dSub( v, p2, p1 );
	d = Vec3dLen( v );

	Vec3dSub( v, t, p1 );
	d1 = Vec3dLen( v );

	Vec3dSub( v, t, p2 );
	d2 = Vec3dLen( v );

	if ( fabs( (d1+d2) - d ) < 0.5 )
		return true;
	
	return false;
}

bool_t CheckCollinearEdge_old2( vec3d_t p1, vec3d_t p2, vec3d_t t )
{
	vec3d_t		v1, v2;

	Vec3dSub( v1, p1, t );
	Vec3dSub( v2, t, p2 );
	Vec3dUnify( v1 );
	Vec3dUnify( v2 );
	if ( Vec3dDotProduct( v1, v2 ) > 0.999 )
		return true;
	return false;
}

bool_t  CheckCollinearEdge( vec3d_t p1, vec3d_t p2, vec3d_t t )                                        
{                                                                                                            
        int             i;                                                                                   
        double          v1[3], v2[3];                                                                        
        double          len, len2, scale, dot;                                                                     
                                                                                                             
        for ( i = 0; i < 3; i++ )                                                                            
        {                                                                                                    
                v1[i] = p1[i]-t[i];                                                                          
                v2[i] = t[i]-p2[i];                                                                          
        }                                                                                                    
                                                                                                             
        len = sqrt( v1[0]*v1[0] + v1[1]*v1[1] + v1[2]*v1[2] );                                               
        scale = 1.0 / len;                                                                                   
//        if ( len < 0.1 ) return true;                                                                        
                                                                                                             
        v1[0] *= scale;                                                                                      
        v1[1] *= scale;                                                                                      
        v1[2] *= scale;                                                                                      
                                                                                                             
        len2 = sqrt( v2[0]*v2[0] + v2[1]*v2[1] + v2[2]*v2[2] );                                               
        scale = 1.0 / len2;                                                                                   

	if ( len2 < 1.0 || len < 1.0 )
		printf ( "( %f, %f) ", len, len2 );
                                                                                                             
//        if ( len2 < 0.1 ) return true;                                                                        
                                                                                                             
        v2[0] *= scale;                                                                                      
        v2[1] *= scale;                                                                                      
        v2[2] *= scale;                                                                                      
                                                                                                             
        dot = v1[0]*v2[0] + v1[1]*v2[1] + v1[2]*v2[2];                                                       
                                                                                                             
        if ( dot > 0.999999 )                                                                                   
                return true;                                                                                 
        return false;                                                                                        
}                                                                                                            


bool_t CheckSame( vec3d_t p1, vec3d_t p2 )
{
	int		i;

	for ( i = 0; i < 3; i++ )
	{
		if ( fabs( p1[i]-p2[i] ) > 0.1 )
			return false;
	}
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
  ========================================
  unify vertices

  ========================================
*/

#define NEW( x )	( (x *)(memset( (malloc(sizeof(x)) ), 0, sizeof(x) ) ) )

#define		HASHTAB_SIZE		( 100 )

typedef struct hashvec3d_s {

	hobj_t			*self;
	vec3d_t			v;
	struct hashvec3d_s	*next;

} hashvec3d_t;

hashvec3d_t		*hashtab[ HASHTAB_SIZE ];

hashvec3d_t * NewHashvec( void )
{
	out_vertexnum++;
	return NEW( hashvec3d_t );
}

void FreeHashvec( hashvec3d_t *hv )
{
	out_vertexnum--;
	free( hv );
}

void InitHashtab( void )
{
	int		i;

	printf( " init hash table\n" );

	for ( i = 0; i < HASHTAB_SIZE; i++ ) {
		hashtab[i] = NULL;
	}
}


hobj_t * BuildVertexClass( vec3d_t v )
{
	hobj_t		*vertex;
	hpair_t		*pair;
	char		tt[256];

	sprintf( tt, "#%u", HManagerGetFreeID() );
	vertex = NewClass( "vertex", tt );
	
	sprintf( tt, "%f %f %f", v[0], v[1], v[2] );
	pair = NewHPair2( "vec3d", "point", tt );
	InsertHPair( vertex, pair );

	return vertex;
}

hobj_t *FindVertex( vec3d_t v, hobj_t *vertexcls )
{
	int		hashkey;
	hashvec3d_t	*hv;

//	hashkey = ((((int)(v[2]))+((int)(v[1]))+((int)(v[0]))) & ((HASHTAB_SIZE-1) << 3 )) >> 3;
	hashkey = (int)(((v[2]-p_min[2]) / (p_max[2]-p_min[2])) * (float)(HASHTAB_SIZE-1));
	hv = hashtab[ hashkey ];

	// search in list
	for ( ; hv ; hv=hv->next ) {
		
		if ( CheckSame( hv->v, v ) )
			break;

	}

	if ( !hv ) {
		// vertex not found

		hv = NewHashvec();

		hv->next = hashtab[hashkey];
		hashtab[hashkey] = hv;

		Vec3dCopy( hv->v, v );

		hv->self = BuildVertexClass( v );
		InsertClass( vertexcls, hv->self );
	}

	return hv->self;
}

hobj_t * BuildFixPolygonClass( polygon_t *p, hobj_t *vertexcls )
{
	hobj_t		*fixpoly;
	hpair_t		*pair;
	char		tt[256];
	int		i;

	sprintf( tt, "#%u", HManagerGetFreeID() );
	fixpoly = NewClass( "fixpolygon", tt );

	sprintf( tt, "%d", p->pointnum );
	pair = NewHPair2( "int", "pointnum", tt );
	InsertHPair( fixpoly, pair );

	for( i = 0; i < p->pointnum; i++ )
	{
		sprintf( tt, "%d", i );
		pair = NewHPair2( "ref", tt, FindVertex( p->p[i], vertexcls )->name );
		InsertHPair( fixpoly, pair );
	}

	return fixpoly;
}

void UnifyVertices( face_t *list, char *vertex_name )
{
	face_t		*f;
	hobj_t		*vertexcls;
	FILE		*h;
	hpair_t		*pair;
	hobj_t		*poly;

	printf( "unify vertices ...\n" );
	
	vertexcls = NewClass( "vertices", "vertices0" );

	out_facevertexnum = 0;
	out_vertexnum = 0;
	InitHashtab();
	for ( f = list; f ; f=f->next )
	{
		pair = FindHPair( f->self, "lightdef" );
		// copy lightdef

		InsertClass( f->surface, poly = BuildFixPolygonClass( f->fix, vertexcls ) );		
		if ( pair )
		{
			pair = NewHPair2( pair->type, pair->key, pair->value );
			InsertHPair( poly, pair );
		}

		if ( f->fix2 )
		{
			InsertClass( f->surface, poly = BuildFixPolygonClass( f->fix2, vertexcls ) );	
			if ( pair )
			{
				pair = NewHPair2( pair->type, pair->key, pair->value );
				InsertHPair( poly, pair );	
			}
		}
	}
	
	printf( " %d unique vertices\n", out_vertexnum );
	printf( " %d face vertices\n", out_facevertexnum );

	printf( "write vertex class ...\n" );
	h = fopen( vertex_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( vertexcls, h );
	fclose( h );
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



polygon_t* PolygonFromJPoly( void )
{
	polygon_t	*p;
	int		start;
	int		i;

	p = NewPolygon( jpnum );
	p->pointnum = jpnum;

	for ( i = 0; i < jpnum; i++ )
	{
		Vec3dCopy( p->p[i], jpoly[i] );
	}
	
	return p;
}



void PolygonListFixTjunctions( u_list_t *list )
{
	int		m;
	int		k;
	int		count, count2;

	int		trinum, trinum2;
	int		addnum;

	u_list_iter_t	iter1;
	u_list_iter_t	iter2;

	fixpoly_t	*fix1;
	fixpoly_t	*fix2;

	count2 = count = 0;
	trinum2 = trinum = 0;

	printf( "fixing tjunctions ...\n" );

	U_ListIterInit( &iter1, list );
	for ( ; ( fix1 = U_ListIterNext( &iter1 ) ) ; )
	{
//		printf( "in num: %d\n", f->p->pointnum );
		InitJPolyFromPolygon( fix1->p );
		
//		count2 += f->p->pointnum;
		count++;
		trinum += ( jpnum - 2 );

		addnum = 0;
//		for ( f2 = list; f2 ; f2=f2->next )
//		{			
		U_ListIterInit( &iter2, list );
		for ( ; ( fix2 = U_ListIterNext( &iter2 ) ) ; )
		{
			if ( fix1 == fix2 )
				continue;

			for ( k = 0; k < 3; k++ )
			{
				if ( fix1->min[k] > fix2->max[k] || fix1->max[k] < fix2->min[k] )
					break;
			}
			if ( k != 3 )
				continue;
			

			for ( m = 0; m < fix2->p->pointnum; m++ )
			{
				CheckPoint( fix2->p->p[m] );
			}
		}
//		printf( "jpoly: %d\n", jpnum );

		trinum2 += ( jpnum-2 );
		if ( jpnum == fix1->p->pointnum ) count2++;

		fix1->fix = PolygonFromJPoly();

	}

	printf( " fixed faces %d of %d\n", count2, count );
	printf( " trinum from %d to %d\n", trinum, trinum2 );
}


/*
  =======================================
  write

  ========================================
*/

/*
  ==================================================
  read classes

  ==================================================
*/
face_t * NewFaceFromPolygonClass( hobj_t *poly )
{
	polygon_t	*p, *p2;
	hpair_t		*pair;
	int		pointnum;
	int		i;
	face_t		*f;
	char		tt[256];

	f = (face_t *) malloc( sizeof( face_t ) );
	memset( f, 0, sizeof( face_t ) );

	Vec3dInitBB( f->min, f->max, 999999.9 );
	
	pair = FindHPair( poly, "num" );
	if ( !pair )
		Error( "missing polygon pointnum.\n" );
	HPairCastToInt_safe( &pointnum, pair );
	p = NewPolygon( pointnum );
	p->pointnum = pointnum;
	for ( i = 0; i < pointnum; i++ )
	{
		sprintf( tt, "%d", i );
		pair = FindHPair( poly, tt );
		if ( !pair )
			Error( "missing point.\n" );
		HPairCastToVec3d_safe( p->p[i], pair );
		Vec3dAddToBB( f->min, f->max, p->p[i] );
		Vec3dAddToBB( p_min, p_max, p->p[i] );
	}	

	p2 = PolygonRemoveEqualPoints( p );
	if ( p->pointnum != p2->pointnum )
	{
//		printf( "removed equal points !\n" );
		FreePolygon( p );
		f->p = p2;
		f->fix = p2;	// hack
	}
	else
	{
		FreePolygon( p2 );
		f->p = p;
		f->fix = p;	// hack
	}

	// calc plane
	PlaneFromPolygon( f->p, f->norm, &f->dist );
	
	// expand bb by equal_dist
	for ( i = 0; i < 3; i++ )
	{
		f->min[i] -= equal_dist;
		f->max[i] += equal_dist;
	}

	return f;
}


/*
  ==================================================
  
  
  ==================================================
*/

/*
  ==============================
  BuildPolygonList

  ==============================
*/
u_list_t * BuildPolygonList( hmanager_t *shapehm )
{
	u_list_t	*list;
	hobj_search_iterator_t		iter;
	hobj_t			*shape;
	hobj_t			*polygon;
	hpair_t		*pair;

	list = U_NewList();

	InitClassSearchIterator( &iter, HManagerGetRootClass( shapehm ), "shape" );
	for ( ; ( shape = SearchGetNextClass( &iter ) ) ; )
	{
		fixpoly_t	*fix;

		pair = FindHPair( shape, "tess_name" );
		if ( !pair )
			Error( "missing key 'tess_name' in shape '%s'\n", shape->name );

		if ( !strcmp( pair->value, "sface" ) )
		{
			// ok
		}
		else
		{
			continue;
		}

		polygon = FindClassType( shape, "polygon" );
		
		fix = NEW( fixpoly_t );
		fix->self = polygon;
		U_ListInsertAtHead( list, fix );
	}
	
	return list;
}

/*
  ==============================
  SetupPolygonList

  ==============================
*/
void SetupPolygonList( u_list_t *list )
{
	u_list_iter_t		iter;
	fixpoly_t		*fix;
	hpair_t		*pair;

	U_ListIterInit( &iter, list );
	for ( ; ( fix = U_ListIterNext( &iter ) ) ; )
	{
		int		i;
		int		pointnum;
		hobj_t		*polycls;
		polygon_t	*p, *p2;
		char		tt[256];

		polycls = fix->self;
		Vec3dInitBB( fix->min, fix->max, 999999.9 );

		// init polygon_t
		EasyFindInt( &pointnum, polycls, "num" );
		p = NewPolygon( pointnum );
		p->pointnum = pointnum;

		for ( i = 0; i < pointnum; i++ )
		{
			sprintf( tt, "%d", i );
			EasyFindVec3d( p->p[i], polycls, tt );
			Vec3dAddToBB( fix->min, fix->max, p->p[i] );			
		}
		
		p2 = PolygonRemoveEqualPoints( p );
		if ( p2->pointnum == p->pointnum )
		{
			FreePolygon( p2 );
			fix->p = p;
		}
		else
		{
			FreePolygon( p );
			fix->p = p2;
		}

		PlaneFromPolygon( fix->p, fix->norm, &fix->dist );

		// expand bb by equal_dist
		for ( i = 0; i < 3; i++ )
		{
			fix->min[i] -= equal_dist;
			fix->max[i] += equal_dist;
		}		
	}
}

/*
  ==============================
  BuildPolygonClass

  ==============================
*/
void UpdatePolygonClass( u_list_t *list )
{
	u_list_iter_t		iter;
	fixpoly_t		*fix;
	hpair_t		*pair;
	char		tt[256];
			

	U_ListIterInit( &iter, list );
	for ( ; ( fix = U_ListIterNext( &iter ) ) ; )
	{
		polygon_t	*p;
		hobj_t		*polygon;
		int		i, num;
		vec3d_t		tmp;

		polygon = fix->self;

		if ( !fix->fix )
			continue;

		// remove old points
		pair = EasyFindInt( &num, polygon, "num" );
		RemoveHPair( polygon, pair );
		for ( i = 0; i < num; i++ )
		{
			sprintf( tt, "%d", i );	
			pair = EasyFindVec3d( tmp, polygon, tt );
			RemoveHPair( polygon, pair );
		}

		// insert new points
		p = fix->fix;

		EasyNewInt( polygon, "num", p->pointnum );
		for ( i = 0; i < p->pointnum; i++ )
		{
			sprintf( tt, "%d", i );
			EasyNewVec3d( polygon, tt, p->p[i] );
		}
	}	
}

void ShowHelp( void )
{

}

int main( int argc, char *argv[] )
{
	char	*in_shape_name;
	char	*out_shape_name;
//	char	*out_vertex_name;

	hmanager_t	*shapehm;
	u_list_t	*poly_list;

	FILE		*h;

	printf( "===== shapetjfix - find unique vertieces and fix tjunctions =====\n" );

	SetCmdArgs( argc, argv );
	Vec3dInitBB( p_min, p_max, 999999.9 );

	in_shape_name = GetCmdOpt2( "-i" );	
	out_shape_name = GetCmdOpt2( "-o" );

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

	if ( !in_shape_name )
	{
		Error( "no input shape class\n" );
	}
	else
	{
		printf( " input shape class: %s\n", in_shape_name );
	}

	if ( !out_shape_name )
	{
		Error( "no output shape class\n" );
	}
	else
	{
		printf( " output shape class: %s\n", out_shape_name );
	}

	printf( "load shape class ...\n" );
	if ( !(shapehm = NewHManagerLoadClass( in_shape_name ) ) )
		Error( "failed\n" );

	poly_list = BuildPolygonList( shapehm );

	printf( " %d input polygons\n", U_ListLength( poly_list ) );
	SetupPolygonList( poly_list );
	PolygonListFixTjunctions( poly_list );
	UpdatePolygonClass( poly_list );

	printf( "write output shape class ...\n" );
	h = fopen( out_shape_name, "w" );
	if ( !h )
		Error( "failed\n" );
	WriteClass( HManagerGetRootClass( shapehm ), h );
	fclose( h );
	
	HManagerSaveID();

	exit(0);
}
