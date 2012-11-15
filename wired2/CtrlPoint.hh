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



// CtrlPoint.hh

#ifndef __CtrlPoint
#define __CtrlPoint

#include <stdio.h>
#include "lib_token.h"
#include "lib_math.h"
#include "lib_hobj.h"
#include "EditAble.hh"

/*
  ==================================================
  class: CtrlPoint

  a Vertex/ControlPoint class for meshes
  ==================================================
*/
class CtrlPoint: public EditAble
{

public:
	CtrlPoint( void );
	CtrlPoint( vec3d_t _spawnpos );

	virtual ~CtrlPoint();

	// ====================
	// EditAble stuff
	// ====================

        CtrlPoint*	getNext();
        void		setNext( CtrlPoint *_next );
	void		checkViewBounds( vec3d_t viewmin, vec3d_t viewmax );
	bool		isVisible( void );
	void		getBB( vec3d_t _min, vec3d_t _max );
        bool		intersectWithRay( vec3d_t start, vec3d_t dir );
	void		setSelect( int _select );
	int		getSelect( void ); 
	bool		testFlagSelect( int _flag );
	void		setFlagSelect( int _flag );
	void		resetFlagSelect( int _flag );

	// copySelf
	CtrlPoint*	copySelf( void );

	// moveSelf
	void		moveSelf( vec3d_t from, vec3d_t to );

	// ====================
	// CtrlPoint stuff
	// ====================

	void		getPos( vec3d_t _pos, int *_u, int *_v );
	void		setPos( vec3d_t _pos, int _u, int _v );

	hpair_t*	buildHPair( void );
	void		init( void );

	void		setProjectionPlane( vec3d_t _norm, float _dist );
	void		getProjectionPlane( vec3d_t _norm, float *_dist );
	void		useProjectionPlane( bool _use );
	
private: // methodes
	void calcBB( void );

private:
	// EditAble stuff
	CtrlPoint*	next;
	int		select;

	// CtrlPoint stuff
	vec3d_t		min, max;	// a point gets a small bb for easier selection
	vec3d_t		pos;
	int		u, v;

	// special for CPoly
	bool		use_projection;
	vec3d_t		norm;
	float		dist;
};



/*
  ========================================
  class: CtrlPointIterator

  ========================================
*/
class CtrlPointChecker
{
public:
	virtual bool isOk( CtrlPoint * ) = 0;
	virtual ~CtrlPointChecker() {}
};

class CtrlPointChecker_always: public CtrlPointChecker
{
public:
	bool isOk( CtrlPoint * );
};

class CtrlPointChecker_select: public CtrlPointChecker
{
public:
	CtrlPointChecker_select( int _flag );
	bool isOk( CtrlPoint *obj );
private:
	int		flag;
};

class CtrlPointIterator: public EAIterator
{
public:
	CtrlPointIterator( CtrlPoint* _head, CtrlPointChecker* _checker = NULL );
	virtual ~CtrlPointIterator();
	CtrlPoint*	getNext( void );
	void		reset( void );
	bool		isLast( void );

private:
	CtrlPoint	*head;
	CtrlPoint	*current;
	CtrlPointChecker	*checker;
};
#endif
