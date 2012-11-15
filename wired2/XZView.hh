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



// XZView.hh

#ifndef __XZView_included
#define __XZView_included

#include <math.h>
#include <stdlib.h>

#include <qwidget.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "vec.h"
#include "brush.h"

#include "VecMath.hh"

class XZView;

extern XZView		*xzview_i;

void XZStartDraw( void ); 
void XZEndDraw( void );
void XZColor( QColor *color );
void XZDrawLine( vec3d_t from, vec3d_t to );

void XZDrawHLine( vec3d_t v );
void XZDrawVLine( vec3d_t v );
void XZDrawCross( vec3d_t v );

void XZDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );

/*                                                                              
  ===============================================                               
  class: XZView                                                              
  ===============================================                               
*/                                                                              
 
class XZView : public QWidget
{
	Q_OBJECT

	friend void XZStartDraw( void );
	friend void XZEndDraw( void );
	friend void XZColor( QColor *color );
	friend void XZDrawLine( vec3d_t from, vec3d_t to );

	friend void XZDrawHLine( vec3d_t v );
	friend void XZDrawVLine( vec3d_t v );
	friend void XZDrawCross( vec3d_t v );
	friend void XZDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );
public:
	XZView( QWidget *parent = 0, const char *name = 0);
	virtual ~XZView();

	void	setSuperBounds( vec2d_t min, vec2d_t max );
	void	getViewBounds( vec2d_t min, vec2d_t max );

	void	setZoom( float zoom, int pagestep, int linestep, int dragstep );
	void	setOrigin( vec2d_t argorigin );
	void	getOrigin( vec2d_t argorigin );
	void	setGrid( int arggrid);

	void	drawGrid( float ); // private
	void	drawSelf( void );
	void	drawLine( vec2d_t, vec2d_t );    

	void	screen2Back( void );
	void	back2Screen( void );

	// draw
	void	startDraw( void );
	void	endDraw( void );
	void	setColor( QColor *color );
	void	setPenStyle( QPen *pen );
	void	draw3dLine( vec3d_t from, vec3d_t to, int shift = 0 );	
	void	draw3dHLine( vec3d_t v );
	void	draw3dVLine( vec3d_t v );
	void	draw3dCross( vec3d_t v );
	void	draw3dBox( vec3d_t v );
	void	draw3dQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );
	void	draw3dFace( face_t *f, int shift = 0 );
	
	
protected:
	void		vecWorld2Painter( vec2d_t out, vec2d_t in );
	void		vecPainter2World( vec2d_t out, vec2d_t in );
	void		vecWorld2Grid( vec2d_t out, vec2d_t in );
	void		setViewBounds( void );
	void		setScrollBarRange( void ); // vbound, sbound

	virtual void	resizeEvent ( QResizeEvent * );
	virtual void	mouseMoveEvent( QMouseEvent * );
	virtual void	mousePressEvent( QMouseEvent * );
	virtual void	mouseReleaseEvent( QMouseEvent * );

	virtual void	enterEvent( QEvent * );
	virtual void	leaveEvent( QEvent * );

private slots:
	void	xScrollSlot( int );
	void	zScrollSlot( int );

signals:
	void	originChangedSignal();
	void	pannerChangedSignal( int );

	// type :  0 = press, 1 = move, 2 = release
	// state : see Qt QMouseEvent
	void	xzPressSignal( Vec2 ); // type 0
	void	xzMoveSignal( Vec2 );  // type 1
	void	xzReleaseSignal( Vec2 ); // type 2
	// 1. vec2d_t snapped world
	// 2. vec2d_t none-snapped world
	void	xzMouseEventSignal( int type, int state, Vec2, Vec2 ); // one of the three above

private:

	QScrollBar	*qsb_x;
	QScrollBar	*qsb_z;
	QPainter	*qp_draw;
	QColor		*qc_drawcolor; 
	QPixmap		*qpm_backscreen;
	
	float		scale;
	vec2d_t		origin;

	int		grid;

	vec2d_t		smin, smax; // super bounds
	vec2d_t		vmin, vmax; // view bounds

	float		drawwidth; // painter
	float		drawheight;

	bool		panflag;
	int		ypan;
};

#endif // __XZView_included
