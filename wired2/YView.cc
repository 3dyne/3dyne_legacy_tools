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



// YView.cc

#include "Wired.hh"
#include "YView.hh"


YView		*yview_i;

// friends of YView

void YStartDraw( void )
{
	yview_i->qp_draw->begin( yview_i );
	yview_i->qp_draw->setClipRect( 0, 0, (int)yview_i->drawwidth, (int)yview_i->drawheight );
}

void YEndDraw( void )
{
	yview_i->qp_draw->end();
	yview_i->qc_drawcolor = NULL;
}

void YColor( QColor *color )
{
	if ( yview_i -> qc_drawcolor != color ) {
		yview_i -> qc_drawcolor = color;
		yview_i -> qp_draw -> setPen( *color );
	}	
}

void YDrawLine( vec3d_t from, vec3d_t to )
{
	vec2d_t		v0, v1;

	v0[0] = from[0];
	v0[1] = from[1];
	v1[0] = to[0];
	v1[1] = to[1];

	yview_i->vecWorld2Painter( v0, v0 );
	yview_i->vecWorld2Painter( v1, v1 );

	yview_i->qp_draw->drawLine( (int)v0[0], (int)v0[1], (int)v1[0], (int)v1[1]);
}

void YDrawHLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = 0;
	v2[1] = v[1];

	yview_i->vecWorld2Painter( v2, v2 );
	yview_i->qp_draw->drawLine( 0, (int)v2[1], (int)yview_i->drawwidth, (int)v2[1] );
}

void YDrawVLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = 0;

	yview_i->vecWorld2Painter( v2, v2 );
	yview_i->qp_draw->drawLine( (int)v2[0], 0, (int)v2[0], (int)yview_i->drawheight );
}

void YDrawCross( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[1];

	yview_i->vecWorld2Painter( v2, v2 );
	yview_i->qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );
	yview_i->qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]+3, (int)v2[1]-3 );
}

void YDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color )
{
	vec2d_t		v2;
	int		pw, ph;

	v2[0] = v[0];
	v2[1] = v[1];
	
	pw = qpm->width();
	ph = qpm->height();

	yview_i->vecWorld2Painter( v2, v2 );
	yview_i->qp_draw->drawPixmap( (int)v2[0]-pw/2, (int)v2[1]-ph/2, *qpm );

	if ( color )
	{
		YColor( color );
		yview_i->qp_draw->drawRect( (int)v2[0]-pw/2, (int)v2[1]-ph/2, pw, ph );
	}
}


/*                                                                              
  ===============================================                               
  class: YView                                                              
  ===============================================                               
*/                                                                              

YView::YView( QWidget *parent, const char *name )
	: QWidget( parent, name )
{

	yview_i = this;

	Vec2dInit( origin, 0, 0 );    
	Vec2dInit( vmin, -1, -1 );
	Vec2dInit( vmax, 1, 1 );
	Vec2dInit( smin, -10, -10 );
	Vec2dInit( smax, 10, 10 );
	scale = 1;
	grid = 16;

	qpm_backscreen = new QPixmap();

	qsb_y = new QScrollBar( QScrollBar::Vertical, this );       	

	qp_draw = new QPainter();
	qc_drawcolor = NULL;
       
	connect( qsb_y, SIGNAL( valueChanged( int ) ), this, SLOT( yScrollSlot( int ) ) );
}

YView::~YView()
{
	delete qsb_y;
	delete qp_draw;
}

void YView::setSuperBounds( vec2d_t min, vec2d_t max )
{

	// new bounding box of the world
	Vec2dCopy( smin, min );
	Vec2dCopy( smax, max );

	// set new scroll bar range
	setScrollBarRange();
}	


void YView::getViewBounds( vec2d_t min, vec2d_t max )
{      
	Vec2dCopy( min, vmin );
	Vec2dCopy( max, vmax );
}

void YView::setZoom( float zoom, int pagestep, int linestep, int /*dragstep*/ )
{
	scale = zoom;
	
	// set scroll bar steps
	qsb_y->setSteps( linestep, pagestep );

	setViewBounds();
	drawSelf();
}

void YView::setOrigin( vec2d_t argorigin )
{
	Vec2dCopy( origin, argorigin );

	// set scroll bar value
	qsb_y->blockSignals( true );
	qsb_y->setValue( (int) -origin[1] );
	qsb_y->blockSignals( false );

	setViewBounds();
	drawSelf();
}

void YView::getOrigin( vec2d_t argorigin ) 
{
	Vec2dCopy( argorigin, origin );
}

void YView::setGrid( int arggrid )
{
	grid = arggrid;
	drawSelf();
}

// drawGrid should be private
void YView::drawGrid( float arggrid )
{
	int		vgrid, x, y, y1;

	vgrid = (int) (arggrid * scale + 0.5 );
	if ( vgrid >= 2 ) {
		x = (int) (-origin[0] * scale + drawwidth / 2.0 + 0.5 );
                y = (int) (-origin[1] * scale + drawheight / 2.0 + 0.5 );
		
		x = x%vgrid;
		y1 = y%vgrid;
		
		for ( ; x < drawwidth; x+=vgrid ) {
			y = y1;
			for ( ; y < drawheight; y+=vgrid ) {
				qp_draw->drawPoint( x, (int)(drawheight - y) );
			}
		}
 	}

}

void YView::drawSelf( void )
{
	vec2d_t		v;

//	int		vgrid, x, y, y1;

	printf("YView::drawSelf.\n");

	v[0]=v[1]=0.0;

	qp_draw->begin( this );
	qp_draw->eraseRect( 0, 0, (int)drawwidth, (int)drawheight );

	// draw origin
	vecWorld2Painter( v, v );
	qp_draw->drawLine( (int) v[0], 0, (int) v[0], (int)drawheight );             
        qp_draw->drawLine( 0, (int) v[1], (int)drawwidth, (int) v[1] );	

	if ( grid < 64 )
	{
		// draw grid
		qp_draw->setPen( *colorblack_i );
		drawGrid( grid );
		
		// draw 64 base grid
		qp_draw->setPen( *coloryellow_i );
		drawGrid( 64.0 );
	}
	else if ( grid > 64 )
	{
		// draw 64 base grid
		qp_draw->setPen( *coloryellow_i );
		drawGrid( 64.0 );

		qp_draw->setPen( *colorblack_i );
		drawGrid( grid );
	}
	else
	{
		// draw grid
		qp_draw->setPen( *colorblack_i );
		drawGrid( grid );	
	}
	
	qp_draw->end();
}


void YView::screen2Back( void )
{
	bitBlt( qpm_backscreen, 0, 0, this, 0, 0, (int)drawwidth, (int)drawheight );
}

void YView::back2Screen( void )
{
	bitBlt( this, 0, 0, qpm_backscreen, 0, 0, (int)drawwidth, (int)drawheight );
}


//
// draw
//

void YView::startDraw( void )
{
	qp_draw->begin( this );
	qp_draw->setClipRect( 0, 0, (int)drawwidth, (int)drawheight );

}

void YView::endDraw( void )
{
	qp_draw->end();
	qc_drawcolor = NULL;
}

void YView::setColor( QColor *color )
{
	if ( qc_drawcolor != color ) {
		qc_drawcolor = color;
		qp_draw -> setPen( *color );
	}	
}

void YView::setPenStyle( QPen *pen )
{
	qp_draw->setPen( *pen );
}

void YView::draw3dLine( vec3d_t from, vec3d_t to, int shift )
{
	vec2d_t		v0, v1;
	int		xs0, ys0, xs1, ys1;

	v0[0] = from[0];
	v0[1] = from[1];
	v1[0] = to[0];
	v1[1] = to[1];

	vecWorld2Painter( v0, v0 );
	vecWorld2Painter( v1, v1 );

#if 1
	xs0 = xs1 = ys0 = ys1 = 0;
	if ( v0[0] > v1[0] )
	{
		// x1 < x2 : forward
//		ys0 = ys1 = shift;
		ys0 += shift;
		ys1 += shift;
	}
	else if ( v0[0] < v1[0] )
	{
//		ys0 = ys1 = -shift;
		ys0 -= shift;
		ys1 -= shift;
	}

	if ( v0[1] < v1[1] )
	{
		// y1 < y2 : up
//		xs0 = xs1 = shift;
		xs0 += shift;
		xs1 += shift;
	}
	else if ( v0[1] > v1[1] )
	{
//		xs0 = xs1 = -shift;
		xs0 -= shift;
		xs1 -= shift;
	}
#endif

	qp_draw->drawLine( (int)v0[0]+xs0, (int)v0[1]+ys0, (int)v1[0]+xs1, (int)v1[1]+ys1 );
}

void YView::draw3dHLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = 0;
	v2[1] = v[1];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( 0, (int)v2[1], (int)drawwidth, (int)v2[1] );

}

void YView::draw3dVLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = 0;

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0], 0, (int)v2[0], (int)drawheight );

}

void YView::draw3dCross( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[1];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]+3, (int)v2[1]-3 );

}


void YView::draw3dBox( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[1];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]-3 );	
	qp_draw->drawLine( (int)v2[0]+3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );	
	qp_draw->drawLine( (int)v2[0]+3, (int)v2[1]+3, (int)v2[0]-3, (int)v2[1]+3 );	
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]-3, (int)v2[1]-3 );	
}

void YView::draw3dQPixmap( vec3d_t v, QPixmap *qpm, QColor *color )
{
	vec2d_t		v2;
	int		pw, ph;

	v2[0] = v[0];
	v2[1] = v[1];
	
	pw = qpm->width();
	ph = qpm->height();

	vecWorld2Painter( v2, v2 );
	qp_draw->drawPixmap( (int)v2[0]-pw/2, (int)v2[1]-ph/2, *qpm );

	if ( color )
	{
		setColor( color );
		qp_draw->drawRect( (int)v2[0]-pw/2, (int)v2[1]-ph/2, pw, ph );
	}

}

static int	cull = 0, draw = 0;
void YView::draw3dFace( face_t *f, int shift )
{
	int		i;

	if ( f->plane.norm[2] <= 0.0 )
	{
		cull++;
		return;
	}
	draw++;

	if ( !f->polygon )
		return;

	for ( i = 0; i < f->polygon->pointnum; i++ ) {
		draw3dLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum], shift );
	}
//	printf( "cull: %d, draw: %d\n", cull, draw );
}

//

void YView::vecWorld2Painter( vec2d_t out, vec2d_t in )                     
{                                                                               
        vec2d_t         v;                                                      
        v[0] = in[0];                                                          
        v[1] = in[1];                                                          
                                                                                
        v[1] = -v[1];                                                           
        v[0]-= origin[0];                                                       
        v[1]+= origin[1];                                                       
                                                                                
        v[0] *= scale;                                                       
        v[1] *= scale;                                                       
                                                                                
        out[0] = v[0] + ( drawwidth / 2.0 );                                   
        out[1] = v[1] + ( drawheight / 2.0 );                                  
}    

void YView::vecPainter2World( vec2d_t out, vec2d_t in )                     
{                                                                               
        out[0] = origin[0] + ( in[0] - ( drawwidth / 2.0 ) ) / scale;     
        out[1] = -(-origin[1] + ( in[1] - ( drawheight / 2.0 ) ) / scale);
}                                                                               

void YView::vecWorld2Grid( vec2d_t out, vec2d_t in )
{
	float	realgrid, realgrid_2;

	if ( !grid ) {
		out[0] = in[0];
		out[1] = in[1];
		return;
	}
		
	realgrid = (float) grid;
	realgrid_2 = realgrid / 2.0;

	out[0] = (float)(floor((double)((in[0]+realgrid_2)/realgrid))*realgrid);
	out[1] = (float)(floor((double)((in[1]+realgrid_2)/realgrid))*realgrid);	
}

void YView::setViewBounds( void )
{
	vec2d_t v;

	v[0] = 0;
	v[1] = drawheight;
	vecPainter2World( vmin, v );

	v[0] = drawwidth;
	v[1] = 0;
	vecPainter2World( vmax, v );

//	setScrollBarRange();
}

void YView::setScrollBarRange( void )
{
	qsb_y->setRange( (int)( smin[1] + (vmax[1]-vmin[1])/2) , (int)(smax[1] - (vmax[1]-vmin[1])/2 ));
}



void YView::resizeEvent( QResizeEvent * )
{
	printf("YView::resizeEvent\n");

	qsb_y->setGeometry( width()-16, 0, 16, height() );
	
	drawwidth = width()-16;
	drawheight = height();

	qpm_backscreen->resize( (int)drawwidth, (int)drawheight );
	
	setViewBounds();
	setScrollBarRange();
	drawSelf();
}


/*
  ====================
  Slots
  ====================
*/

void YView::yScrollSlot( int y )
{
	y = -y;
	printf("YView::zScrollSlot %d\n", y );
	
	origin[1] = (float) y;
	setViewBounds();
	drawSelf();
	emit originChangedSignal();
}



/*
  ======================
  Events
  ======================
*/

void YView::mousePressEvent( QMouseEvent *e )
{
	vec2d_t		pos;
	vec2d_t		snap;

	int		ex, ey;
	int		s;

	ex = e->x();
	ey = e->y();
	s = e->state() | e->button(); // state enthaelt bei einem press-event nicht die taste

	pos[0] = ex;
	pos[1] = ey;
	vecPainter2World( pos, pos );
	vecWorld2Grid( snap, pos );

	emit yPressSignal( pos );
	emit yMouseEventSignal( 0, s, snap, pos );	
}

void YView::mouseReleaseEvent( QMouseEvent *e )
{
	int	ex, ey;
	int		s;
	vec2d_t		pos;
	vec2d_t		snap;
	
	ex = e->x();
	ey = e->y();
	s = e->state();
	
	pos[0] = ex;
	pos[1] = ey;
	vecPainter2World( pos, pos );
	vecWorld2Grid( snap, pos );
	
	emit yMouseEventSignal( 2, s, snap, pos );	
}

void YView::mouseMoveEvent( QMouseEvent *e )
{
	vec2d_t		pos;
	vec2d_t		snap;

	int		ex, ey;
	int		s;

	ex = e->x();
	ey = e->y();
	s = e->state();

	pos[0] = ex;
	pos[1] = ey;
	vecPainter2World( pos, pos );
	vecWorld2Grid( snap, pos );
	emit yMouseEventSignal( 1, s, snap, pos );
}

void YView::enterEvent( QEvent * )
{
	printf( "YView::enterEvent\n" );
	wired_i->enableAccel();
}

void YView::leaveEvent( QEvent * )
{
	printf( "YView::leaveEvent\n" );
	wired_i->disableAccel();
}
