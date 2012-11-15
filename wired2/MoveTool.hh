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



// MoveTool.hh

#ifndef __MoveTool_included
#define __MoveTool_included

#include <qobject.h>

#include "vec.h"
#include "brush.h"

#include "EditAble.hh"

#include "VecMath.hh"

class MoveTool;

extern MoveTool		*movetool_i;

/*                                                                              
  ===============================================                               
  class: MoveTool                                                              
  ===============================================                               
*/                                                                              

class MoveTool : public QObject
{
	Q_OBJECT

public:
	MoveTool( QObject* parent = NULL, char* name = NULL );
	~MoveTool();

private:
	void	moveBrushes( unsigned int select);
	void	moveFaces( bool cleanup, unsigned int select );

public slots:
	void xzPressSlot( Vec2 v, unsigned int select );
	void xzDragSlot( Vec2 v, unsigned int select );
	void xzReleaseSlot( Vec2 v, unsigned int select );

	void yPressSlot( Vec2 v, unsigned int select );
	void yDragSlot( Vec2 v, unsigned int select );
	void yReleaseSlot( Vec2 v, unsigned int select );

	void Face_xzPressSlot( Vec2 v, unsigned int select );
	void Face_xzDragSlot( Vec2 v, unsigned int select );
	void Face_xzReleaseSlot( Vec2 v, unsigned int select );

	void Face_yPressSlot( Vec2 v, unsigned int select );
	void Face_yDragSlot( Vec2 v, unsigned int select );
	void Face_yReleaseSlot( Vec2 v, unsigned int select );

	//
	// new EditAble stuff
	//
	void startMoveCycle( EAIterator *iter, Vec3 v );
	void dragMoveCycle( Vec3 v );
	void finishMoveCycle( Vec3 v );

private:
	vec3d_t		from;
	vec3d_t		move;

	EAIterator	*cur_iter;
};

#endif // __MoveTool_included
