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



// CPoly.cc

#include "CPoly.hh"
#include "lib_bezier.h"




/*
  ==================================================
  class: CPoly

  ==================================================
*/

CPoly::CPoly( int _edgenum )
{
	edgenum = _edgenum;
	pointnum = _edgenum * 2;
	ctrlpoints = NULL;
	select = 0;
	
	self_id = UNIQUE_INVALIDE;

	select_brush = new SelectBrush;
}

CPoly::~CPoly()
{
	printf( "CPoly::~CPoly\n" );

	// free ctrlpoint list
	CtrlPoint	*cp, *next;
	for ( cp = ctrlpoints; cp ; cp=next )
	{
		next = cp->getNext();
		delete cp;
	}
}

void CPoly::setID( unique_t id )
{
	self_id = id;
}

unique_t CPoly::getID( void )
{
	return self_id;
}

CPoly* CPoly::getNext( void )
{
	return next;
}

void CPoly::setNext( CPoly *_next )
{
	next = _next;
}

void CPoly::checkViewBounds( vec3d_t viewmin, vec3d_t viewmax )
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

bool CPoly::isVisible( void )
{
	return select & SELECT_VISIBLE;
}

void CPoly::getBB( vec3d_t _min, vec3d_t _max )
{
	Vec3dCopy( _min, min );
	Vec3dCopy( _max, max );
}

bool CPoly::intersectWithRay( vec3d_t start, vec3d_t dir )
{
	return select_brush->intersectWithRay( start, dir );
	
#if 0

#if 1
	DistStraightToPoints( start, dir, min, max, &d1, &d2 );
#else
	d1 = DistStraightToPoint( start, dir, min );
	d2 = DistStraightToPoint( start, dir, max );
#endif
	Vec3dSub( delta, max, min );

	printf( "d1 %f d2 %f delta %f\n", d1, d2, Vec3dLen( delta ) );
	
	if ( /*Vec3dLen( delta ) >= fabs(d1)+fabs(d2)*/ d1 * d2 < 0.0 )
	{
//		printf( "intersect\n" );
		return true;
	}

//	printf( "%f %f\n", d1, d2 );
	return false;	

#endif
}


void CPoly::setSelect( int _select )
{
	select = _select;
}

int CPoly::getSelect( void )
{
	return select;
}

bool CPoly::testFlagSelect( int _flag )
{
	return select & _flag;
}

void CPoly::setFlagSelect( int _flag )
{
	select |= _flag;
}

void CPoly::resetFlagSelect( int _flag )
{
	select &= ~_flag;
}

CPoly* CPoly::copySelf( void )
{
	CPoly		*cpoly;

	cpoly = new CPoly( 0 );

	// copy static
	cpoly->edgenum = this->edgenum;
	cpoly->pointnum = this->pointnum;
	Vec3dCopy( cpoly->norm, this->norm );
	cpoly->dist = this->dist;
	memcpy( &cpoly->td, &this->td, sizeof( cptexdef_t ) );

	// copy ctrlpoints
	cpoly->ctrlpoints = NULL;
	CtrlPointIterator	cpi( this->ctrlpoints );
	CtrlPoint		*cp, *cpnew;

	for ( cpi.reset(); ( cp = cpi.getNext() ) ; )
	{
		cpnew = cp->copySelf();
		cpnew->setNext( cpoly->ctrlpoints );
		cpoly->ctrlpoints = cpnew;
		
	}

	cpoly->calcBB();
	cpoly->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE;
	return cpoly;
}

void CPoly::moveSelf( vec3d_t from, vec3d_t to )
{
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;
	int			i;
	vec3d_t			pts[3];
	int			u, v;

	// fixme: the plane move is crap
	
	for ( iter.reset(), i = 0; ( cp = iter.getNext() ); i++ )
	{
		cp->useProjectionPlane( false );
		cp->moveSelf( from, to );
		cp->useProjectionPlane( true );

		vec3d_t		tmp;
		cp->getPos( tmp, &u, &v );
		if ( u == 0 )
		{
			Vec3dCopy( pts[0], tmp );
		}
		else if ( u == 2 )
		{
			Vec3dCopy( pts[1], tmp );
		}
		else if ( u == 4 )
		{
			Vec3dCopy( pts[2], tmp );
		}
	}

	calcBB();

	select |= SELECT_UPDATE;

	//
	// recalc plane
	//
//	Vec3dInitPlane( norm, &dist, pts[0], pts[1], pts[2] );
	dist = Vec3dInitPlane2( norm, pts[0] );
	
	setPlaneOfCtrlPoints();
}

void CPoly::calcBB( void )
{
	CtrlPointIterator	iter( ctrlpoints );

	EAL_CalcTotalBB( &iter, min, max );

	select_brush->reset();
	select_brush->setupFromBB( min, max );
}

/*
  ====================
  insertCtrlPoint

  ====================
*/
void CPoly::insertCtrlPoint( CtrlPoint *obj )
{
	printf( "CPoly::insertCtrlPoint\n" );

	obj->setProjectionPlane( norm, dist );
	obj->useProjectionPlane( true );

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
void CPoly::removeCtrlPoint( CtrlPoint *obj )
{
	CtrlPoint		*head, *c, *next;

	printf( "CPoly::removeCtrlPoint\n" );
	
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
  setPlaneOfCtrlPoints

  ====================
*/
void CPoly::setPlaneOfCtrlPoints( void )
{
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

	for ( iter.reset(); ( cp = iter.getNext() ); )
	{
		cp->setProjectionPlane( norm, dist );
		cp->useProjectionPlane( true );
	}
}



/*
  ====================
  getFirstCtrlPoint

  ====================
*/
CtrlPoint* CPoly::getFirstCtrlPoint( void )
{
	return ctrlpoints;
}



/*
  ====================
  setPlane

  ====================
*/
void CPoly::setPlane( vec3d_t _norm, float _dist )
{
	Vec3dCopy( norm, _norm );
	dist = _dist;
	setPlaneOfCtrlPoints();
}

/*
  ====================
  getPlane

  ====================
*/
void CPoly::getPlane( vec3d_t _norm, float *_dist )
{
	Vec3dCopy( _norm, norm );
	*_dist = dist;
}



/*
  ====================
  getEdgeNum

  ====================
*/
int CPoly::getEdgeNum( void )
{
	return edgenum;
}



/*
  ====================
  generateCurveOfEdge

  ====================
*/
uvmesh_t* CPoly::generateCurveOfEdge( int edge, int ustepnum )
{
	curve_ctrl_t *sc = NewBezierCurve( 3 );
	
	int c1 = edge * 2;
	int c2 = edge * 2 +1;
	int c3 = edge * 2 +2;
	if ( c3 == pointnum )
		c3 = 0;

	
	CtrlPointIterator	cpi( ctrlpoints );
	CtrlPoint		*cp;
	int i;
	for ( cpi.reset(), i = 0; ( cp = cpi.getNext() ); )
	{
		vec3d_t		tmp;
		int		u, v;

		cp->getPos( tmp, &u, &v );
		if ( u == c1 )
		{
			SetCurveCtrlPoint( sc, 0, tmp );
			i++;
		}
		else if ( u == c2 )
		{
			SetCurveCtrlPoint( sc, 1, tmp );
			i++;
		}
		else if ( u == c3 )
		{
			SetCurveCtrlPoint( sc, 2, tmp );
			i++;
		}
	}

	if ( i != 3 )
	{
		printf( "WARNING: CPoly::generateCurveOfEdge: *** internal inconsistency ***\n" );
		printf( " can't find enough CtrlPoints for edge.\n" );
		FreeBezierCurve( sc );
		return NULL;
	}

	curve_points_t *sp = EvalCurvePoints( sc, ustepnum );
	uvmesh_t *mesh = NewUVMesh( ustepnum, 1 );
	for ( int u = 0; u < ustepnum; u++ )
	{
		vec3d_t	p;
		GetCurvePoint( sp, u, p );
		SetUVMeshPoint( mesh, u, 0, p );
	}
	
	FreeCurvePoints( sp );
	FreeBezierCurve( sc );

	return mesh;
}



/*
  ====================
  getCenter

  ====================
*/
void CPoly::getCenter( vec3d_t center )
{
	int		i;

	Vec3dInit( center, 0, 0, 0 );

	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

	for ( iter.reset(), i = 0; ( cp = iter.getNext() ); i++ )
	{
		vec3d_t		tmp;
		int		u, v;

		cp->getPos( tmp, &u, &v );
		Vec3dAdd( center, center, tmp );
	}

	float scale = 1.0 / i;
	Vec3dScale( center, scale, center );
}



/*
  ====================
  setTexdef

  ====================
*/
void CPoly::setTexdef( cptexdef_t *_td )
{
	memcpy( &td, _td, sizeof( cptexdef_t ) );
}



/*
  ====================
  getTexdef

  ====================
*/
void CPoly::getTexdef( cptexdef_t *_td )
{
	memcpy( _td, &td, sizeof( cptexdef_t ) );
}





/*
  ==============================
  buildClass
  
  ==============================
*/
hobj_t * CPoly::buildClass( void )
{
	hobj_t		*cpoly;
	hpair_t		*pair;
	char		tt[256];

	sprintf( tt, "#%u", self_id );
	cpoly = NewClass( "cpoly", tt );

	// pointnum
	sprintf( tt, "%d", pointnum );
	pair = NewHPair2( "int", "pointnum", tt );
	InsertHPair( cpoly, pair );

	// edgenum
	sprintf( tt, "%d", edgenum );
	pair = NewHPair2( "int", "edgenum", tt );
	InsertHPair( cpoly, pair );

	// norm
	sprintf( tt, "%f %f %f", norm[0], norm[1], norm[2] );
	pair = NewHPair2( "vec3d", "norm", tt );
	InsertHPair( cpoly, pair );

	// dist
	sprintf( tt, "%f", dist );
	pair = NewHPair2( "float", "dist", tt );
	InsertHPair( cpoly, pair );

	// CtrlPoints
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

	for ( iter.reset(); (cp = iter.getNext()); )
	{
		pair = cp->buildHPair();
		InsertHPair( cpoly, pair );
	}	

	// ident
	sprintf( tt, "%s", td.ident );
	pair = NewHPair2( "string", "ident", tt );
	InsertHPair( cpoly, pair );

	// shift
	sprintf( tt, "%f %f", td.shift[0], td.shift[1] );
	pair = NewHPair2( "vec2d", "shift", tt );
	InsertHPair( cpoly, pair );

	// scale
	sprintf( tt, "%f %f", td.scale[0], td.scale[1] );
	pair = NewHPair2( "vec2d", "scale", tt );
	InsertHPair( cpoly, pair );

	// rotate
	sprintf( tt, "%f", td.rotate );
	pair = NewHPair2( "float", "rotate", tt );
	InsertHPair( cpoly, pair );

	// hack: write bb, to support cpoly lighting	
	sprintf( tt, "%f %f %f", min[0]+4.0, min[1]+4.0, min[2]+4.0 );
	pair = NewHPair2( "vec3d", "min", tt );
	InsertHPair( cpoly, pair );

	sprintf( tt, "%f %f %f", max[0]-4.0, max[1]-4.0, max[2]-4.0 );
	pair = NewHPair2( "vec3d", "max", tt );
	InsertHPair( cpoly, pair );
      
	return cpoly;
}

/*
  ==============================
  initFromClass

  ==============================
*/
bool CPoly::initFromClass( hobj_t *cls )
{
	int		i;
	char		tt[256];

	self_id = StringToUnique( cls->name );

	EasyFindInt( &pointnum, cls, "pointnum" );
	EasyFindInt( &edgenum, cls, "edgenum" );
	
	ctrlpoints = NULL;
	for ( i = 0; i < pointnum; i++ )
	{
		CtrlPoint	*cp;
		vec3d_t		pos;

		cp = new CtrlPoint();	

		sprintf( tt, "u%d_v%d", i, 0 );
		EasyFindVec3d( pos, cls, tt );

		cp->setPos( pos, i, 0 );

		cp->setNext( ctrlpoints );
		ctrlpoints = cp;
		cp->init();
		cp->useProjectionPlane( true );
	}

	EasyFindString( td.ident, cls, "ident" );
	EasyFindFloat( &td.rotate, cls, "rotate" );
	EasyFindVec2d( td.scale, cls, "scale" );
	EasyFindVec2d( td.shift, cls, "shift" );

	EasyFindFloat( &dist, cls, "dist" );
	EasyFindVec3d( norm, cls, "norm" );
	
	setPlaneOfCtrlPoints();
	calcBB();

	return true;	
}


SelectBrush * CPoly::getSelectBrush( void )
{
	return this->select_brush;
}

/*
  ==================================================
  class: CPolyChecker

  ==================================================
*/
bool CPolyChecker_always::isOk( CPoly * )
{
	return true;
}

CPolyChecker_select::CPolyChecker_select( int _flag )
{
	flag = _flag;
}

bool CPolyChecker_select::isOk( CPoly *obj )
{
	if ( !obj )
	{
		printf( "CPolyChecker_select::isOk CPoly == NULL\n" );
		getchar();
		return false;
	}

	if ( obj->getSelect() & flag )
		return true;
	else
		return false;	
}

/*
  ==================================================
  class: CPolyIterator

  ==================================================
*/

CPolyIterator::CPolyIterator( CPoly *_head, CPolyChecker *_checker )
{
	head = _head;
	current = head;
	checker = _checker;
}

CPolyIterator::~CPolyIterator()
{
	if ( checker )
		delete checker;
}

CPoly* CPolyIterator::getNext( void )
{
	CPoly		*cur;
	
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

bool CPolyIterator::isLast( void )
{
	if ( current == NULL )
		return true;

	return false;
}

void CPolyIterator::reset( void )
{
	current = head;
}
