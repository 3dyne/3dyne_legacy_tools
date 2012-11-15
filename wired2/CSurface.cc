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



// CSurface.cc

#include "CSurface.hh"
#include "lib_bezier.h"

// support
static fp_t DistStraightToPoint( vec3d_t start, vec3d_t dir, vec3d_t p )
{
	vec3d_t		delta;
	vec3d_t		norm, norm2;
	fp_t		dist, d;

	Vec3dSub( delta, p, start );
	Vec3dUnify( delta );
	Vec3dCrossProduct( norm, delta, dir );
	Vec3dCrossProduct( norm2, norm, dir );
	Vec3dUnify( norm2 );
	dist = Vec3dDotProduct( start, norm2 );
	d = Vec3dDotProduct( p, norm2 ) - dist;
//	printf( "*%f\n", d );

	return d;
}


/*
  ==================================================
  class: CSurface

  ==================================================
*/

CSurface::CSurface( int usize, int vsize )
{
	upointnum = usize;
	vpointnum = vsize;
	ctrlpoints = NULL;
	select = 0;

	self_id = UNIQUE_INVALIDE;

#if 0
	Vec2dInit( td.vecs[0], 1, 0 );
	Vec2dInit( td.vecs[1], 0, 1 );
	Vec2dInit( td.ofs, 0, 0 );
	Vec2dInit( td.len, 64, 64 );
#endif
}

CSurface::~CSurface()
{
	printf( "CSurface::~CSurface\n" );

	// free ctrlpoint list
	CtrlPoint	*cs, *next;
	for ( cs = ctrlpoints; cs ; cs=next )
	{
		next = cs->getNext();
		delete cs;
	}
}

void CSurface::setID( unique_t id )
{
	self_id = id;
}

unique_t CSurface::getID( void )
{
	return self_id;
}


CSurface* CSurface::getNext( void )
{
	return next;
}

void CSurface::setNext( CSurface *_next )
{
	next = _next;
}

void CSurface::checkViewBounds( vec3d_t viewmin, vec3d_t viewmax )
{
	int		i;
	
	select |= SELECT_VISIBLE;

	for ( i = 0; i < 3; i++ )
	{
		if ( max[i] < viewmin[i] || min[i] > viewmax[i] )
		{
			select &= ~SELECT_VISIBLE;
			break;
		}
	}	
}

bool CSurface::isVisible( void )
{
	return select & SELECT_VISIBLE;
}

void CSurface::getBB( vec3d_t _min, vec3d_t _max )
{
	Vec3dCopy( _min, min );
	Vec3dCopy( _max, max );	
}

bool CSurface::intersectWithRay( vec3d_t start, vec3d_t dir )
{
	fp_t		d1, d2;
	vec3d_t		delta;

	d1 = DistStraightToPoint( start, dir, min );
	d2 = DistStraightToPoint( start, dir, max );
	Vec3dSub( delta, max, min );
	
	if ( Vec3dLen( delta ) >= fabs(d1)+fabs(d2) )
	{
//		printf( "intersect\n" );
		return true;
	}

//	printf( "%f %f\n", d1, d2 );
	return false;	
}

void CSurface::setSelect( int _select )
{
	select = _select;
}

int CSurface::getSelect( void )
{
	return select;
}

bool CSurface::testFlagSelect( int _flag )
{
	return select & _flag;
}

void CSurface::setFlagSelect( int _flag )
{
	select |= _flag;
}

void CSurface::resetFlagSelect( int _flag )
{
	select &= ~_flag;
}

CSurface* CSurface::copySelf( void )
{
	CSurface		*csnew;

	csnew = new CSurface( 0, 0 );

	// copy static
	csnew->upointnum = this->upointnum;
	csnew->vpointnum = this->vpointnum;
	memcpy( &csnew->td, &this->td, sizeof( cstexdef_t ) );
	memcpy( &csnew->texelpoints, &this->texelpoints, sizeof( texelpoints ) ); // fixme: remove
	
	// copy ctrlpoints
	csnew->ctrlpoints = NULL;
	CtrlPointIterator	cpi( this->ctrlpoints );
	CtrlPoint		*cp, *cpnew;

	for( cpi.reset(); ( cp = cpi.getNext() ); )
	{
		cpnew = cp->copySelf();
		cpnew->setNext( csnew->ctrlpoints );
		csnew->ctrlpoints = cpnew;
	}

	csnew->calcBB();
	csnew->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE;
	return csnew;
}


void CSurface::moveSelf( vec3d_t from, vec3d_t to )
{
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

//	printf( "TestBox::moveSelf\n" );
	for ( iter.reset(); (cp = iter.getNext()); )
	{
		cp->moveSelf( from, to );
	}

	calcBB();

	select |= SELECT_UPDATE;	

#if 0
	// hack: texel vec tes
	vec2d_t		vecs[2];
	vec2d_t		len;
	vec2d_t		ofs;
	Vec2dSub( vecs[0], texelpoints[1][0], texelpoints[0][0] );
	Vec2dSub( vecs[1], texelpoints[0][1], texelpoints[0][0] );
	
	len[0] = Vec2dLen( vecs[0] );
	len[1] = Vec2dLen( vecs[1] );

	Vec2dCopy( ofs, texelpoints[0][0] );
	Vec2dUnify( vecs[0], vecs[0] );
	Vec2dUnify( vecs[1], vecs[1] );

	printf( "csurface texel vecs:\n" );
	printf( " u: " ); Vec2dPrint( td.vecs[0] );
	printf( " v: " ); Vec2dPrint( td.vecs[1] );
	printf( " ulen, vlen: " ); Vec2dPrint( td.len );
	printf( " uofs, vofs: " ); Vec2dPrint( td.ofs );
#endif	
}

void CSurface::calcBB( void )
{
	CtrlPointIterator	iter( ctrlpoints );

	EAL_CalcTotalBB( &iter, min, max );
}



/*
  ====================
  insertCtrlPoint

  ====================
*/
void CSurface::insertCtrlPoint( CtrlPoint *obj )
{
	printf( "CSurface::insertCtrlPoint\n" );
	
	obj->setNext( ctrlpoints );
	ctrlpoints = obj;

	obj->setSelect( SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE );

	calcBB();
}



/*
  ====================
  removeCtrlPoint

  ====================
*/
void CSurface::removeCtrlPoint( CtrlPoint *obj )
{
	CtrlPoint		*head, *c, *next;

	printf( "CSurface::removeCtrlPoint\n" );
	
	head = NULL;
	for ( c = ctrlpoints; c ; c=next )
	{
		next = c->getNext();

		if ( c == obj )
		{
			c->setNext( NULL );
			continue;
		}
		c->setNext( head );
		head = c;
	}
	ctrlpoints = head;

	calcBB();
}



/*
  ====================
  getFirstCtrlPoint

  ====================
*/
CtrlPoint* CSurface::getFirstCtrlPoint( void )
{
	return ctrlpoints;
}



/*
  ====================
  generateUVMesh

  ====================
*/
uvmesh_t* CSurface::generateUVMesh( int ustepnum, int vstepnum )
{
	surface_ctrl_t			*sc;
	CtrlPointIterator		iter( ctrlpoints );
	CtrlPoint			*cp;
	int				cpnum;
	vec3d_t				p;
	int				u, v;
	surface_points_t		*sp;
	uvmesh_t			*mesh;

	sc = NewBezierSurface( upointnum, vpointnum );
	for ( cpnum = 0, iter.reset(); (cp = iter.getNext()); cpnum++ )
	{
		cp->getPos( p, &u, &v );
		SetSurfaceCtrlPoint( sc, u, v, p );
	}

	if ( cpnum != upointnum*vpointnum )
	{
		printf( "CSurface::generateUVMesh: *** internal inconsistency ***\n" );
		printf( " found not enough CtrlPoints for CSurface ( %d insted of %d )\n", cpnum, upointnum*vpointnum );
		FreeBezierSurface( sc );
		return NULL;
	}
	
	sp = EvalSurfacePoints( sc, ustepnum, vstepnum );
	mesh = NewUVMesh( ustepnum, vstepnum );
	for ( u = 0; u < ustepnum; u++ )
		for ( v = 0; v < vstepnum; v++ )
		{
			GetSurfacePoint( sp, u, v, p );
			SetUVMeshPoint( mesh, u, v, p );
		}

	FreeSurfacePoints( sp );
	FreeBezierSurface( sc );

	return mesh;
}



/*
  ====================
  setTexelCtrlPoint

  ====================
*/
void CSurface::setTexelCtrlPoint( int u, int v, vec2d_t _pos )
{
	Vec2dCopy( texelpoints[u][v], _pos );
}



/*
  ====================
  getTexelCtrlPoint

  ====================
*/
void CSurface::getTexelCtrlPoint( int u, int v, vec2d_t _pos )
{
	Vec2dCopy( _pos, texelpoints[u][v] );
}



/*
  ====================
  setTexdef

  ====================
*/
void CSurface::setTexdef( cstexdef_t *_td )
{
	memcpy( &td, _td, sizeof( cstexdef_t ) );
}



/*
  ====================
  getTexdef

  ====================
*/
void CSurface::getTexdef( cstexdef_t *_td )
{
	memcpy( _td, &td, sizeof( cstexdef_t ) );
}





/*
  ==============================
  buildClass

  ==============================
*/
hobj_t * CSurface::buildClass( void )
{
	hobj_t		*csurf;
	hpair_t		*pair;
	char		tt[256];

	sprintf( tt, "#%u", self_id );
	csurf = NewClass( "csurface", tt );
	
	// upointnum
	sprintf( tt, "%d", upointnum );
	pair = NewHPair2( "int", "upointnum", tt );
	InsertHPair( csurf, pair );

	// vpointnum
	sprintf( tt, "%d", vpointnum );
	pair = NewHPair2( "int", "vpointnum", tt );
	InsertHPair( csurf, pair );

	// ident
	pair = NewHPair2( "string", "ident", td.ident );
	InsertHPair( csurf, pair );

	// shift
	sprintf( tt, "%f %f", td.shift[0], td.shift[1] );
	pair = NewHPair2( "vec2d", "shift", tt );
	InsertHPair( csurf, pair );

	// vec0
	sprintf( tt, "%f %f", td.vecs[0][0], td.vecs[0][1] );
	pair = NewHPair2( "vec2d", "vec0", tt );
	InsertHPair( csurf, pair );

	// vec1
	sprintf( tt, "%f %f", td.vecs[1][0], td.vecs[1][1] );
	pair = NewHPair2( "vec2d", "vec1", tt );
	InsertHPair( csurf, pair );

	// vec2
	sprintf( tt, "%f %f", td.vecs[2][0], td.vecs[2][1] );
	pair = NewHPair2( "vec2d", "vec2", tt );
	InsertHPair( csurf, pair );
	
	// scale
	sprintf( tt, "%f %f", td.scale[0], td.scale[1] );
	pair = NewHPair2( "vec2d", "scale", tt );
	InsertHPair( csurf, pair );

	// CtrlPoints
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

	for ( iter.reset(); (cp = iter.getNext()); )
	{
		pair = cp->buildHPair();
		InsertHPair( csurf, pair );
	}

	return csurf;
}

/*
  ==============================
  initFromClass

  ==============================
*/
bool CSurface::initFromClass( hobj_t *cls )
{
	int		u, v;
	char		tt[256];

	self_id = StringToUnique( cls->name );

	EasyFindInt( &upointnum, cls, "upointnum" );
	EasyFindInt( &vpointnum, cls, "vpointnum" );

	EasyFindString( td.ident, cls, "ident" );
	EasyFindVec2d( td.shift, cls, "shift" );
	EasyFindVec2d( td.scale, cls, "scale" );
	EasyFindVec2d( &td.vecs[0][0], cls, "vec0" );
	EasyFindVec2d( &td.vecs[1][0], cls, "vec1" );
	EasyFindVec2d( &td.vecs[2][0], cls, "vec2" );

	ctrlpoints = NULL;
	for ( u = 0; u < upointnum; u++ )
	{
		for ( v = 0; v < vpointnum; v++ )
		{
			CtrlPoint	*cp;
			vec3d_t		pos;

			sprintf( tt, "u%d_v%d", u, v );
			
			EasyFindVec3d( pos, cls, tt );
		      			
			cp = new CtrlPoint();
			cp->setPos( pos, u, v );
			cp->setNext( ctrlpoints );
			ctrlpoints = cp;
			cp->init();
		}
	}

	calcBB();
	select |= SELECT_UPDATE;

	return true;	
}

/*
  ==================================================
  class: CSurfaceChecker

  ==================================================
*/
bool CSurfaceChecker_always::isOk( CSurface * )
{
	return true;
}

CSurfaceChecker_select::CSurfaceChecker_select( int _flag )
{
	flag = _flag;
}

bool CSurfaceChecker_select::isOk( CSurface *obj )
{
	if ( obj->getSelect() & flag )
		return true;
	else
		return false;	
}

/*
  ==================================================
  class: CSurfaceIterator

  ==================================================
*/

CSurfaceIterator::CSurfaceIterator( CSurface *_head, CSurfaceChecker *_checker )
{
	head = _head;
	current = head;
	checker = _checker;
}

CSurfaceIterator::~CSurfaceIterator()
{
	if ( checker )
		delete checker;
}

CSurface* CSurfaceIterator::getNext( void )
{
	CSurface	*cur;
	
	cur = NULL;

	if ( !current )
		return NULL;

	// is current ok
	if ( !checker )
	{
		cur = current;
	}
	else
	{
		// search with checker test
		for ( ; current; current=current->getNext() )
		{
			if ( checker->isOk( current ) )
			{
				cur = current;
				break;
			}
		}
		if ( !current )
			return NULL;
	}

	current = current->getNext();

	return cur;
}

bool CSurfaceIterator::isLast( void )
{
	if ( current == NULL )
		return true;

	return false;
}

void CSurfaceIterator::reset( void )
{
	current = head;
}
