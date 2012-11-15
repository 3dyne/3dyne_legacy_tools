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



// TestBox.cc

#include "TestBox.hh"
#include <stdlib.h>

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

TestBox::TestBox( void )
{
	next = NULL;
	select = 0;
}

TestBox::TestBox( vec3d_t _spawnpos )
{
	printf( "TestBox:: spawn at " );
	Vec3dPrint( _spawnpos );
	next = NULL; 
	select = 0;	

	vec3d_t		space = { 32, 32 ,32 };
	Vec3dAdd( this->max, space, _spawnpos );
	Vec3dSub( this->min, _spawnpos, space );

	//
	// init CtrlPoints
	//
	ctrlpoints = NULL;

	//
	// test: generate some random CtrlPoints
	//
	int		i;
	vec3d_t		v;
	CtrlPoint	*cp;
	for ( i = 0; i < 10; i++ )
	{
		Vec3dInit( v, 32.0-(random()%64), 32.0-(random()%64), 32.0-(random()%64) );
		Vec3dAdd( v, _spawnpos, v );
		cp = new CtrlPoint( v );
		this->insertCtrlPoint( cp );
	}
}



/*
  ====================
  getNext

  ====================
*/
TestBox* TestBox::getNext() 
{ 
			return next;
}



/*
  ====================
  setNext

  ====================
*/
void TestBox::setNext( TestBox *_next ) 
{ 
	next = _next;
}



/*
  ====================
  intersectWithRay

  ====================
*/
bool TestBox::intersectWithRay( vec3d_t start, vec3d_t dir )
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
void TestBox::checkViewBounds( vec3d_t viewmin, vec3d_t viewmax )
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
  calcBB
  private
  ====================
*/
void TestBox::calcBB( void )
{
	CtrlPointIterator	iter( ctrlpoints );

	EAL_CalcTotalBB( &iter, min, max );
}




/*
  ====================
  moveSelf

  ====================
*/
void TestBox::moveSelf( vec3d_t from, vec3d_t to )
{
	CtrlPointIterator	iter( ctrlpoints );
	CtrlPoint		*cp;

//	printf( "TestBox::moveSelf\n" );
	for ( iter.reset(); ( cp = iter.getNext() ); )
	{
		cp->moveSelf( from, to );
	}

	calcBB();

	select |= SELECT_UPDATE;
}

/*
  ========================================
  CtrlPoints of a TestBox

  ========================================
*/

/*
  ====================
  insertCtrlPoint

  ====================
*/
void TestBox::insertCtrlPoint( CtrlPoint *obj )
{
	printf( "TestBox::insertCtrlPoint\n" );
	
//	EAL_Insert( &ctrlpoints, obj );
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
void TestBox::removeCtrlPoint( CtrlPoint *obj )
{
	CtrlPoint	*head, *c, *next;
	printf( "TestBox::removeCtrlPoint\n" );
//	EAL_Remove( &ctrlpoints, obj );
	
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
CtrlPoint* TestBox::getFirstCtrlPoint( void )
{
	return ctrlpoints;
}
