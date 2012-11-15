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



// CameraView.cc

#include "Wired.hh"
#include "WWM.hh"
#include "CameraView.hh"
#include "XZView.hh"
#include "YView.hh"

#include "lib_mesh.h"
#include "lib_math.h"
#include "lib_bezier.h"
#include "CSurface.hh"
#include "CtrlPoint.hh"

CameraView	*cameraview_i;

// friends of CameraView

void CameraStartDraw( void )
{
	cameraview_i->qp_draw->begin( cameraview_i );
	cameraview_i->qp_draw->setBrush( *cameraview_i->qb_drawbrush );
	cameraview_i->qp_draw->setClipRect( 0, 0, (int)cameraview_i->drawwidth, (int)cameraview_i->drawheight );	
}

void CameraEndDraw( void )
{
	cameraview_i->qp_draw->end();
	cameraview_i->qc_drawcolor = NULL;
}

void CameraColor( QColor *color ) {
	if ( cameraview_i -> qc_drawcolor != color ) {
		cameraview_i -> qc_drawcolor = color;
		cameraview_i -> qp_draw -> setPen( *color );
	}
}

void CameraDrawLine( vec3d_t from, vec3d_t to )
{
	vec3d_t		v0, v1;

	R_Vec3dRot( v0, from );
	R_Vec3dRot( v1, to );

	if ( !R_FrustumClipLine( v0, v1, 0 ) )
		return;

	R_Vec3dPer( v0, v0 );
	R_Vec3dPer( v1, v1 );

	cameraview_i->qp_draw->drawLine( (int)v0[0], (int)v0[1], (int)v1[0], (int)v1[1]);
}

void CameraDrawPolygon( vec3d_t	p[], int pointnum )
{
	int		i;
	vec3d_t		temp;
	QPointArray	qpa( pointnum );

	for ( i = 0 ; i < pointnum; i++ )
	{
		R_Vec3dRot( temp, p[i] );
		R_Vec3dPer( temp, temp );

		qpa.setPoint( i, (int)temp[0], (int)temp[1] );
	}

	cameraview_i->qp_draw->drawPolygon( qpa );

}

/*                                                                              
  ===============================================                               
  class: CameraView                                                              
  ===============================================                               
*/ 
                                                                             
CameraView::CameraView( QWidget *parent, const char *name )
	: QWidget( parent, name )
{

	cameraview_i = this;
	Vec3dInit( origin, 0,0,0 );
	Vec3dInit( lookat, 0, 64, 0 );

	qp_draw = new QPainter();
	qc_drawcolor = NULL;
	qb_drawbrush = new QBrush( *colorred_i, Dense3Pattern );

	scale = 1.0;

}

CameraView::~CameraView()
{

}

void CameraView::world2Painter( vec2d_t /*out*/, vec3d_t /*in*/ )
{
//	vec3d_t temp;
//	R_Vec3dRot
}

//
// draw
//

void CameraView::startDraw( void )
{
	qp_draw->begin( this );
	qp_draw->setBrush( *qb_drawbrush );
	qp_draw->setClipRect( 0, 0, (int)drawwidth, (int)drawheight );	
}

void CameraView::endDraw( void )
{
	qp_draw->end();
	qc_drawcolor = NULL;
}

void CameraView::setColor( QColor *color )
{
	if ( qc_drawcolor != color ) {
		qc_drawcolor = color;
		qp_draw -> setPen( *color );
	}
}

void CameraView::draw3dLine( vec3d_t from, vec3d_t to )
{
	vec3d_t		v0, v1;

	R_Vec3dRot( v0, from );
	R_Vec3dRot( v1, to );

	if ( !R_FrustumClipLine( v0, v1, 0 ) )
		return;

	R_Vec3dPer( v0, v0 );
	R_Vec3dPer( v1, v1 );

	qp_draw->drawLine( (int)v0[0], (int)v0[1], (int)v1[0], (int)v1[1]);

}

void CameraView::draw3dPolygon( vec3d_t p[], int pointnum )
{
	int		i;
	vec3d_t		temp;
	QPointArray	qpa( pointnum );

	for ( i = 0 ; i < pointnum; i++ )
	{
		R_Vec3dRot( temp, p[i] );
		R_Vec3dPer( temp, temp );

		qpa.setPoint( i, (int)temp[0], (int)temp[1] );
	}

	qp_draw->drawPolygon( qpa );
}
//

void CameraView::setCamera( vec3d_t from, vec3d_t to )
{
	vec3d_t		v;
	float		alpha, beta;
	FILE		*h;
	char		tmp[256];

	Vec3dCopy( origin, from );
	Vec3dCopy( lookat, to );
	Vec3dSub( v, lookat, origin );
	Vec3dUnify( v );
       
	R_SetOrigin( origin );

	alpha = 0;

//	alpha = atan2(fabs(v[2]),v[1]);
	alpha = asin(-v[1]);
//	beta = atan2(v[2],v[0]) - M_PI_2;
	beta = atan2(v[2],v[0]); // - M_PI_2;

	alpha = alpha*(180/M_PI);
/*
	if ( alpha < 0 )
		alpha = 360 + alpha;
	alpha = alpha - 90;
*/
	beta = beta*(180/M_PI);
	if ( beta < 0 )
		beta = 360 + beta;
	beta = beta - 90;
	
	printf(" beta = %f, alpha = %f\n", beta, alpha );
	R_CalcMatrix( alpha * M_PI/180, beta * M_PI/180, 0 );
//	R_CalcMatrix( 0, 0, 0 );

	drawSelf();

	sprintf( tmp, "%s/camera", wired_i->getProjectDir() );
	h = fopen( tmp, "w" );
	if ( !h )
		return;
	fprintf( h, "# camera file.\n" );
	fprintf( h, "# generate by: wired\n" );
	fprintf( h, "# <origin x> <origin y> <origin z> <roll> <pitch> <yaw>\n" );
	fprintf( h, "%f %f %f %f %f %f\n", origin[0], origin[1], origin[2], alpha, beta, 0.0 );
	fclose( h );
		
}

void CameraView::setZoom( float zoom )
{
	scale = zoom;
	R_SetZoom( scale );
}

void CameraView::drawSelf( void )
{

	printf("CameraView::drawSelf\n");

	qp_draw->begin( this );
	qp_draw->eraseRect( 0, 0, (int)drawwidth, (int)drawheight );

	qp_draw->end();
}


void CameraView::render( void )
{
//	int		i;
	QPixmap		qp((int)drawwidth,(int)drawheight);
	QImage		qi;
	void	*ptr;

	qi = qp.convertToImage();

	if ( qi.depth() != 32 )
	{
		printf( "convert depth\n" );
		qi = qi.convertDepth( 32 );
	}

	printf(" width = %d, height = %d, depth = %d\n", qi.width(), qi.width(), qi.depth() );

	ptr = (void*) qi.bits();

	R_SetFrameBuffer( ptr, qi.depth() );
	R_InitFrame();
	R_RenderTextureFrame( wwm_i->getFirstBrush() );
	this->renderCSurfaces();
	this->renderCPolys();
	R_DumpStat();

	unsigned char *img = qi.bits();
	for ( int i = 0; i < qi.width() * qi.height() * 4; i+=4 ) {
		unsigned char r = img[i+0];
		unsigned char g = img[i+1];
		unsigned char b = img[i+2];
		img[i+0] = b;
		img[i+1] = g;
		img[i+2] = r;
	}
	
	qp.convertFromImage( qi );
	bitBlt( this, 0, 0, &qp, 0, 0, (int)drawwidth, (int)drawheight );

}

#define V5INIT( _v, _0, _1, _2, _3, _4 ) { \
	_v[0] = _0; \
	_v[1] = _1; \
	_v[2] = _2; \
	_v[3] = _3; \
	_v[4] = _4; \
}
	

void CameraView::renderCSurfaces( void )
{
	face_t		*faces;
	int		u, v;
	
	faces = NULL;

	// for each CSurface
	//	generate pos/texel
	//		generate faces

	surface_ctrl_t		*texeval;	// texel eval 
	surface_points_t	*texmesh;	// texel mesh

	// this polygon is rendered
	polygon_t *p = NewPolygon( 4 );
	p->pointnum = 4;
	

	CSurfaceIterator	iter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_VISIBLE ) );
	CSurface		*cs;

	for ( iter.reset(); ( cs = iter.getNext() ); )
	{
		cstexdef_t	td;
		cs->getTexdef( &td );
		R_SetTexture( td.ident );

		// pos mesh
		uvmesh_t *pmesh = cs->generateUVMesh( 10, 10 );
		
		// texel mesh

		texeval = NewBezierSurface( 2, 2 );
#if 0		
		vec3d_t		tp;
		cs->getTexelCtrlPoint( 0, 0, tp );
		SetSurfaceCtrlPoint3f( texeval, 0, 0, tp[0], tp[1], 0 );
		cs->getTexelCtrlPoint( 1, 0, tp );
		SetSurfaceCtrlPoint3f( texeval, 1, 0, tp[0], tp[1], 0 );
		cs->getTexelCtrlPoint( 1, 1, tp );
		SetSurfaceCtrlPoint3f( texeval, 1, 1, tp[0], tp[1], 0 );
		cs->getTexelCtrlPoint( 0, 1, tp );
		SetSurfaceCtrlPoint3f( texeval, 0, 1, tp[0], tp[1], 0 );

#else
		vec2d_t		tmp;
		SetSurfaceCtrlPoint3f( texeval, 0, 0, td.shift[0], td.shift[1], 0 );
		tmp[0] = td.shift[0] + td.vecs[0][0]*td.scale[0];
		tmp[1] = td.shift[1] + td.vecs[0][1]*td.scale[1];
		SetSurfaceCtrlPoint3f( texeval, 1, 0, tmp[0], tmp[1], 0 );
		tmp[0] = td.shift[0] + td.vecs[1][0]*td.scale[0];
		tmp[1] = td.shift[1] + td.vecs[1][1]*td.scale[1];
		SetSurfaceCtrlPoint3f( texeval, 1, 1, tmp[0], tmp[1], 0 );
		tmp[0] = td.shift[0] + td.vecs[2][0]*td.scale[0];
		tmp[1] = td.shift[1] + td.vecs[2][1]*td.scale[1];
		SetSurfaceCtrlPoint3f( texeval, 0, 1, tmp[0], tmp[1], 0 );
#endif
		texmesh = EvalSurfacePoints( texeval, 10, 10 );

		
		// build faces
		for ( u = 0; u < 10 - 1; u++ )
			for ( v = 0; v < 10 - 1; v++ )
			{
				vec3d_t		p1, p2, p3, p4;
				vec3d_t		t1, t2, t3, t4;
				
				GetUVMeshPoint( pmesh, u, v, p1 );
				GetUVMeshPoint( pmesh, u+1, v, p2 );
				GetUVMeshPoint( pmesh, u+1, v+1, p3 );
				GetUVMeshPoint( pmesh, u, v+1, p4 );

				GetSurfacePoint( texmesh, u, v, t1 );
				GetSurfacePoint( texmesh, u+1, v, t2 );
				GetSurfacePoint( texmesh, u+1, v+1, t3 );
				GetSurfacePoint( texmesh, u, v+1, t4 );
				
				// fixme: why is it flipped ?
				V5INIT( p->p[3], p1[0], p1[1], p1[2], t1[0], t1[1] );
				V5INIT( p->p[2], p2[0], p2[1], p2[2], t2[0], t2[1] );
				V5INIT( p->p[1], p3[0], p3[1], p3[2], t3[0], t3[1] );
				V5INIT( p->p[0], p4[0], p4[1], p4[2], t4[0], t4[1] );
				
				R_RenderPolygon( p );
			}

		FreeUVMesh( pmesh );
		FreeSurfacePoints( texmesh );
		FreeBezierSurface( texeval );
	}
	FreePolygon( p );
}

#define PROJECT_X	( 0 )
#define PROJECT_Y	( 1 )
#define PROJECT_Z	( 2 )

static int TypeOfProjection( vec3d_t norm )
{
	int		i;
	vec3d_t		an;

	if ( norm[0] == 1.0 || norm[0] == -1.0 )
		return PROJECT_X;
	if ( norm[1] == 1.0 || norm[1] == -1.0 )
		return PROJECT_Y;
	if ( norm[2] == 1.0 || norm[2] == -1.0 )
		return PROJECT_Z;

	for ( i = 0; i < 3; i++ )
		an[i] = fabs( norm[i] );

	if ( an[0] >= an[1] && an[0] >= an[2] )
		return PROJECT_X;
	else if ( an[1] >= an[0] && an[1] >= an[2] )
		return PROJECT_Y;
	else if ( an[2] >= an[0] && an[2] >= an[1] )
		return PROJECT_Z;
	
	return -1;
}

void GenerateVec5d( vec5d_t out, vec3d_t in, int flag, cptexdef_t *td )
{
	out[0] = in[0];
	out[1] = in[1];
	out[2] = in[2];

	//
	// project to 2d
	//

	if ( flag == PROJECT_X )
	{
		out[3] = in[2]; // Y
		out[4] = in[1]; // Z
	}
	else if ( flag == PROJECT_Y )
	{
		out[3] = in[0]; // X
		out[4] = in[2]; // Z
	}
	else if ( flag == PROJECT_Z )
	{
		out[3] = in[0]; // X
		out[4] = in[1]; // Y
	}

	//
	// calc rotation
	//
	vec2d_t	vec[2];
	if ( td->rotate != 0.0 )
	{
		float angle = td->rotate / 180*M_PI;
		float s = sin( angle );
		float c = cos( angle );

		vec2d_t axis[2] = { {1, 0}, {0, 1} };
		
		for ( int i = 0; i < 2; i++ )
		{
			vec[i][0] = c * axis[i][0] - s * axis[i][1];
			vec[i][1] = s * axis[i][0] + c * axis[i][1];
		}
		vec2d_t	tmp;
		tmp[0] = out[3]*vec[0][0] + out[4]*vec[0][1];
		tmp[1] = out[3]*vec[1][0] + out[4]*vec[1][1];
		

		out[3] = tmp[0];
		out[4] = tmp[1];		
	}
	
	
	out[3] /= td->scale[0];
	out[4] /= td->scale[1];	

	out[3] -= td->shift[0];
	out[4] -= td->shift[1];

//	out[4] = -out[4];
}

void CameraView::renderCPolys( void )
{
	CPolyIterator		iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_VISIBLE ) );
	CPoly			*cpoly;

	polygon_t *p = NewPolygon( 3 );
	p->pointnum = 3;

	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		cptexdef_t	td;
		cpoly->getTexdef( &td );
		R_SetTexture( td.ident );

		vec3d_t		norm;
		float		dist;
		int		type;
		vec3d_t		center;

		cpoly->getPlane( norm, &dist );
		type = TypeOfProjection( norm );
		
		cpoly->getCenter( center );

		int		edgenum;
		edgenum = cpoly->getEdgeNum();

		for ( int i = 0; i < edgenum; i++ )
		{
			uvmesh_t *curve = cpoly->generateCurveOfEdge( i, 10 );

			for ( int u = 0; u < 10 - 1; u++ )
			{
				vec3d_t		p1, p2;
				
				GetUVMeshPoint( curve, u, 0, p1 );
				GetUVMeshPoint( curve, u+1, 0, p2 );

				GenerateVec5d( p->p[0], p1, type, &td );
				GenerateVec5d( p->p[1], p2, type, &td );
				GenerateVec5d( p->p[2], center, type, &td );

				R_RenderPolygon( p );
			}
			FreeUVMesh( curve );
		}
	}
	FreePolygon( p );
}

void CameraView::render_csg( void )
{
//	int		i;
	QPixmap		qp((int)drawwidth,(int)drawheight);
	QImage		qi;
	void	*ptr;

	qi = qp.convertToImage();


	printf(" width = %d, height = %d, depth = %d\n", qi.width(), qi.width(), qi.depth() );

	ptr = (void*) qi.bits();

	R_SetFrameBuffer( ptr, qi.depth() );
	R_InitFrame();
	R_RenderDebugCSGFaces( "csgfaces" );
	R_DumpStat();

	qp.convertFromImage( qi );
	bitBlt( this, 0, 0, &qp, 0, 0, (int)drawwidth, (int)drawheight );

}

void CameraView::resizeEvent( QResizeEvent * )
{
	printf("CameraView::resizeEvent\n");

	drawwidth = width();
	drawheight = height();

	R_SetView( drawwidth, drawheight );

	drawSelf();
}

void CameraView::mousePressEvent( QMouseEvent *e )
{
	int		s;
	int		button;

	vec3d_t		to;
	vec3d_t		from;
//	vec3d_t		v, v2;
	vec3d_t		ray;

	printf("CameraView::mousePressEvent\n");

	button = e->button();

	to[0] = (float) e->x() - drawwidth/2.0;
	to[1] = (float) e->y() - drawheight/2.0;
	to[2] = 0.0;
	
	from[0] = 0.0;
	from[1] = 0.0;
	from[2] = -drawwidth/2.0;
	
	R_Vec3dInverseRot( to, to );
	R_Vec3dInverseRot( from, from );
//	Vec3dPrint( v );
//	Vec3dPrint( origin );
	
	Vec3dSub( ray, to, from );
	Vec3dUnify( ray );

	Vec3dCopy( from, to );
//	Vec3dScale( to, 8192.0, ray );
//	Vec3dAdd( to, from, to );
	
/*
  XZStartDraw();
  XZDrawCross( to );
  XZDrawCross( from );
  XZEndDraw();
*/

	s = e->state() | e->button();

	emit cameraRaySignal( 0, s, from, ray );

#if 0
	
	bestbrush = NULL;
	bestface = NULL;
	bestl = 999999;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next ) {
		
		if ( !b->visible )
			continue;
		
		Vec3dCopy( s0, from );
		Vec3dCopy( s1, to );
		
		hit = ClipRayByBrush( b, s0, s1 );
		
		if ( hit ) {

			Vec3dSub( ray, s0, from );
			l = Vec3dLen( ray );
			printf(" l = %f\n", l );
			if ( !bestface ) {
				bestface = hit;
				bestbrush = b;
				bestl = l;
			}
			else {
				// is the last hitface better than the bestface ???
//				if ( Vec3dDotProduct( from, hit->plane.norm ) - hit->plane.dist <
//				     Vec3dDotProduct( from, bestface->plane.norm ) - bestface->plane.dist )

				if ( l < bestl ) {
					bestface = hit;
					bestbrush = b;
					bestl = l;
				}
			}
		}		
	}
	
	if ( bestface ) {
		printf(" hit face %p.\n", bestface );
		
		// polygon clipped away ?
		if ( bestface->polygon ) {
			
			CameraStartDraw();
			CameraColor( coloryellow_i );
			
			for ( i = 0; i < bestface->polygon->pointnum; i++ ) {
				CameraDrawLine( bestface->polygon->p[i], bestface->polygon->p[(i+1)%bestface->polygon->pointnum] );
			}
			CameraEndDraw();
		}

		if ( button == LeftButton ) {
			wired_i->printComment( "copy currenttexdef to clicked face.");
			texdef_i->getTexDef( &bestface->texdef );

			// the polygons have to be clipped new, for the new texdef !!
			FreeBrushPolygons( bestbrush );
			CleanUpBrush( bestbrush );
		}
		if ( button == RightButton ) 
		{
			wired_i->printComment( "copy texdef of clicked face to currenttexdef.");
			texdef_i->setTexDef( &bestface->texdef );
			texdef_i->drawSelf();
		}

		if ( button == MidButton ) {
			printf("FaceDump:\n");
			for ( i = 0; i < bestface->polygon->pointnum; i++ ) {
				printf(" p %d : x = %f, y = %f, z = %f, tx = %f, tx(int) = %d, ty = %f\n", i,
				       bestface->polygon->p[i][0],
				       bestface->polygon->p[i][1],
				       bestface->polygon->p[i][2],
				       bestface->polygon->p[i][3],
				       (int)bestface->polygon->p[i][3],
				       bestface->polygon->p[i][4] );
			}
		}
	}	
//	emit cameraRaySignal( from, to );	

#endif
}
