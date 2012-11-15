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



// YView.hh

#ifndef __YView_included
#define __YView_included

#include <math.h>
#include <stdlib.h>

#include <qwidget.h>
#include <qscrollbar.h>
#include <qpainter.h>
#include <qpixmap.h>

#include "vec.h"
#include "brush.h"

#include "VecMath.hh"

class YView;

extern YView		*yview_i;

void YStartDraw( void );
void YEndDraw( void );
void YColor( QColor *color );
void YDrawLine( vec3d_t from, vec3d_t to );

void YDrawHLine( vec3d_t v );
void YDrawVLine( vec3d_t v );
void YDrawCross( vec3d_t v );

void YDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );

/*                                                                              
  ===============================================                               
  class: YView
  ===============================================                               
*/                                                                              

class YView : public QWidget
{
	Q_OBJECT

	friend void YStartDraw( void );
	friend void YEndDraw( void );
	friend void YColor( QColor *color );
	friend void YDrawLine( vec3d_t from, vec3d_t to );

	friend void YDrawHLine( vec3d_t v );
	friend void YDrawVLine( vec3d_t v );
	friend void YDrawCross( vec3d_t v );

	friend void YDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color );

public:
	YView( QWidget *parent = 0, const char *name = 0);
	virtual ~YView();

	void	setSuperBounds( vec2d_t min, vec2d_t max ); // vec3d, vec3d
	void	getViewBounds( vec2d_t min, vec2d_t max ); // float, float

	void	setZoom( float zoom, int pagestep, int linestep, int dragstep );
	void	setOrigin( vec2d_t argorigin ); // vec3d
	void	getOrigin( vec2d_t argorigin ); // float
	void	setGrid( int arggrid);

	void    drawGrid( float ); // private
	void	drawSelf( void );
	void	drawLine( vec2d_t, vec2d_t );    

	void	back2Screen( void );
	void	screen2Back( void );

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
	
	virtual	void	resizeEvent ( QResizeEvent * );
	virtual void	mouseMoveEvent( QMouseEvent * );
	virtual void	mousePressEvent( QMouseEvent * );
	virtual void	mouseReleaseEvent( QMouseEvent * );
	
	virtual void	enterEvent( QEvent * );
	virtual void	leaveEvent( QEvent * );

private slots:
	void	yScrollSlot( int );

signals:
	void	originChangedSignal();
	void	yPressSignal( Vec2 );

	// type :  0 = press, 1 = move, 2 = release
	// state : see Qt QMouseEvent
	// 1. vec2d_t snap
	// 2. vec2d_t real
	void	yMouseEventSignal( int type, int state, Vec2, Vec2 );

private:
	QScrollBar	*qsb_y;
	QPainter	*qp_draw;
	QColor		*qc_drawcolor;
	QPixmap		*qpm_backscreen;

	float		scale;
	vec2d_t		origin; // vec3d

	int		grid;
	
	vec2d_t		smin, smax; // super bounds // vec3d
	vec2d_t		vmin, vmax; // view bounds  // float

	float		drawwidth; // painter
	float		drawheight;
	
};

#endif // __YView_included
