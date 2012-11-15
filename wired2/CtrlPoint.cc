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



// CtrlPoint.cc

#include "CtrlPoint.hh"

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

CtrlPoint::CtrlPoint( void )
{
	Vec3dInit( pos, 0, 0, 0 );
	next = NULL; 
	select = 0;	
	u = v = 0;
	use_projection = false;
	calcBB();	
}

CtrlPoint::CtrlPoint( vec3d_t _spawnpos )
{
//	printf( "CtrlPoint:: spawn at " );
//	Vec3dPrint( _spawnpos );
	Vec3dCopy( this->pos, _spawnpos );
	next = NULL; 
	select = 0;	
	u = v = 0;
	use_projection = false;
	calcBB();
}

CtrlPoint::~CtrlPoint()
{
	printf( "CtrlPoint::~CtrlPoint\n" );
}

CtrlPoint* CtrlPoint::getNext( void ) 
{
	return next;
}

void CtrlPoint::setNext( CtrlPoint *_next )
{
	next = _next;
}


bool CtrlPoint::isVisible( void )
{
	return select & SELECT_VISIBLE;	
}

void CtrlPoint::getBB( vec3d_t _min, vec3d_t _max )
{
	Vec3dCopy( _min, min );
	Vec3dCopy( _max, max );
}

void CtrlPoint::setSelect( int _select )
{
	select = _select;
}

int CtrlPoint::getSelect( void )
{
	return select;
}

bool CtrlPoint::testFlagSelect( int _flag )
{
	return select & _flag;
}

void CtrlPoint::setFlagSelect( int _flag )
{
	select |= _flag;
}

void CtrlPoint:: resetFlagSelect( int _flag )
{
	select &= ~_flag;	
}

void CtrlPoint::getPos( vec3d_t _pos, int *_u, int *_v )
{
	Vec3dCopy( _pos, pos );
	*_u = u;
	*_v = v;
}

void CtrlPoint::setPos( vec3d_t _pos, int _u, int _v )
{
	Vec3dCopy( pos, _pos );
	u = _u;
	v = _v;
	calcBB();
	select |= SELECT_UPDATE;
}

void CtrlPoint::calcBB( void )
{
	vec3d_t		space = { 4.0, 4.0, 4.0 };
//	printf( "CtrlPoint::calcBB\n" );
	Vec3dPrint( this->pos );
	Vec3dAdd( this->max, space, this->pos );
	Vec3dSub( this->min, this->pos, space );
// 	Vec3dPrint( this->max );
//	Vec3dPrint( this->min );
}



/*
  ====================
  intersectWithRay

  ====================
*/
bool CtrlPoint::intersectWithRay( vec3d_t start, vec3d_t dir )
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



/*
  ====================
  checkViewBounds

  ====================
*/
void CtrlPoint::checkViewBounds( vec3d_t viewmin, vec3d_t viewmax )
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



/*
  ====================
  copySelf

  ====================
*/
CtrlPoint* CtrlPoint::copySelf( void )
{
	CtrlPoint	*cpnew;

	cpnew = new CtrlPoint();

	cpnew->u = this->u;
	cpnew->v = this->v;
	Vec3dCopy( cpnew->pos, this->pos );

	cpnew->calcBB();
	cpnew->select = SELECT_NORMAL | SELECT_UPDATE | SELECT_VISIBLE;

	return cpnew;
}

/*
  ====================
  moveSelf

  ====================
*/
#define PROJECT_X	( 0 )
#define PROJECT_Y	( 1 )
#define PROJECT_Z	( 2 )

static int TypeOfProjection( vec3d_t norm )
{
	int		i;
	vec3d_t		an;

	if ( norm[0] == 1.0 || norm[0] == -1.0 )
		return PROJECT_X;
	if ( norm[1] == 1.0 || norm[1] == -1.0 )
		return PROJECT_Y;
	if ( norm[2] == 1.0 || norm[2] == -1.0 )
		return PROJECT_Z;

	for ( i = 0; i < 3; i++ )
		an[i] = fabs( norm[i] );

	if ( an[0] >= an[1] && an[0] >= an[2] )
		return PROJECT_X;
	else if ( an[1] >= an[0] && an[1] >= an[2] )
		return PROJECT_Y;
	else if ( an[2] >= an[0] && an[2] >= an[1] )
		return PROJECT_Z;
	
	return -1;
}

void CtrlPoint::moveSelf( vec3d_t from, vec3d_t to )
{

	if ( !use_projection )
	{
		vec3d_t		delta;
		
//	printf( "CtrlPoint::moveSelf\n" );
		
		Vec3dSub( delta, to, from );
		Vec3dAdd( pos, pos, delta );
		
		calcBB();
		
		select |= SELECT_UPDATE;
	}
	else
	{
		vec3d_t		delta;
		vec3d_t		tmp;
		int		type;
		vec3d_t		p1, p2;
		float		d1, d2, d;
		
		Vec3dSub( delta, to, from );
		Vec3dAdd( tmp, pos, delta );		

		type = TypeOfProjection( norm );

		if ( type == PROJECT_X )
		{
			Vec3dInit( p1, -8000.0, tmp[1], tmp[2] );
			Vec3dInit( p2, 8000.0, tmp[1], tmp[2] );
		}
		else if ( type == PROJECT_Y )
		{
			Vec3dInit( p1, tmp[0], -8000.0, tmp[2] );
			Vec3dInit( p2, tmp[0], 8000.0, tmp[2] );
		}
		else if ( type == PROJECT_Z )
		{
			Vec3dInit( p1, tmp[0], tmp[1], -8000.0 );
			Vec3dInit( p2, tmp[0], tmp[1], 8000.0 );
		}
		
	        d1 = Vec3dDotProduct( p1, norm ) - dist;
		d2 = Vec3dDotProduct( p2, norm ) - dist;

		if ( d1 * d2 >= 0.0 )
		{
			printf( "WARNING: CtrlPoint::moveSelf\n" );
			printf( " internal projection error, points on same side of plane.\n" );
			return;
		}

		d = d1 / ( d1 - d2 );
		for ( int i = 0; i < 3; i++ )
			tmp[i] = p1[i] + d * ( p2[i]-p1[i] );

		Vec3dCopy( pos, tmp );
		calcBB();
		
		select |= SELECT_UPDATE;
	}
}



/*
  ====================
  setProjectionPlane

  ====================
*/
void CtrlPoint::setProjectionPlane( vec3d_t _norm, float _dist )
{
	Vec3dCopy( norm, _norm );
	dist = _dist;
}



/*
  ====================
  getProjectionPlane
  
  ====================
*/
void CtrlPoint::getProjectionPlane( vec3d_t _norm, float *_dist )
{
	Vec3dCopy( _norm, norm );
	*_dist = dist;
}


/*
  ====================
  useProjectionPlane

  ====================
*/
void CtrlPoint::useProjectionPlane( bool _use )
{
	use_projection = _use;
}



/*
  ==============================
  buildHPair

  ==============================
*/
hpair_t * CtrlPoint::buildHPair( void )
{
	hpair_t		*pair;
	char		key[256];
	char		value[256];

	sprintf( key, "u%d_v%d", u, v );
	sprintf( value, "%f %f %f", pos[0], pos[1], pos[2] );
	pair = NewHPair2( "vec3d", key, value );
	
	return pair;
}

/*
  ==============================
  init

  ==============================
*/
void CtrlPoint::init( void )
{
	calcBB();                                                               
        select = SELECT_UPDATE | SELECT_UPDATE | SELECT_VISIBLE;
}


/*
  ==================================================
  class: CtrlPointChecker

  ==================================================
*/
bool CtrlPointChecker_always::isOk( CtrlPoint * )
{
	return true;
}

CtrlPointChecker_select::CtrlPointChecker_select( int _flag )
{
	flag = _flag;
}

bool CtrlPointChecker_select::isOk( CtrlPoint *obj )
{
	if ( obj->getSelect() & flag )
		return true;
	else
		return false;	
}



/*
  ==================================================
  class: CtrlPointIterator

  ==================================================
*/
CtrlPointIterator::CtrlPointIterator( CtrlPoint* _head, CtrlPointChecker* _checker )
{
	current = head = _head;
	checker = _checker;
}


CtrlPointIterator::~CtrlPointIterator()
{
	if ( checker )
		delete checker;	
}



CtrlPoint* CtrlPointIterator::getNext( void )
{
	CtrlPoint	*cur;
	
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


void CtrlPointIterator::reset( void )
{
	current = head;
}

bool CtrlPointIterator::isLast( void )
{
	if ( current == NULL )
		return true;
	
	return false;	
}
