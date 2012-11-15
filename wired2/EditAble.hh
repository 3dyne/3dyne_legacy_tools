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



// EditAble.hh

#ifndef __EditAble
#define __EditAble

#include <stdio.h>
#include "lib_math.h"
#include "lib_container.h"

#include "brush.h"

#define         SELECT_NORMAL   ( 0 )                                           
#define         SELECT_RED      ( 1 )                                           
#define         SELECT_GREEN    ( 2 )                                           
                                                                                
#define         SELECT_BLUE     ( 4 )                                           
#define         SELECT_PRE      ( 8 )                                           
                                                                                
#define         SELECT_VISIBLE  ( 256 )                                         
#define         SELECT_UPDATE   ( 512 )                                         


/*
  ==================================================
  class: EditAble

  abstract class
  ==================================================
*/
class EditAble
{
public:
	// list
//	virtual EditAble*	getNext( void ) = 0;
//	virtual void		setNext( EditAble * ) = 0;

	// visibility
	virtual void		checkViewBounds( vec3d_t viewmin, vec3d_t viewmax ) = 0;
	virtual bool		isVisible( void ) = 0; // same as testFlagSelect( SELECT_VISIBLE )
	virtual void		getBB( vec3d_t min, vec3d_t max ) = 0;

	/* virtual unique_t	getUnique() ??? */

	// select
	virtual int		getSelect( void ) = 0;
	virtual void		setSelect( int ) = 0;
	virtual bool		testFlagSelect( int ) = 0;
	virtual void		setFlagSelect( int ) = 0;
	virtual void		resetFlagSelect( int ) = 0;
	virtual bool		intersectWithRay( vec3d_t start, vec3d_t dir ) = 0;

	// edit
	virtual EditAble*	copySelf( void ) = 0;
	virtual void		moveSelf( vec3d_t from, vec3d_t to ) = 0;

};

class SelectBrush
{
public:
	SelectBrush();
	~SelectBrush();

	// init
	void reset();
	void addPlane( vec3d_t norm, fp_t dist );
	void setupFromBB( vec3d_t min, vec3d_t max );

	// test
	bool			intersectWithRay( vec3d_t origin, vec3d_t dir );
	fp_t			rayLength( void );

private:
	u_list_t		plane_list;
	fp_t			ray_len;
};

class EAIterator
{
public:
#if 0
	virtual EditAble*	getCurrent( void ) = 0;
	virtual bool		next( void ) = 0;
	virtual void		rewind( void ) = 0;
#else
	virtual EditAble*	getNext( void ) = 0;
	virtual bool		isLast( void ) = 0;
	virtual void		reset( void ) = 0;
	virtual ~EAIterator() {}
#endif
};


/*
  ==================================================
  EditAbleList functions

  ==================================================
*/

// EAL list functions itself
void EAL_Init( EditAble **list );
void EAL_Insert( EditAble **list, EditAble *obj );
void EAL_Remove( EditAble **list, EditAble *obj );

// visibility / update
void EAL_CheckViewBounds( EAIterator *iter, vec3d_t viewmin, vec3d_t viewmax );
void EAL_AllUpdateFlagsTrue( EAIterator *iter );
void EAL_CalcTotalBB( EAIterator *iter, vec3d_t totalmin, vec3d_t totalmax );

// selections
void EAL_BlueRaySelector( EAIterator *iter, vec3d_t start, vec3d_t dir );
void EAL_BlueToRed( EAIterator *iter );
void EAL_BlueToGreen( EAIterator *iter );
void EAL_BlueToNormal( EAIterator *iter );
void EAL_DeselectBlue( EAIterator *iter );

// edit
void EAL_BlueMove( EAIterator *iter, vec3d_t from, vec3d_t to );
void EAL_BlueRedMove( EAIterator *iter, vec3d_t from, vec3d_t to );
void EAL_BlueCopy( EAIterator *iter );
void EAL_BlueDelete( EAIterator *iter );
void EAL_BlueRedDelete( EAIterator *iter );

void EAL_Move( EAIterator *iter, vec3d_t from, vec3d_t to );

#endif 
