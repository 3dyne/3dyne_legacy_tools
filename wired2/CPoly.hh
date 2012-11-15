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



// CPoly.hh

#ifndef __CPoly
#define __CPoly

#include <stdio.h>

#include "lib_math.h"
#include "lib_mesh.h"
#include "lib_token.h"
#include "lib_unique.h"
#include "EditAble.hh"
#include "CtrlPoint.hh"
#include "texture.h"

/*
  ==================================================
  class: CPoly

  ==================================================
*/
#if 0
typedef struct {
	char		ident[32];
	
	vec2d_t		shift;
	vec2d_t		scale;
	float		rotate;
} cptexdef_t;
#else
typedef texturedef_t cptexdef_t;

#endif

class CPoly: public EditAble
{
public:
	CPoly( int _edgenum );
	virtual ~CPoly();

	// EditAble interface
	CPoly*			getNext();
	void			setNext( CPoly *_next );
	void checkViewBounds( vec3d_t viewmin, vec3d_t viewmax );
	bool			isVisible( void );
	void			getBB( vec3d_t _min, vec3d_t _max );
	bool			intersectWithRay( vec3d_t start, vec3d_t dir );
	void			setSelect( int _select );
	int			getSelect( void );
	bool			testFlagSelect( int _flag );
	void			setFlagSelect( int _flag );
	void			resetFlagSelect( int _flag );
	
	CPoly*			copySelf( void );
	void			moveSelf( vec3d_t from, vec3d_t to );

	void		setID( unique_t id );
	unique_t	getID( void );

private:
	CPoly*			next;
	int			select;
	vec3d_t			min, max;	

	SelectBrush		*select_brush;

public:
	void			insertCtrlPoint( CtrlPoint *obj );
	void			removeCtrlPoint( CtrlPoint *obj );
	CtrlPoint*		getFirstCtrlPoint( void );

	void			calcBB( void );	

	void			setPlane( vec3d_t _norm, float _dist );
	void			getPlane( vec3d_t _norm, float *_dist );

	void			setPlaneOfCtrlPoints( void );	// private ?

	int			getEdgeNum( void );
	uvmesh_t*		generateCurveOfEdge( int edge, int ustepnum );

	void			getCenter( vec3d_t center );

	void			setTexdef( cptexdef_t *_td );
	void			getTexdef( cptexdef_t *_td );

	hobj_t *		buildClass( void );
	bool			initFromClass( hobj_t *cls );

	// hack
	SelectBrush *		getSelectBrush( void );

private:
	int			edgenum;
	int			pointnum; // = edgenum * 2
	vec3d_t			norm;
	float			dist;
	CtrlPoint		*ctrlpoints;	// list
	cptexdef_t		td;

	unique_t		self_id;
};


/*
  ==================================================
  class: CPolyIterator

  ==================================================
*/
class CPolyChecker
{
public:
	virtual bool isOk( CPoly * ) = 0;
	virtual ~CPolyChecker() {}
};

class CPolyChecker_always: public CPolyChecker
{
public:
	bool isOk( CPoly * );
};

class CPolyChecker_select: public CPolyChecker
{
public:
	CPolyChecker_select( int _flag );
	bool isOk( CPoly *obj );

private:
	int	flag;
};



class CPolyIterator: public EAIterator
{
public:
	CPolyIterator( CPoly *_head, CPolyChecker *_checker = NULL );
	virtual ~CPolyIterator();

	CPoly* getNext( void );
	bool isLast( void );
	void reset( void );

private:
	CPoly		*head;
	CPoly		*current;
	CPolyChecker	*checker;
};

#endif
