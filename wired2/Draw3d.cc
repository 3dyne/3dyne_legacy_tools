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



// Draw3d.cc

#include "Draw3d.hh"

/*
  ==================================================
  class: Draw3d

  ==================================================
*/

Draw3d::Draw3d( void )
{
	printf( "Draw3d::Draw3d\n" );
	active_views = VIEW_NONE;
}

Draw3d::~Draw3d()
{
	printf( "Draw3d::~Draw3d\n" );
}

/*
  ====================
  startDraw

  ====================
*/
void Draw3d::startDraw( int views )
{
	if ( ( views & VIEW_XZ ) )
	{
		if ( ! ( active_views & VIEW_XZ ) )
		{
			active_views |= VIEW_XZ;
			xzview_i->startDraw();
		}
	}

	if ( ( views & VIEW_Y ) )
	{
		if ( ! ( active_views & VIEW_Y ) )
		{
			active_views |= VIEW_Y;
			yview_i->startDraw();
		}
	}

	if ( ( views & VIEW_CAMERA ) )
	{
		if ( ! ( active_views & VIEW_CAMERA ) )
		{
			active_views |= VIEW_CAMERA;
			cameraview_i->startDraw();
		}
	}
}



/*
  ====================
  endDraw

  ====================
*/
void Draw3d::endDraw( void )
{
	if ( ( active_views & VIEW_XZ ) )
	{
		xzview_i->endDraw();		
	}

	if ( ( active_views & VIEW_Y ) )
	{
		yview_i->endDraw();
	}

	if ( ( active_views & VIEW_CAMERA ) )
	{
		cameraview_i->endDraw();
	}

	active_views = VIEW_NONE;
}


#define CHECKV( _V ) ( active_views & VIEW_##_V )

/*
  ====================
  setColor

  ====================
*/
void Draw3d::setColor( QColor *color )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->setColor( color );
	}
	if ( CHECKV( Y ) )
	{
		yview_i->setColor( color );
	}
	if ( CHECKV( CAMERA ) )
	{
		cameraview_i->setColor( color );
	}
}



/*
  ====================
  setPenStyle

  ====================
*/
void Draw3d::setPenStyle( QPen *pen )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->setPenStyle( pen );
	}
	if ( CHECKV( Y ) )
	{
		yview_i->setPenStyle( pen );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
//		cameraview_i->setPenStyle( pen );
	}
}



/*
  ====================
  drawLine

  ====================
*/
void Draw3d::drawLine( vec3d_t from, vec3d_t to, int shift )
{
	if ( CHECKV( XZ ) ) 
	{
		xzview_i->draw3dLine( from, to, shift );
	}
	if ( CHECKV( Y ) )
	{
		yview_i->draw3dLine( from, to, shift );
	}
	if ( CHECKV( CAMERA ) )
	{
		cameraview_i->draw3dLine( from, to );
	}
}



/*
  ====================
  drawHLine

  ====================
*/
void Draw3d::drawHLine( vec3d_t v )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->draw3dHLine( v );
	}
	if ( CHECKV( Y ) ) 
	{
		yview_i->draw3dHLine( v );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
	}
}



/*
  ====================
  drawVLine

  ====================
*/
void Draw3d::drawVLine( vec3d_t v )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->draw3dVLine( v );
	}
	if ( CHECKV( Y ) ) 
	{
		yview_i->draw3dVLine( v );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
	}
}



/*
  ====================
  drawCross

  ====================
*/
void Draw3d::drawCross( vec3d_t v )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->draw3dCross( v );
	}
	if ( CHECKV( Y ) ) 
	{
		yview_i->draw3dCross( v );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
//		cameraview_i->draw3dCross( v );
	}
}



/*
  ====================
  drawBox

  ====================
*/
void Draw3d::drawBox( vec3d_t v )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->draw3dBox( v );
	}
	if ( CHECKV( Y ) ) 
	{
		yview_i->draw3dBox( v );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
//		cameraview_i->draw3dBox( v );
	}
}




/*
  ====================
  drawQPixmap

  ====================
*/
void Draw3d::drawQPixmap( vec3d_t v, QPixmap *qpm, QColor *color )
{
	if ( CHECKV( XZ ) )
	{
		xzview_i->draw3dQPixmap( v, qpm, color );
	}
	if ( CHECKV( Y ) )
	{
		yview_i->draw3dQPixmap( v, qpm, color );
	}
	if ( CHECKV( CAMERA ) )
	{
		// not available
	}
}



/*
  ====================
  drawBB

  ====================
*/
#define V3INIT( _v, _x1, _x2, _x3 ) {	\
	_v[0] = _x1;			\
	_v[1] = _x2;			\
	_v[2] = _x3;			\
	}

void Draw3d::drawBB( vec3d_t min, vec3d_t max )
{
	vec3d_t		v1, v2, v3, v4;
	vec3d_t		v5, v6, v7, v8;

	V3INIT( v1, min[0], min[1], min[2] );
	V3INIT( v2, min[0], max[1], min[2] );
	V3INIT( v3, min[0], max[1], max[2] );
	V3INIT( v4, min[0], min[1], max[2] );
	
	V3INIT( v5, max[0], min[1], min[2] );
	V3INIT( v6, max[0], max[1], min[2] );
	V3INIT( v7, max[0], max[1], max[2] );
	V3INIT( v8, max[0], min[1], max[2] );

	drawLine( v1, v2 );
	drawLine( v2, v3 );
	drawLine( v3, v4 );
	drawLine( v4, v1 );

	drawLine( v5, v6 );
	drawLine( v6, v7 );
	drawLine( v7, v8 );
	drawLine( v8, v5 );

	drawLine( v1, v5 );
	drawLine( v2, v6 );
	drawLine( v3, v7 );
	drawLine( v4, v8 );	
}

