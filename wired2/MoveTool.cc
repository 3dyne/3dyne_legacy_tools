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



// MoveTool.cc

#include "Wired.hh"
#include "WWM.hh"
#include "MoveTool.hh"

MoveTool	*movetool_i;

/*                                                                              
  ===============================================                               
  class: MoveTool                                                              
  ===============================================                               
*/                                                                              
 
MoveTool::MoveTool( QObject* parent, char* name )
	: QObject( parent, name )
{

	movetool_i = this;

	cur_iter = NULL;
}

MoveTool::~MoveTool()
{

}

void MoveTool::xzPressSlot( Vec2 v, unsigned int /*select*/ )
{
	printf("MoveTool::pressSlot\n");
	
	from[0] = v[0];
	from[1] = 0;
	from[2] = v[1];

	xzview_i->screen2Back();
	yview_i->screen2Back();
}

void MoveTool::xzDragSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
	
	printf("MoveTool::releaseSlot\n");

	to[0] = v[0];
	to[1] = 0;
	to[2] = v[1];

	Vec3dSub( move, to, from );
	if ( move[0]!=0 || move[1]!=0 || move[2]!=0 ) {

		xzview_i->back2Screen();
		yview_i->back2Screen();

		moveBrushes( select );
		wired_i->updateViews();
		Vec3dCopy( from, to );
	}
}

void MoveTool::xzReleaseSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
	
	printf("MoveTool::releaseSlot\n");

	to[0] = v[0];
	to[1] = 0;
	to[2] = v[1];

	Vec3dSub( move, to, from );
	Vec3dPrint( move );

	moveBrushes( select );

	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
}

void MoveTool::yPressSlot( Vec2 v, unsigned int /*select*/ )
{
	printf("MoveTool::pressSlot\n");
	
	from[0] = v[0];
	from[1] = v[1];
	from[2] = 0;

	xzview_i->screen2Back();
	yview_i->screen2Back();	
}

void MoveTool::yDragSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
	
	printf("MoveTool::releaseSlot\n");

	to[0] = v[0];
	to[1] = v[1];
	to[2] = 0;

	Vec3dSub( move, to, from );
	if ( move[0]!=0 || move[1]!=0 || move[2]!=0 ) {

		xzview_i->back2Screen();
		yview_i->back2Screen();

		moveBrushes( select );
		wired_i->updateViews();
		Vec3dCopy( from, to );
	}
}

void MoveTool::yReleaseSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
//	vec3d_t		move;
	
	printf("MoveTool::releaseSlot\n");

	to[0] = v[0];
	to[1] = v[1];
	to[2] = 0;

	Vec3dSub( move, to, from );
	Vec3dPrint( move );

	moveBrushes( select );

	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
}


void MoveTool::moveBrushes( unsigned int select )
{
	vec3d_t		norm, origin;
	float		dist;

	brush_t		*b;
	face_t		*f;	
	// move all selected brushes

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next ) {

		// all selected brushes were moved to the head of the list
		// so we can stop to search after the first not selected brush
		// was found
		if ( !(b->select&select ) )
			continue;

		for ( f = b->faces; f ; f=f->next ) {
			Vec3dCopy( norm, f->plane.norm );// norm = f->plane.norm;
			dist = f->plane.dist;
			
			Vec3dScale( origin, dist, norm );
			Vec3dAdd( origin, origin, move );
			
			dist = Vec3dDotProduct( origin, norm );

			Vec3dCopy( f->plane.norm, norm ); // f->plane.norm = norm;
			f->plane.dist = dist;
			
			// free old polygons
			FreePolygon( f->polygon );
		}	

		// generate new polygons
		ClipBrushFaces( b );

		b->select|=SELECT_UPDATE;
	}	
	
//	wired_i->updateViews();		
	
}


void MoveTool::Face_xzPressSlot( Vec2 v, unsigned int /*select*/ )
{
	from[0] = v[0];
	from[1] = wired_i->getYChecker();
	from[2] = v[1];

	xzview_i->screen2Back();
	yview_i->screen2Back();
}

void MoveTool::Face_xzDragSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
	
	to[0] = v[0];
	to[1] = wired_i->getYChecker();
	to[2] = v[1];

	Vec3dSub( move, to, from );
	if ( move[0]!=0 || move[1]!=0 || move[2]!=0 ) {

		xzview_i->back2Screen();
		yview_i->back2Screen();

		moveFaces( false, select ); // drag faces
		wired_i->updateViews();
		Vec3dCopy( from, to );
	}

}

void MoveTool::Face_xzReleaseSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;

	to[0] = v[0];
	to[1] = wired_i->getYChecker();
	to[2] = v[1];

	Vec3dSub( move, to, from );

	moveFaces( true, select ); // cleanup brush

	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
}

void MoveTool::Face_yPressSlot( Vec2 v, unsigned int /*select*/ )
{
	from[0] = v[0];
	from[1] = v[1];
	from[2] = wired_i->getZChecker();

	xzview_i->screen2Back();
	yview_i->screen2Back();
}

void MoveTool::Face_yDragSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;
	
	to[0] = v[0];
	to[1] = v[1];
	to[2] = wired_i->getZChecker();

	Vec3dSub( move, to, from );
	if ( move[0]!=0 || move[1]!=0 || move[2]!=0 ) {

		xzview_i->back2Screen();
		yview_i->back2Screen();

		moveFaces( false, select ); // drag faces
		wired_i->updateViews();
		Vec3dCopy( from, to );
	}
	
}

void MoveTool::Face_yReleaseSlot( Vec2 v, unsigned int select )
{
	vec3d_t		to;

	to[0] = v[0];
	to[1] = v[1];
	to[2] = wired_i->getZChecker();

	Vec3dSub( move, to, from );

	moveFaces( true, select ); // cleanup brush

	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
}


void MoveTool::moveFaces( bool cleanup, unsigned int select )
{
	brush_t		*b;
	face_t		*f;

	float		dist;
	vec3d_t		norm, origin;

	plane_t		pnew;

	printf("MoveTool::moveFaces\n");

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next ) {

		if ( !(b->select&select) )
			continue;
		
		for ( f = b->faces; f ; f=f->next ) {

			dist = Vec3dDotProduct( from, f->plane.norm ) - f->plane.dist;
			printf(" dist = %f\n", dist );

			if ( dist >= 0 ) {
				Vec3dCopy(norm, f->plane.norm); // norm = f->plane.norm;
				dist = f->plane.dist;
				
				Vec3dScale( origin, dist, norm );
				Vec3dAdd( origin, origin, move );
				
				dist = Vec3dDotProduct( origin, norm );
	
				Vec3dCopy(pnew.norm, norm); // pnew.norm = norm;
				pnew.dist = dist;

				if ( CheckBrushAndPlane( b, &pnew ) ) {
					Vec3dCopy(f->plane.norm, norm); // f->plane.norm = norm;
					f->plane.dist = dist;
				}
			}

//			FreePolygon( f->polygon );			
		}

		FreeBrushPolygons( b );

		// should facese with clipped polygons be kept ( for dragging )
		if ( !cleanup )
			ClipBrushFaces( b );
		else
			CleanUpBrush( b );

//		CleanUpBrush( b );
		b->select|=SELECT_UPDATE;
	}
}



/*
  ==================================================
  new EditAble style

  ==================================================
*/

/*
  ====================
  startMoveCycle

  ====================
*/
void MoveTool::startMoveCycle( EAIterator *iter, Vec3 v )
{
	printf( "MoveTool::startMoveCycle iterator %p\n", iter );

	//Vec3dCopy( from, v );
	v.get(from);

	cur_iter = iter;

	xzview_i->screen2Back();
	yview_i->screen2Back();
}

/*
  ====================
  dragMoveCycle

  ====================
*/
void MoveTool::dragMoveCycle( Vec3 v )
{
	vec3d_t		to;
	
	printf( "MoveTool::dragMoveCycle\n" );

	v.get(to);
	//Vec3dCopy( to, v );

	Vec3dSub( move, to, from );
	if ( move[0]!=0 || move[1]!=0 || move[2]!=0 )
	{
		xzview_i->back2Screen();
		yview_i->back2Screen();	

		// move
		EAL_Move( cur_iter, from, to );

		wired_i->updateViews();
		Vec3dCopy( from, to );
	}
}



/*
  ====================
  finishMoveCycle

  ====================
*/
void MoveTool::finishMoveCycle( Vec3 v )
{
	vec3d_t		to;

	printf( "MoveTool::finishMoveCycle\n" );

	//Vec3dCopy( to, v );
	v.get(to);

	Vec3dSub( move, to, from );
	
	// move
	EAL_Move( cur_iter, from, to );
	
	// destroy iterator
	delete cur_iter;
	
	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();	
}


