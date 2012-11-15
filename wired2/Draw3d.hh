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



// Draw3d.hh

#ifndef __Draw3d
#define __Draw3d

#include <stdio.h>
#include <math.h>

#include <qpixmap.h>
#include <qpainter.h>

#include "Wired.hh"
#include "XZView.hh"
#include "YView.hh"
#include "CameraView.hh"

#include "lib_math.h"
#include "brush.h"


/*
  ==================================================
  class: Draw3d

  controls XZView, YView, CameraView
  
  ==================================================
*/

#define VIEW_NONE	( 0 )
#define VIEW_XZ		( 1 )
#define VIEW_Y		( 2 )
#define VIEW_CAMERA	( 4 )

class Draw3d
{
public:
	Draw3d();
	~Draw3d();

	void startDraw( int views );
	void endDraw( void );

	void setColor( QColor *color );
	void setPenStyle( QPen *pen );

	void drawLine( vec3d_t from, vec3d_t to, int shift = 0 );
	void drawHLine( vec3d_t v );
	void drawVLine( vec3d_t v );
	void drawFace( face_t *f, int shift = 0 );

	void drawCross( vec3d_t v );
	void drawBox( vec3d_t v );
	void drawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );
	
	void drawBB( vec3d_t min, vec3d_t max );

private:
	int	active_views;
};


#endif
