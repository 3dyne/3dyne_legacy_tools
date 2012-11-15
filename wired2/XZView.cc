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



// XZView.cc

#include "Wired.hh"
#include "XZView.hh"


XZView		*xzview_i;

// friends of XZView

void XZStartDraw( void ) 
{
	xzview_i->qp_draw->begin( xzview_i );
	xzview_i->qp_draw->setClipRect( 0, 0, (int)xzview_i->drawwidth, (int)xzview_i->drawheight );
}

void XZEndDraw( void )
{
	xzview_i->qp_draw->end();
	xzview_i->qc_drawcolor = NULL;
}

void XZColor( QColor *color )
{
	if ( xzview_i -> qc_drawcolor != color ) {
		xzview_i -> qc_drawcolor = color;
		xzview_i -> qp_draw -> setPen( *color );
	}
}

void XZDrawLine( vec3d_t from, vec3d_t to )
{
	vec2d_t		v0, v1;

	v0[0] = from[0];
	v0[1] = from[2];
	v1[0] = to[0];
	v1[1] = to[2];

	xzview_i->vecWorld2Painter( v0, v0 );
	xzview_i->vecWorld2Painter( v1, v1 );

	xzview_i->qp_draw->drawLine( (int)v0[0], (int)v0[1], (int)v1[0], (int)v1[1]);
}


void XZDrawHLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = 0;
	v2[1] = v[2];

	xzview_i->vecWorld2Painter( v2, v2 );
	xzview_i->qp_draw->drawLine( 0, (int)v2[1], (int)xzview_i->drawwidth, (int)v2[1] );

}

void XZDrawVLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = 0;

	xzview_i->vecWorld2Painter( v2, v2 );
	xzview_i->qp_draw->drawLine( (int)v2[0], 0, (int)v2[0], (int)xzview_i->drawheight );
}

void XZDrawCross( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[2];

	xzview_i->vecWorld2Painter( v2, v2 );
	xzview_i->qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );
	xzview_i->qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]+3, (int)v2[1]-3 );
}

void XZDrawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color )
{
	vec2d_t		v2;
	int		pw, ph;

	v2[0] = v[0];
	v2[1] = v[2];
	
	pw = qpm->width();
	ph = qpm->height();

	xzview_i->vecWorld2Painter( v2, v2 );
	xzview_i->qp_draw->drawPixmap( (int)v2[0]-pw/2, (int)v2[1]-ph/2, *qpm );

	if ( color )
	{
		XZColor( color );
		xzview_i->qp_draw->drawRect( (int)v2[0]-pw/2, (int)v2[1]-ph/2, pw, ph );
	}
}

/*                                                                              
  ===============================================                               
  class: XZView                                                              
  ===============================================                               
*/                                                                              

XZView::XZView( QWidget *parent, const char *name )
	: QWidget( parent, name )
{

	xzview_i = this;

	Vec2dInit( origin, 0, 0 );    
	Vec2dInit( vmin, -1, -1 );
	Vec2dInit( vmax, 1, 1 );
	Vec2dInit( smin, -10, -10 );
	Vec2dInit( smax, 10, 10 );
	scale = 1;
	grid = 16;

	panflag = false;

	qpm_backscreen = new QPixmap();
	
	qsb_x = new QScrollBar( QScrollBar::Horizontal, this );
	qsb_z = new QScrollBar( QScrollBar::Vertical, this );       	

	qp_draw = new QPainter();
	qc_drawcolor = NULL;

	connect( qsb_x, SIGNAL( valueChanged( int ) ), this, SLOT( xScrollSlot( int ) ) );
	connect( qsb_z, SIGNAL( valueChanged( int ) ), this, SLOT( zScrollSlot( int ) ) );
}

XZView::~XZView()
{
	delete qsb_x;
	delete qsb_z;
	delete qp_draw;
}

void XZView::setSuperBounds( vec2d_t min, vec2d_t max )
{
	
	// new bounding box of the world
	Vec2dCopy( smin, min );
	Vec2dCopy( smax, max );

	// set new scroll bar range
	setScrollBarRange();
}

void XZView::getViewBounds( vec2d_t min, vec2d_t max )
{
	Vec2dCopy( min, vmin );
	Vec2dCopy( max, vmax );
}

void XZView::setZoom( float zoom, int pagestep, int linestep, int /*dragstep*/ )
{
	scale = zoom;
	
	// set scroll bar steps
	qsb_x->setSteps( linestep, pagestep );
	qsb_z->setSteps( linestep, pagestep );

	setViewBounds();
	drawSelf();
}

void XZView::setOrigin( vec2d_t argorigin )
{
	Vec2dCopy( origin, argorigin );

	// set scroll bar value
	qsb_x->blockSignals( true );
	qsb_z->blockSignals( true );

	qsb_x->setValue( (int) origin[0] );
	qsb_z->setValue( (int) -origin[1] );

	qsb_x->blockSignals( false );
	qsb_z->blockSignals( false );

	setViewBounds();
	drawSelf();
}

void XZView::getOrigin( vec2d_t argorigin ) 
{
	Vec2dCopy( argorigin, origin );
}

void XZView::setGrid( int arggrid )
{
	grid = arggrid;

	drawSelf();
}

// drawGrid should be private
void XZView::drawGrid( float arggrid )
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

void XZView::drawSelf( void )
{
	vec2d_t		v;


	printf("XZView::drawSelf.\n");

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


//
// public draw
//
void XZView::startDraw( void )
{
	qp_draw->begin( this );
	qp_draw->setClipRect( 0, 0, (int)drawwidth, (int)drawheight );
}

void XZView::endDraw( void )
{
	qp_draw->end();
	qc_drawcolor = NULL;	
}

void XZView::setColor( QColor *color )
{
	if ( qc_drawcolor != color ) {
		qc_drawcolor = color;
		qp_draw -> setPen( *color );
	}	
}

void XZView::setPenStyle( QPen *pen )
{
	qp_draw->setPen( *pen );
}

void XZView::draw3dLine( vec3d_t from, vec3d_t to, int shift )
{
	vec2d_t		v0, v1;
	int		xs0, ys0, xs1, ys1;
	
	v0[0] = from[0];
	v0[1] = from[2];
	v1[0] = to[0];
	v1[1] = to[2];

	vecWorld2Painter( v0, v0 );
	vecWorld2Painter( v1, v1 );
#if 1
	xs0 = xs1 = ys0 = ys1 = 0;
	if ( v0[0] < v1[0] )
	{
		// x1 < x2 : forward
//		ys0 = ys1 = shift;
		ys0 += shift;
		ys1 += shift;
	}
	else if ( v0[0] > v1[0] )
	{
//		ys0 = ys1 = -shift;
		ys0 -= shift;
		ys1 -= shift;
	}

	if ( v0[1] > v1[1] )
	{
		// y1 < y2 : up
//		xs0 = xs1 = shift;
		xs0 += shift;
		xs1 += shift;
	}
	else if ( v0[1] < v1[1] )
	{
//		xs0 = xs1 = -shift;
		xs0 -= shift;
		xs1 -= shift;
	}
#endif

	qp_draw->drawLine( (int)v0[0]+xs0, (int)v0[1]+ys0, (int)v1[0]+xs1, (int)v1[1]+ys1 );	
}

void XZView::draw3dHLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = 0;
	v2[1] = v[2];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( 0, (int)v2[1], (int)drawwidth, (int)v2[1] );
	
}

void XZView::draw3dVLine( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = 0;

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0], 0, (int)v2[0], (int)drawheight );
}

void XZView::draw3dCross( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[2];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]+3, (int)v2[1]-3 );

}

void XZView::draw3dBox( vec3d_t v )
{
	vec2d_t		v2;

	v2[0] = v[0];
	v2[1] = v[2];

	vecWorld2Painter( v2, v2 );
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]-3 );	
	qp_draw->drawLine( (int)v2[0]+3, (int)v2[1]-3, (int)v2[0]+3, (int)v2[1]+3 );	
	qp_draw->drawLine( (int)v2[0]+3, (int)v2[1]+3, (int)v2[0]-3, (int)v2[1]+3 );	
	qp_draw->drawLine( (int)v2[0]-3, (int)v2[1]+3, (int)v2[0]-3, (int)v2[1]-3 );		
}

void XZView::draw3dQPixmap( vec3d_t v, QPixmap *qpm, QColor *color )
{
	vec2d_t		v2;
	int		pw, ph;

	v2[0] = v[0];
	v2[1] = v[2];
	
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
void XZView::draw3dFace( face_t *f, int shift )
{
	int		i;

	if ( f->plane.norm[1] <= 0.0 )
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


void XZView::screen2Back( void )
{
	bitBlt( qpm_backscreen, 0, 0, this, 0, 0, (int)drawwidth, (int)drawheight );
}

void XZView::back2Screen( void )
{
	bitBlt( this, 0, 0, qpm_backscreen, 0, 0, (int)drawwidth, (int)drawheight );
}

void XZView::vecWorld2Painter( vec2d_t out, vec2d_t in )                     
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

void XZView::vecPainter2World( vec2d_t out, vec2d_t in )                     
{                                                                               
        out[0] = origin[0] + ( in[0] - ( drawwidth / 2.0 ) ) / scale;     
        out[1] = -(-origin[1] + ( in[1] - ( drawheight / 2.0 ) ) / scale);
}                                                                               

void XZView::vecWorld2Grid( vec2d_t out, vec2d_t in ) 
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

void XZView::setViewBounds( void )
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

void XZView::setScrollBarRange( void )
{
	qsb_x->setRange( (int)(smin[0] + (vmax[0]-vmin[0])/2) , (int)(smax[0] - (vmax[0]-vmin[0])/2) );
	qsb_z->setRange( (int)(smin[1] + (vmax[1]-vmin[1])/2) , (int)(smax[1] - (vmax[1]-vmin[1])/2) );
}

/*
  ======================
  Events
  ======================
*/

void XZView::mouseMoveEvent( QMouseEvent *e )
{
	vec2d_t		pos;
	vec2d_t		snap;

	int		ex, ey;
	int		s;

	printf("XZView::mouseMoveEvent\n");

	ex = e->x();
	ey = e->y();	
	s = e->state();

	pos[0] = ex;
	pos[1] = ey;
	vecPainter2World( pos, pos );
	vecWorld2Grid( snap, pos );
	emit xzMoveSignal( pos );
	emit xzMouseEventSignal( 1, s, snap, pos );
}

void XZView::mousePressEvent( QMouseEvent *e )
{
	vec2d_t		pos;
	vec2d_t		snap;

	int		ex, ey;
	int		s;

	ex = e->x();
	ey = e->y();
	s = e->state() | e->button(); // state enthaelt bei einem press-event nicht die taste

//	printf("XZView::mousePressEvent button = %d\n", e->button() ); 
//	printf(" panner pressed at %d, %d\n", ex, ey );
	if ( ex > drawwidth && ey > drawheight ) {
		// der panner bereich 
		panflag = true;
		ypan = ey;
		printf(" panner pressed at %d, %d\n", ex, ey );
	}
	else {

		pos[0] = ex;
		pos[1] = ey;
		
		vecPainter2World( pos, pos );
		vecWorld2Grid( snap, pos );
		printf("XZView::mousePressEvent %f, %f\n", pos[0], pos[1] );
		emit xzPressSignal( pos );
		emit xzMouseEventSignal( 0, s, snap, pos );
	}
}

void XZView::mouseReleaseEvent( QMouseEvent *e )
{
	int	ex, ey;
	int		s;
	vec2d_t		pos;
	vec2d_t		snap;

	ex = e->x();
	ey = e->y();
	s = e->state();
	
	if ( panflag ) {
		panflag = false;
		printf(" panner released at %d, %d\n", ex, ey );
		emit pannerChangedSignal( ey - ypan );
	}	
	else {
		pos[0] = ex;
		pos[1] = ey;
		vecPainter2World( pos, pos );
		vecWorld2Grid( snap, pos );
		emit xzReleaseSignal( pos );
		emit xzMouseEventSignal( 2, s, snap, pos );
	}
}

void XZView::resizeEvent( QResizeEvent * )
{
	printf("XZView::resizeEvent\n");

	qsb_x->setGeometry( 0, height()-16, width()-16, 16 );
	qsb_z->setGeometry( width()-16, 0, 16, height()-16 );
	
	drawwidth = width()-16;
	drawheight = height()-16;

	qpm_backscreen->resize( (int)drawwidth, (int)drawheight );

//	qp_draw->setClipRect( 0, 0, drawwidth, drawheight );

	setViewBounds();
	setScrollBarRange();
	drawSelf();
}

void XZView::enterEvent( QEvent * )
{
	printf( "XZView::enterEvent\n" );
	wired_i->enableAccel();
}

void XZView::leaveEvent( QEvent * )
{
	printf( "XZView::leaveEvent\n" );
	wired_i->disableAccel();
}
/*
  ====================
  Slots
  ====================
*/

void XZView::xScrollSlot( int x )
{
	printf("XZView::xScrollSlot %d\n", x );

	origin[0] = (float) x;
	setViewBounds();
	drawSelf();
	emit originChangedSignal();
}

void XZView::zScrollSlot( int z )
{
	z = -z;
	printf("XZView::zScrollSlot %d\n", z );
	
	origin[1] = (float) z;
	setViewBounds();
	drawSelf();
	emit originChangedSignal();
}
