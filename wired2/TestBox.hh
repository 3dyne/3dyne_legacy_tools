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



// TestBox.hh

#ifndef __TestBox
#define __TestBox

#include "lib_math.h"
#include "EditAble.hh"
#include "CtrlPoint.hh"

/*
  ==================================================
  class: TestBox

  demo EditAble
  ==================================================
*/
class TestBox: public EditAble
{
	
public:
	TestBox( void );
	TestBox( vec3d_t _spawnpos );

	virtual ~TestBox() { }

	// ====================
	// EditAble stuff
	// ====================

        TestBox*	getNext();
        void		setNext( TestBox *_next );

	// checkViewBounds
	void		checkViewBounds( vec3d_t viewmin, vec3d_t viewmax );

	// isVisible
	bool		isVisible( void )
		{
			return select & SELECT_VISIBLE;
		}

	// getBB
	void		getBB( vec3d_t _min, vec3d_t _max )
		{
			Vec3dCopy( _min, min );
			Vec3dCopy( _max, max );
		}


	// intersectWithRay
        bool		intersectWithRay( vec3d_t start, vec3d_t dir );

	// setSelect
	void		setSelect( int _select ) 
		{
			select = _select;
		}

	// getSelect
	int		getSelect( void ) 
		{
			return select;
		}

	// testFlagSelect
	bool		testFlagSelect( int _flag )
		{
			return select & _flag;
		}
	// setFlagSelect
	void		setFlagSelect( int _flag )
		{
			select |= _flag;
		}

	// resetFlagSelect
	void		resetFlagSelect( int _flag )
		{
			select &= ~_flag;
		}

	// copySelf
	EditAble*	copySelf( void )
		{
			// fix me
//			EditAble	*copy;
//			copy = new TestBox();
//			return copy;
			return NULL;
		}

	// moveSelf
	void		moveSelf( vec3d_t from, vec3d_t to );


	// ====================
	// TestBox stuff
	// ====================
	
	// CtrlPoints stuff
	void		insertCtrlPoint( CtrlPoint *obj );
	void		removeCtrlPoint( CtrlPoint *obj );
	CtrlPoint*	getFirstCtrlPoint( void );

private: // methodes
	void		calcBB( void );

private:
	// EditAble stuff
//	EditAble*	next;
	TestBox*	next;
	int		select;
	vec3d_t		min, max;

	// TestBox stuff
	CtrlPoint	*ctrlpoints;
};


/*
  ========================================
  class: TestBoxIterator

  ========================================
*/
class TestBoxChecker
{
public:
	virtual bool isOk( TestBox * ) = 0;
};

class TestBoxChecker_always: public TestBoxChecker
{
public:
	bool isOk( TestBox * ) { return true; }
};

class TestBoxChecker_select: public TestBoxChecker
{
public:
	TestBoxChecker_select( int _flag )
		{
			flag = _flag;
		}
	bool isOk( TestBox *obj )
		{
			if ( obj->getSelect() & flag )
				return true;
			else
				return false;
		}
private:
	int	flag;
};

class TestBoxIterator: public EAIterator
{

public:
	TestBoxIterator( TestBox* _head, TestBoxChecker *_checker = NULL )
		{
			current = head = _head;
			checker = _checker;
		}

	TestBox*	getNext( void )
		{
			TestBox	*cur;
	
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

	bool		isLast( void )
		{
			if ( current == NULL )
				return true;
			
			return false;
		}	
	
	void		reset( void )
		{
			current = head;
		}
	
private:
	TestBox		*head;
	TestBox		*current;
	TestBoxChecker	*checker;

};
#endif
