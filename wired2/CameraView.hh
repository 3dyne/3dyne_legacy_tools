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



// CameraView.hh

#ifndef __CameraView_included
#define __CameraView_included

#include <math.h>
#include <stdlib.h>

#include <qwidget.h>
#include <qpainter.h>
#include <qimage.h>

#include "vec.h"
#include "render.h"

#include "VecMath.hh"

class CameraView;
extern CameraView		*cameraview_i;


void CameraStartDraw( void );
void CameraEndDraw( void );
void CameraColor( QColor *color );
void CameraDrawLine( vec3d_t from, vec3d_t to );
void CameraDrawPolygon( vec3d_t p[], int pointnum );


/*                                                                              
  ===============================================                               
  class: CameraView                                                           
  ===============================================                               
*/                                                                              
 
class CameraView : public QWidget
{
	Q_OBJECT

	friend void CameraStartDraw( void );
	friend void CameraEndDraw( void );
	friend void CameraColor( QColor *color );
	friend void CameraDrawLine( vec3d_t from, vec3d_t to );
	friend void CameraDrawPolygon( vec3d_t p[], int pointnum );
public:
	CameraView( QWidget *parent = 0, const char *name = 0);
	virtual ~CameraView();

	void world2Painter( vec3d_t, vec3d_t ); // ???

	void setCamera( vec3d_t, vec3d_t );
	void setZoom( float );
	void drawSelf( void );

	void render( void );
	void renderCSurfaces( void );
	void renderCPolys( void );
	void render_csg( void );


	// draw
	void	startDraw( void );
	void	endDraw( void );
	void	setColor( QColor *color );
	void	draw3dLine( vec3d_t from, vec3d_t to );
	void	draw3dPolygon( vec3d_t p[], int pointnum );

protected:
	virtual void		resizeEvent( QResizeEvent * );
	virtual void		mousePressEvent( QMouseEvent * );

signals:
	// supports only type 0 ( press ).
	void	cameraRaySignal( int type, int state, Vec3 from, Vec3 to );

private:
	


	QPainter	*qp_draw;
	QColor		*qc_drawcolor;
	QBrush		*qb_drawbrush;

	vec3d_t		origin;
	vec3d_t		lookat;

	float		scale;

	float		drawwidth;
	float		drawheight;

};

#endif // __CameraView_included
