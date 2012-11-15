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



// CSurface.hh

#ifndef __CSurface
#define __CSurface

#include <stdio.h>

#include "lib_math.h"
#include "lib_mesh.h"
#include "lib_hobj.h"
#include "lib_token.h"
#include "lib_unique.h"
#include "EditAble.hh"
#include "CtrlPoint.hh"

/*
  ==================================================
  class: CSurface

  ==================================================
*/
typedef struct {
	char			ident[32];
//	vec2d_t			vecs[2];
//	vec2d_t			len;
//	vec2d_t			ofs;	
	vec2d_t			shift;	// p0
	vec2d_t			vecs[3];  // p1-p0, p2-p0, p3-p0
	vec2d_t			scale;
} cstexdef_t;

class CSurface: public EditAble
{
public:
	CSurface( int usize, int vsize );
	virtual ~CSurface();

	// EditAble interface
	CSurface*		getNext( void );
	void			setNext( CSurface *_next );
	void			checkViewBounds( vec3d_t viewmin, vec3d_t viewmax );
	bool			isVisible( void );
	void			getBB( vec3d_t _min, vec3d_t _max );
	bool			intersectWithRay( vec3d_t start, vec3d_t dir );
	void			setSelect( int _select );
	int			getSelect( void );
	bool			testFlagSelect( int _flag );
	void			setFlagSelect( int _flag );
	void			resetFlagSelect( int _flag );
	
	CSurface*		copySelf( void );
	void			moveSelf( vec3d_t from, vec3d_t to );

	void			setID( unique_t id );
	unique_t		getID( void );

private:
	CSurface*		next;
	int			select;
	vec3d_t			min, max;

public:
	// 
	void			insertCtrlPoint( CtrlPoint *obj );
	void			removeCtrlPoint( CtrlPoint *obj );
	CtrlPoint*		getFirstCtrlPoint( void );

	void			calcBB( void );

	uvmesh_t*		generateUVMesh( int ustepnum, int vstepnum );

	void			setTexelCtrlPoint( int u, int v, vec2d_t _pos );
	void			getTexelCtrlPoint( int u, int v, vec2d_t _pos );
	void			setTexdef( cstexdef_t *_td );
	void			getTexdef( cstexdef_t *_td );

	hobj_t*			buildClass( void );
	bool			initFromClass( hobj_t *cls );
private:      
	// ctrlpoints
	int			upointnum;
	int			vpointnum;
	CtrlPoint		*ctrlpoints;

	// texdef
	vec2d_t			texelpoints[2][2];
	cstexdef_t		td;

	unique_t		self_id;
};



/*
  ==================================================
  class: CSurfaceIterator

  ==================================================
*/
class CSurfaceChecker
{
public:
	virtual bool isOk( CSurface * ) = 0;
	virtual ~CSurfaceChecker() {}
};

class CSurfaceChecker_always: public CSurfaceChecker
{
public:
	bool isOk( CSurface * );
};

class CSurfaceChecker_select: public CSurfaceChecker
{
public:
	CSurfaceChecker_select( int _flag );
	bool isOk( CSurface *obj );
private:
	int	flag;
};


class CSurfaceIterator: public EAIterator
{
public:
	CSurfaceIterator( CSurface* _head, CSurfaceChecker* _checker = NULL );
	virtual ~CSurfaceIterator();

	CSurface* getNext( void );
	bool isLast( void );
	void reset( void );

private:
	CSurface	*head;
	CSurface	*current;
	CSurfaceChecker	*checker;
};
  

#endif
