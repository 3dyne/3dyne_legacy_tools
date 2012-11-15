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



// ClipperTool.cc

#include "ClipperTool.hh"

ClipperTool	*clippertool_i;

/*                                                                              
  ===============================================                               
  class: ClipperTool                                                              
  ===============================================                               
*/                                                                              
 
ClipperTool::ClipperTool( QObject* parent, char* name )
	: QObject( parent, name )
{

	clippertool_i = this;
	reset();
}

ClipperTool::~ClipperTool()
{

}

void ClipperTool::reset( void )
{
	xznum = 0;
	ynum = 0;

	vnum = 0;

	clippersane = false;

	// update views...
	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
}

void ClipperTool::setSplitPlane( plane_t *pl, vec3d_t center )
{
	Vec3dCopy( clipplane.norm, pl->norm );
	clipplane.dist = pl->dist;
	
	Vec3dCopy( origin, center );

	xznum = 0;
	ynum = 0;
	vnum = 0;
	clippersane = true;
	wired_i->redrawViews();
}

bool ClipperTool::isPlaneValid( void )
{
	return clippersane;
}

void ClipperTool::getSplitPlane( plane_t *pl )
{
	memcpy( pl, &clipplane, sizeof( plane_t ) );
}

void ClipperTool::drawSelf( void )
{
	int		i;
  
	vec3d_t		to;

	XZStartDraw();
	YStartDraw();
	XZColor( colorgreen_i );
	YColor( colorgreen_i );

	for ( i = 0; i < vnum; i++ ) {
		XZDrawCross( vec[i] );	
		YDrawCross( vec[i] );
	}


	if ( clippersane ) {
		// there is a normal
		Vec3dCopy( to, clipplane.norm );
		Vec3dScale( to, 100, to );
		Vec3dAdd( to, origin, to );
		XZDrawLine( origin, to );
		YDrawLine( origin, to );
	}

	XZEndDraw();
	YEndDraw();
}

void ClipperTool::xzPressSlot( vec2d_t v )
{
	vec3d_t		v3;

	printf("ClipperTool::xzPressSlot\n");
//	wired_i->printComment( "setup clipper." );

	if ( vnum == 3 )
	{
		this->reset();
	}

	if ( vnum == 0 )
	{
		wired_i->printComment( "ClipperTool: set first point.");
	}

	v3[0] = v[0];
//	v3[1] = wired_i->getYChecker();
	v3[1] = cursor3d_i->getY();
	v3[2] = v[1];
	
	if ( vnum < 3 ) {
		Vec3dCopy( vec[vnum++], v3 );
		xznum++;
		checkClipper();
		drawSelf();
	}
	else
		wired_i->printComment( "ClipperTool: XZView-vec allready defined. ignored.");
}

void ClipperTool::yPressSlot( vec2d_t v )
{
	vec3d_t		v3;

	printf("ClipperTool::yPressSlot\n");

	if ( vnum == 3 )
	{
		this->reset();
	}

	if ( vnum == 0 )
	{
		wired_i->printComment( "ClipperTool: set first point.");
	}

	v3[0] = v[0];
	v3[1] = v[1];
//	v3[2] = wired_i->getZChecker();
	v3[2] = cursor3d_i->getZ();
	
	if ( vnum < 3 ) {
		Vec3dCopy( vec[vnum++], v3 );
		ynum++;
		checkClipper();
		drawSelf();
	}
	else
		wired_i->printComment( "ClipperTool: YView-vec allready defined. ignored.");
	
}

void ClipperTool::flip( void )
{
	if ( !clippersane ) {
		wired_i->printComment( "ClipperTool: can't flip normal.");
		return;
	}

	clipplane.dist = -clipplane.dist;
	Vec3dScale( clipplane.norm, -1, clipplane.norm );

	// update views...
	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();

	drawSelf();
}

void ClipperTool::clipBrushes( void )
{
	brush_t		*b;
	brush_t		*bnext;
       	brush_t		*front, *back;
	face_t		*fnew;

	printf("ClipperTool::splitBrushes\n");

	if ( !clippersane ) {
		wired_i->printComment("ClipperTool: can't use clipper.");
		return;
	}

	front = back = NULL;

	for ( b = wwm_i->getFirstBrush(); b ; b=bnext ) {
		bnext = b->next; // it could be removed

		if ( !(b->select&SELECT_BLUE) )
			continue;
#if 0
		CopyBrush( &b2, b );
		b2->status = -BS_NORMAL;
		b2->visible = 1;	
		wwm_i->addBrush( b2 );
#endif

		fnew = brushtool_i->createFace( clipplane.norm, clipplane.dist );
		// work on original and keep back
		if ( AddFaceToBrush( b, fnew ) ) {
			FreeBrushPolygons( b );
			CleanUpBrush( b );

			if ( !b->faces ) {
				printf("WARNING: all faces are clipped away.\n");
				wired_i->printComment("ClipperTool: the whole brush was clipped away.");

				wwm_i->removeBrush( b );
				FreeBrush( b );
			} 
			else {
				b->select|=SELECT_UPDATE;
			}
		}
		else {
			wired_i->printComment("ClipperTool: AddPlaneToBrush failed, brush not clipped.");
			FreeFace( fnew );
		}



	}

	printf("leave split.\n");

}


void ClipperTool::checkClipper( void )
{
//	vec3d_t		viewvec;
	clippersane = false;

	printf("ClipperTool::checkClipper\n");

//	if ( vnum < 2 ) {
//		wired_i->printComment( "ClipperTool: not enough points given. can't use clipper." );
//	}

	if ( vnum == 2 ) {
		if ( xznum != 2 && ynum != 2 )
			wired_i->printComment( "ClipperTool: only 2 points in different views given. can't use clipper.");
		else
			if ( xznum == 2 ) {
				wired_i->printComment( "ClipperTool: XZView-vec only clipper." );
				Vec3dCopy( origin, vec[0] );
				Vec3dInit( up, 0, 1, 0 );
			 
				Vec3dSub( right, vec[1], origin );

				Vec3dCrossProduct( clipplane.norm, right, up );
				Vec3dUnify( clipplane.norm );
				clipplane.dist = Vec3dDotProduct( origin, clipplane.norm );

				clippersane = true;
			}
			else {
				wired_i->printComment( "ClipperTool: YView-vec only clipper." );	
				
				Vec3dCopy( origin, vec[0] );
				Vec3dInit( up, 0, 0, 1 );
				
				Vec3dSub( right, vec[1], origin );
				
				Vec3dCrossProduct( clipplane.norm, right, up );
				Vec3dUnify( clipplane.norm );
				clipplane.dist = Vec3dDotProduct( origin, clipplane.norm );

				clippersane = true;
			}
	}

	if ( vnum == 3 ) {
		wired_i->printComment( "ClipperTool: 3d clipper." );
		
		origin[0] = vec[0][0];
		origin[1] = vec[0][1];
		origin[2] = vec[0][2];

		Vec3dSub( right, vec[1], origin );
		Vec3dSub( up, vec[2], origin );

		Vec3dCrossProduct( clipplane.norm, right, up );
		Vec3dUnify( clipplane.norm );
		clipplane.dist = Vec3dDotProduct( origin, clipplane.norm );

		clippersane = true;
	}	
}

void CSG_SplitBrush( brush_t *in, vec3d_t norm, float dist, brush_t **front, brush_t **back );

#if 0

void ClipperTool::splitBrushes( void )
{
	brush_t		*b, *bnew;
	brush_t		*bnext;
       	brush_t		*front, *back;
	face_t		*fnew;

	plane_t		flipclip;

	printf("ClipperTool::splitBrushes\n");

	if ( !clippersane ) {
		wired_i->printComment("ClipperTool: can't use clipper.");
		return;
	}

	front = back = NULL;

	for ( b = wwm_i->getFirstBrush(); b ; b=bnext ) {
		bnext = b->next;

		if ( !(b->select&SELECT_BLUE) )
			continue;
#if 1
		CopyBrush( &bnew, b );
		CleanUpBrush( bnew );
		bnew->id = GetUniqueNumber();
		wwm_i->addBrush( bnew, false );		
#endif

		fnew = brushtool_i->createFace( clipplane.norm, clipplane.dist );
		// work on original and keep back
		if ( AddFaceToBrush( b, fnew ) ) {
			FreeBrushPolygons( b );
			CleanUpBrush( b );

			if ( !b->faces ) {
				printf("WARNING: all faces are clipped away.\n");
				wired_i->printComment("ClipperTool: the whole brush was clipped away.");

				wwm_i->removeBrush( b );
				FreeBrush( b );
			} 
			else {
				b->select|=SELECT_UPDATE;
			}
		}
		else {
			wired_i->printComment("ClipperTool: AddPlaneToBrush failed, brush not clipped.");
			FreeFace( fnew );
		}

#if 1
		flipclip.dist = -clipplane.dist;
		Vec3dScale( flipclip.norm, -1, clipplane.norm );
		fnew = brushtool_i->createFace( flipclip.norm, flipclip.dist );

		// work on copy and keep front
		if ( AddFaceToBrush( bnew, fnew ) ) {
			FreeBrushPolygons( bnew );
			CleanUpBrush( bnew );

			if ( !bnew->faces ) {
				printf("WARNING: all faces are clipped away.\n");
				wired_i->printComment("ClipperTool: the whole brush was clipped away.");

				wwm_i->removeBrush( bnew );
				FreeBrush( bnew );
			} 
			else {
//				b2->status = -b2->status;
			}
		}
		else {
			wired_i->printComment("ClipperTool: AddPlaneToBrush failed, brush not clipped.");
			FreeFace( fnew );
		}		
#endif


	}

	printf("leave split.\n");
}

#else

void ClipperTool::splitBrushes( void )
{
	brush_t		*b;
	brush_t		*bnext;
       	brush_t		*front, *back;

	printf("ClipperTool::splitBrushes\n");

	if ( !clippersane ) {
		wired_i->printComment("ClipperTool: can't use clipper.");
		return;
	}

	front = back = NULL;

	for ( b = wwm_i->getFirstBrush(); b ; b=bnext ) {
		bnext = b->next;
		
		if ( !(b->select&SELECT_BLUE) )
			continue;

		wwm_i->removeBrush( b );

		CSG_SplitBrush( b, clipplane.norm, clipplane.dist, &front, &back );
		printf( "front: %p, back: %p\n", front, back );

		FreeBrushFaces( b );
		FreeBrush( b );

		if ( front )
		{
			wwm_i->addBrush( front );
			front->id = GetUniqueNumber();
			front->select|=SELECT_UPDATE;
		}

		if ( back )
		{
			wwm_i->addBrush( back, true );
			back->id = GetUniqueNumber();
		}
		
	}
	
	printf("leave split.\n");
}
#endif
/*
  ==================================================
  the CSG-Code is crap in due to the borken design 
  of brush.c

  ==================================================
*/

void CSG_SplitBrush( brush_t *in, vec3d_t norm, float dist, brush_t **front, brush_t **back )
{
	brush_t		*b1, *b2;
	face_t		*fnew;

	plane_t		flipclip;


//	front = back = NULL;



	CopyBrush( &b2, in );
//	b2->id = GetUniqueNumber();
	CleanUpBrush( b2 );
	
	CopyBrush( &b1, in );
//	b1->id = GetUniqueNumber();
	CleanUpBrush( b1 );

	*back = b1;
	*front = b2;

	fnew = brushtool_i->createFace( norm, dist );

	if ( AddFaceToBrush( b1, fnew ) ) {
		FreeBrushPolygons( b1 );
		CleanUpBrush( b1 );
		
		if ( !b1->faces ) {
			FreeBrush( b1 );
			*back = NULL;
		} 
	}
	else {
//		wired_i->printComment("ClipperTool: AddPlaneToBrush failed, brush not clipped.");
		FreeFace( fnew );
	}
	
#if 1
	flipclip.dist = -dist;
	Vec3dScale( flipclip.norm, -1, norm );
	fnew = brushtool_i->createFace( flipclip.norm, flipclip.dist );
	
	// work on copy and keep front
	if ( AddFaceToBrush( b2, fnew ) ) {
		FreeBrushPolygons( b2 );
		CleanUpBrush( b2 );
		
		if ( !b2->faces ) {
				FreeBrush( b2 );
				*front = NULL;
		} 
	}
	else {
		FreeFace( fnew );
	}		
#endif
}


brush_t* CSG_SubstractBrushes( brush_t *b1, brush_t *b2 )
{
	brush_t		*in, *out;
	brush_t		*front, *back;
	face_t		*f;

	in = b1;
	out = NULL;

	for ( f = b2->faces; f ; f=f->next )
	{
		if ( !in )
			break;

		CSG_SplitBrush( in, f->plane.norm, f->plane.dist, &front, &back );

//		if ( in != b1 )
//		{
			FreeBrushFaces( in );
			FreeBrush( in );
//		}
		if ( front )
		{
			front->next = out;
			out = front;
		}
		in = back;
	}

	if ( in )
	{
		FreeBrushFaces( in );
		FreeBrush( in );
	}
#if 0
	else
	{
		for ( b = out; b ; b=bnext )
		{
			bnext = b->next;
			FreeBrushFaces( b );
			FreeBrush( b );
		}
		return b1;
	}
#endif

	return out;
}

#define         PLANE_NORM_EPSILON      ( 0.00001 )                             
#define         PLANE_DIST_EPSILON      ( 0.01 )                                

bool_t	ArePlanesEqual( vec3d_t norm1, float dist1, vec3d_t norm2, float dist2 )
{
        if ( fabs( norm1[0] - norm2[0] ) < PLANE_NORM_EPSILON &&                
             fabs( norm1[1] - norm2[1] ) < PLANE_NORM_EPSILON &&                
             fabs( norm1[2] - norm2[2] ) < PLANE_NORM_EPSILON &&                
             fabs( dist1 - dist2 ) < PLANE_DIST_EPSILON )                       
                return true;                                                    
        return false; 	
}

bool_t CSG_DoBrushesIntersect( brush_t *b1, brush_t *b2 )
{
	int		i;
	vec3d_t		norm2;
	float		dist2;
	face_t		*f1, *f2;

	for ( i = 0; i < 3; i++ )
		if ( b1->min[i] > b2->max[i] ||
		     b1->max[i] < b2->min[i] )
			break;

	if ( i!=3 )
		return false;

	for ( f1 = b1->faces; f1 ; f1=f1->next )
		for ( f2 = b2->faces; f2 ; f2=f2->next )
		{
			Vec3dFlip( norm2, f2->plane.norm );
			dist2 = -f2->plane.dist;
			if ( ArePlanesEqual( f1->plane.norm, f1->plane.dist, norm2, dist2 ) )
			     return false;
		}
	return true;
}

void ClipperTool::csgBrushes( void )
{

	// b1(red) - b2(blue)
	brush_t		*bred;
	brush_t		*bblue;
	brush_t		*b, *bnext;

	brush_t		*csgout;

	bblue = NULL;
	csgout = NULL;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( (b->select&7)==SELECT_BLUE ) // ? only blue
		{
			bblue = b;
			break;
		}
	}

	if ( !bblue )
		return;

	for(;;)
	{
		printf( "### search red.\n" );
		bred = NULL;
		for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
		{
			if ( (b->select&7)==SELECT_RED ) 
			{
				bred = b;
				break;
			}
		}
		if ( !bred )
		{
			printf( "### no red.\n" );
			break;
		}

		wwm_i->removeBrush( bred );

		if ( CSG_DoBrushesIntersect( bred, bblue ) )
		{
			bred = CSG_SubstractBrushes( bred, bblue );
		}

		for ( b = bred; b ; b=bnext )
		{
			bnext=b->next;
			b->next = csgout;
			csgout = b;
		}
		
	}
	
	for ( b = csgout; b ; b=bnext )
	{
		printf( "add csg brush.\n" );
		bnext = b->next;
		b->id=GetUniqueNumber();
		wwm_i->addBrush( b );
		b->select|=(SELECT_UPDATE|SELECT_RED);
	}
//	bred->select|=SELECT_UPDATE;
}
