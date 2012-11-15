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



// Wired.cc

#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <qapplication.h>
#include <qfiledialog.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qmessagebox.h>
#include "b_file.xpm"
#include "b_save.xpm"
#include "i_open.xpm"
#include "i_save.xpm"
#include "i_exit.xpm"
#include "i_zoom.xpm"
#include "i_grid.xpm"
#include "MTemplates.hh"


#include "archetype.h"
#include "Wired.hh"
#include "vec.h"
#include "cdb_service.h"
#include "brush.h"
#include "texture.h"

//#include "lib_poly.h"
#include "vec.h"
#include "draw_portal.h"

#include "BrushEdit.hh"
#include "lib_mesh.h"		// for CSurface

#include "lib_bezier.h"
#include "CSurface.hh"

#include "ATEdit.hh"
#include "TexRes.hh"
// 
// global instances
//

Wired		*wired_i;		// wired main widget/app
BrushSettingWdg	*brushsetting_wdg_i;
BrushTool	*brushtool_i;
Cursor3d	*cursor3d_i;
ClipBoard	*clipboard_i;
Draw3d		*draw3d_i;
TexRes		*texres_i;

int main( int argc, char *argv[] )
{
//	char cwd[256];
//	getcwd( cwd, 255 );
//	printf("cwd: %s\n", cwd);

	CDB_StartUp( 0 );

	QApplication	wiredapp( argc, argv );
	QFont		wiredfont( "Helvetica", 12, QFont::Bold );
	QColorGroup	wiredcg( QColor( "white" ), QColor( "grey60" ), QColor( "grey70" ), QColor( "grey50" ), QColor( "grey60" ), QColor( "white" ), QColor( "grey60" ));

	Wired		*wired;

//	wiredapp.setPalette( QPalette( wiredcg, wiredcg, wiredcg ));
	wiredapp.setFont( wiredfont, true );
	wired = new Wired();

	wiredapp.setMainWidget( wired );

	wired->show();
	if( argc >= 2 )
	{
		wired->loadWire( ( const char* )argv[1] );
	}
	return wiredapp.exec();
}


/*
  =============================================================================
  class: Cursor3d
  =============================================================================
*/



Cursor3d::Cursor3d()
{
	Vec3dInit( origin, 0 ,0 ,0 );
	Vec3dInit( to, 0, 0, 1 );
}

Cursor3d::~Cursor3d()
{

}

void Cursor3d::setX( float x )
{
	origin[0] = x;
}

void Cursor3d::setY( float y )
{
	origin[1] = y;
}

void Cursor3d::setZ( float z )
{
	origin[2] = z;
}


void Cursor3d::setXTo( float x )
{
	to[0] = x;
}

void Cursor3d::setYTo( float y )
{
	to[1] = y;
}

void Cursor3d::setZTo( float z )
{
	to[2] = z;
}

float Cursor3d::getX( void )
{
	return origin[0];
}

float Cursor3d::getY( void )
{
	return origin[1];
}

float Cursor3d::getZ( void )
{
	return origin[2];
}

void Cursor3d::get( vec3d_t v )
{
	Vec3dCopy( v, origin );
}

void Cursor3d::getTo( vec3d_t v )
{
	Vec3dCopy( v, to );
}

void Cursor3d::getDelta( vec3d_t v )
{
	Vec3dSub( v, to, origin );
}

void Cursor3d::getDir( vec3d_t v )
{
	Vec3dSub( v, to, origin );
	Vec3dUnify( v );
}

void Cursor3d::drawSelf( void )
{
	xzview_i->startDraw();
	yview_i->startDraw();
	cameraview_i->startDraw();
	
//	QPen	style = QPen( *colorred_i, 0, DashLine )
//	xzview_i->setPenStyle( style, 0, DashLine ) );
	xzview_i->setColor( colorred_i );       
	yview_i->setColor( colorred_i );
	cameraview_i->setColor( colorred_i );

	xzview_i->draw3dCross( origin );
	yview_i->draw3dCross( origin );

	xzview_i->draw3dLine( origin, to );
	yview_i->draw3dLine( origin, to );
	
	xzview_i->endDraw();
	yview_i->endDraw();
	cameraview_i->endDraw();
}

/*
  ==================================================
  class: ClipBoard

  hold copies
  and make copies
  ==================================================
*/
ClipBoard::ClipBoard()
{
	currentbrush = NULL;
}

ClipBoard::~ClipBoard()
{
	if ( currentbrush )
	{
		FreeBrushFaces( currentbrush );
		FreeBrush( currentbrush );
		currentbrush = NULL;
	}		
}


void ClipBoard::copyBrush( brush_t *b )
{
	//
	// delete old brush
	//
	if ( currentbrush )
	{
		FreeBrushFaces( currentbrush );
		FreeBrush( currentbrush );
		currentbrush = NULL;
	}

	//
	// make copy of brush
	//
	CopyBrush( &currentbrush, b );
	CleanUpBrush( currentbrush );	
}

brush_t* ClipBoard::pasteBrush( void )
{
	brush_t		*bnew;

	if ( !currentbrush )
		return NULL;

	CopyBrush( &bnew, currentbrush );
	CleanUpBrush( bnew );
	bnew->id = GetUniqueNumber();

	return bnew;
}

/*                                                                              
  ===============================================                               
  class: Wired                                                            
  ===============================================                               
*/                                                                              



Wired::Wired( QWidget* parent, char* name )
	: QWidget( parent, name )
{
	const char*	zoomnames[] = {
		"4/1",
		"2/1",
		"1/1",
		"1/2",
		"1/4",
		"1/8",
		NULL
	};

	const char*	gridnames[] = {
		"none",
		"2",	
		"4",
		"8",
		"16",
		"32",
		"64",
		"128",
		"256",
		"1024",
		NULL
	};

	const char*	rotstepnames[] = {
		"1, 360/360",
		"5, 360/72",
		"10, 360/36",
		"30, 360/12",
		"45, 360/8",
		"90, 360/4",
		"5.625, 360/64",
		"11.25, 360/32",
		"22.5, 360/16",
		"26.6, atan 0.5",
		NULL
	};

	qpb_file = 0;
	qpb_save = 0;

	qpm_filepopup = 0;
	qcb_zoom = 0;
	qlb_zoom = 0;
	qcb_grid = 0;
	qlb_grid = 0;
	qcb_rotstep = 0;
	qlb_comment = 0;
	accel = 0;

	qc_brushblack = 0;	// the colors are mode dependent / just names for diffrent selections
	qc_brushblue = 0;
	qc_brushred = 0;
	qc_brushgreen = 0;
	qc_brushliquid = 0; // shifted
	qc_brushlocal = 0;	// shifted
	qc_brushdeco = 0; // shifted
	qc_arche = 0;
	qt_paintthread = 0;
	ptbrush = 0;

	wwm = 0;
	xzview = 0;
	yview = 0;
	cameraview = 0;


	texbrowser = 0;
	movetool = 0;

	clippertool = 0;
	atbrowser = 0;

	qsp_editviews = 0;

	wireloaddir = 0;
	wiresavedir = 0;
	qt_autosave = 0;

	
	vec2d_t		min, max, origin;
	printf("wired constructor.\n");

	panratio = 0.5;

	ychecker = 0.0;
	zchecker = 0.0;

	xzswapflag = false;

	// texdef
	strcpy( currenttexdef.ident, "default" );
	currenttexdef.rotate = 0.0;
	currenttexdef.scale[0] = 1.0;
	currenttexdef.scale[1] = 1.0;
	currenttexdef.shift[0] = 0.0;
	currenttexdef.shift[1] = 0.0;

	Vec3dInit( cameraorigin, 0, 0, 0 );
	Vec3dInit( cameralookat, 0, 64, 0 );
	Vec3dInit( origin, 0, 0, 0 );

	buttonstate = 0;

	//
	// init global instances
	//

	InitCustomize();

	wired_i = this;

	const char *path = CDB_GetString( "project/base" );
	if ( !path ) {
		Error("can't get 'project/base' from cdb\n");
	}
	texres_i = new TexRes( path );

	
	cursor3d_i = new Cursor3d();
	clipboard_i = new ClipBoard();

	brushsetting_wdg_i = new BrushSettingWdg();
	brushsetting_wdg_i->show();
	brushsetting_wdg_i->setBrushContents( BRUSH_CONTENTS_SOLID );
//	brushsetting_wdg_i->setSurfaceContents( SURFACE_CONTENTS_CLOSE|SURFACE_CONTENTS_TEXTURE);
	connect( brushsetting_wdg_i, SIGNAL( CSTexDefChanged( void ) ), this, SLOT( CSTexDefChangedSlot( void ) ) );
	connect( brushsetting_wdg_i, SIGNAL( TexDefChanged( void ) ), this, SLOT( TexDefChangedSlot( void ) ) );


	brushtool_i = new BrushTool();


	// paintthread
	qt_paintthread = new QTimer( this );
	qt_paintthread->stop();
	connect( qt_paintthread, SIGNAL( timeout() ), this, SLOT( paintThreadSlot() ) );

	// GUI
	editmode = EM_BRUSH;
	submode_brush = EM_BRUSH_NORMAL;

#if 1
	accel = new QAccel( this );

	accel->insertItem( Key_E );	// set submode_brush to EM_BRUSH_EXTRUDE
	accel->insertItem( Key_E+SHIFT ); // set submode_brush to EM_BRUSH_FACESCALE

	accel->insertItem( Key_P+CTRL );	// test: pump brush
	
	accel->insertItem( Key_T );	// archetype: draw to-links
	accel->insertItem( Key_M );	// Mesh mode, CSurface
	accel->insertItem( Key_M+SHIFT ); // Mesh mode, CPoly 

	accel->insertItem( Key_B+SHIFT ); // set blue and red to BrushContentsWdg
	accel->insertItem( Key_F+SHIFT ); // set blue and red to SurfaceContentsWdg
	accel->insertItem( Key_F+SHIFT+CTRL ); // hack: set all surfaces of solid brushes to closed+textured. This is for easy fixing degenerated solid brushes of early levels 
	
	accel->insertItem( Key_W ); // 'where' for find brush via unique id     

	accel->insertItem( Key_S ); // CSG: Substract red - blue

	accel->insertItem( Key_Z ); // rotate z +d
	accel->insertItem( Key_Z+SHIFT ); // rotate z -d

	accel->insertItem( Key_Y ); // rotate y +d
	accel->insertItem( Key_Y+SHIFT ); // rotate y -d

	accel->insertItem( Key_X ); // rotate x +d
	accel->insertItem( Key_X+SHIFT ); // rotate x -d

	accel->insertItem( Key_F1 ); // render cameraview 
	accel->insertItem( Key_F2 ); // update all views and wire-cameraview
	accel->insertItem( Key_F3 ); // process brushes
	accel->insertItem( Key_F4 ); // draw portals
//	accel->insertItem( Key_F5 ); // render debug-csg-faces
	

	accel->insertItem( Key_V ); // set Camera viewport

	accel->insertItem( Key_O ); // copy cursor3d origin to currentpair
	accel->insertItem( Key_P ); // copy cursor3d delta to currentpair

	accel->insertItem( Key_A ); // archetype edit
	accel->insertItem( Key_B ); // set editmode to EM_BRUSH
	accel->insertItem( Key_C ); // set submode_brush to EM_BRUSH_CLIPPER ( reset clipper-tool )
	accel->insertItem( Key_C+SHIFT ); // set submode_brush to EM_BRUSH_CLIPPER ( don't reset clipper-tool )

	accel->insertItem( Key_Space ); // tool-dependend action key
	accel->insertItem( Key_Space+SHIFT ); // second tool-dependend action key
	accel->insertItem( Key_Escape ); // tool-dependend reset

	accel->insertItem( Key_G ); // set status of singlebrush to green
	accel->insertItem( Key_R ); // set status of singlebrush to red

	accel->insertItem( Key_U ); // set status of singlebrush to normal ( _U_nselect )
	accel->insertItem( Key_U+SHIFT ); // set status of all brushes to normal

	accel->insertItem( Key_L ); // delete singlebrush
	accel->insertItem( Key_L+SHIFT ); // delete singlebrush and red-brushes

	accel->insertItem( Key_K ); // copy singlebrush
	accel->insertItem( Key_K+SHIFT ); // copy singlebrush and red-brushes

	accel->insertItem( Key_K+CTRL ); // copy single brush into clipboard
	accel->insertItem( Key_I );	// insert single brush from clipboard

	accel->insertItem( Key_F ); // clipper-mode: flip normal, archetype: draw from-links
	accel->insertItem( Key_F+CTRL ); // set cpoly plane to clippertool plane
	accel->insertItem( Key_G+CTRL ); // set clippertool plane to cpoly plane

	// revison
	accel->insertItem( Key_N ); // new object
	accel->insertItem( Key_N+SHIFT ); // archetype: get unique name key

	accel->insertItem( Key_F10+CTRL ); // brush-mode: all brushes/faces get new ID
	accel->insertItem( Key_F9+CTRL ); // move world by cursor3d


	connect( accel, SIGNAL( activated( int ) ), this, SLOT( guiKeySlot( int ) ) );
#endif

	qpb_file = new QPushButton( "File", this );      
//	qpb_file->setStyle( WindowsStyle );
	qpb_file->setPixmap( QPixmap(( const char** ) b_file ) );
	connect( qpb_file, SIGNAL( clicked() ), this, SLOT( guiFileSlot() ) );
	
	qpm_filepopup = new QPopupMenu( NULL );
	qpm_filepopup->insertItem( "New ...", this, SLOT( guiFileNewSlot()));
	qpm_filepopup->insertSeparator();
	qpm_filepopup->insertItem( QPixmap(( const char** ) i_open), "Open ...", this, SLOT( guiFileOpenSlot() ));
	accel->connectItem( accel->insertItem( CTRL+Key_O), this, SLOT( guiFileOpenSlot() ));
	qpm_filepopup->insertSeparator();
	qpm_filepopup->insertItem( QPixmap(( const char** ) i_save), "Save", this, SLOT( guiFileSaveSlot()) );
	qpm_filepopup->insertItem( "Save as ...", this, SLOT( guiFileSaveAsSlot() ));
	qpm_filepopup->insertSeparator();
	qpm_filepopup->insertItem( QPixmap(( const char** )i_exit), "Quit", this, SLOT( guiFileQuitSlot() ), CTRL + Key_Q );
	//qpm_filepopup->setStyle( WindowsStyle );
	qpm_filepopup->hide();

	qpb_save = new QPushButton( "Save", this );      
//	qpb_save->setStyle( WindowsStyle );
	qpb_save->setPixmap( QPixmap(( const char** ) b_save ) );
	connect( qpb_save, SIGNAL( clicked() ), this, SLOT( guiFileSaveSlot())); 


	// zoom
	qcb_zoom = new QComboBox( this );
	qcb_zoom->insertStrList( zoomnames ); 
	//qcb_zoom->setStyle( WindowsStyle );
	connect( qcb_zoom, SIGNAL( activated( int ) ), this, SLOT( guiZoomChangedSlot( int ) ) );
	qlb_zoom = new QLabel( this );
	qlb_zoom->setPixmap( ( const char** )i_zoom );

	// grid
	qcb_grid = new QComboBox( this );
	qcb_grid->insertStrList( gridnames ); 
	//qcb_zoom->setStyle( WindowsStyle );
	connect( qcb_grid, SIGNAL( activated( int ) ), this, SLOT( guiGridChangedSlot( int ) ) );

	qlb_grid = new QLabel( this );
	qlb_grid->setPixmap( ( const char** )i_grid );


	// rotstep
	qcb_rotstep = new QComboBox( this );
	qcb_rotstep->insertStrList( rotstepnames );
	qcb_rotstep->setCurrentItem( 8 ); // set to 360/16


	qlb_comment = new QLabel( this );
	qlb_comment->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	qlb_comment->setLineWidth( 1 );
	qlb_comment->setText( "Hallo" );

//	resize( 640, 480 );
	
 	T_InitTexCache();
//	arr_t	*dummy;
//	dummy = T_GetTextureByName( "metal/metal51_1" );
	//dummy = T_GetTextureByName( "e1u1/c_met11_2" );
	//dummy = T_GetTextureByName( "e1u1/c_met51a" );
//	T_DumpStat();

	
	wwm = new WWM();
	atbrowser = new ATBrowser( this );

//	texdef = new TexDef( this );
	
	Vec2dInit( min, -16000, -16000 );
	Vec2dInit( max,  16000,  16000 );
	Vec2dInit( origin, 0, 0 );

	//
	// spawn qsplitter between xzview and yview

	qsp_editviews = new QSplitter( this );
	qsp_editviews->setOrientation( QSplitter::Vertical );
	qsp_editviews->setOpaqueResize( FALSE ); 

	//
	// spawn editviews 

	xzview = new XZView( qsp_editviews );
//	xzview->setBackgroundColor( QColor( "white" ));
//	xzview->setForegroundColor( QColor( "black" ));
	xzview->setOrigin( origin );
	xzview->setZoom( 1, 100, 10, 5 );
	xzview->setSuperBounds( min, max );	
	xzview->show();

	yview = new YView( qsp_editviews );
	yview->setOrigin( origin );
	yview->setZoom( 1, 100, 10, 5 );
	yview->setSuperBounds( min, max );	
	yview->show();

	cameraview = new CameraView( this );
	cameraview->setZoom( 4.0 );
	cameraview->show();

	connect( xzview, SIGNAL( originChangedSignal() ), this, SLOT( xzviewOriginChangedSlot() ) );
	connect( xzview, SIGNAL( pannerChangedSignal( int ) ), this, SLOT( xzviewPannerChangedSlot( int ) ) );
	connect( yview, SIGNAL( originChangedSignal() ), this, SLOT( yviewOriginChangedSlot() ) );

	draw3d_i = new Draw3d();

//	selecttool = new SelectTool( this );

	movetool = new MoveTool( this );

	clippertool = new ClipperTool( this );

	connect( xzview, SIGNAL( xzMouseEventSignal( int, int, Vec2, Vec2 ) ), this, SLOT( xzMouseEventSlot( int, int, Vec2, Vec2 ) ) );
	connect( yview, SIGNAL( yMouseEventSignal( int, int, Vec2, Vec2 ) ), this, SLOT( yMouseEventSlot( int, int, Vec2, Vec2 ) ) );

	connect( cameraview, SIGNAL( cameraRaySignal( int, int, Vec3, Vec3 ) ), this, SLOT( cameraRaySlot( int, int, Vec3, Vec3 ) ) );

	texbrowser = new TextureBrowser( this );
	texbrowser -> hide();
	
	connect( texbrowser, SIGNAL( acceptSignal( char * ) ), this, SLOT( texBrwAcceptSlot( char * ) ) );

	setMinimumSize( 640, 480 );
//	resize( 640, 480 );
	qt_autosave = new QTimer();

	connect( qt_autosave, SIGNAL( timeout()), this, SLOT( autoSaveSlot()));
	
	loadConfig();
	memset( wirename, 0, 256 );
	//loadWire( "./i100.wire" );

	this->changeEditMode( EM_BRUSH );
	//processview->callProcess( ( unsigned char * )"process", ( unsigned char * )"dm1.sbrush" );
	memset( &g_project, 0, sizeof( w_project_t ));


//	ATEdit		*atedit;
//	atedit = new ATEdit();

//	atedit->show();
}

Wired::~Wired()
{
	printf("wired destructor.\n");
}


void Wired::enableAccel( void )
{
	accel->setEnabled( TRUE );
}

void Wired::disableAccel( void )
{
	accel->setEnabled( FALSE );
}

void Wired::changeEditMode( int mode )
{

	// set the colors for diffrent editmodes

	if ( mode == EM_BRUSH )
	{
		editmode = EM_BRUSH;
		qc_brushblack = colorblack_i;
		qc_brushblue = colorblue_i;
		qc_brushred = colorred_i;
		qc_brushgreen = colorgreen_i;
		qc_brushliquid = colordblue_i;
		qc_brushlocal = colordgreen_i;
		qc_brushdeco = colorviolet_i;

		atbrowser->hide();
		texbrowser->show();
	}
//	else if ( mode == EM_CLIPPER )
//	{
//		editmode = EM_CLIPPER;
//	}
	else if ( mode == EM_ARCHETYPE )
	{
		editmode = EM_ARCHETYPE;
		qc_brushblack = colorgray30_i;
		qc_brushblue = colorgray30_i;
		qc_brushred = colorgray30_i;
		qc_brushgreen = colorgray30_i;	
		qc_brushliquid = colorgray30_i;
		qc_brushlocal = colorgray30_i;
		qc_brushdeco = colorgray30_i;

		texbrowser->hide();
		atbrowser->show();
	}
	else if ( mode == EM_TESTBOX )
	{
		editmode = EM_TESTBOX;		
	}
	else if ( mode == EM_CSURFACE )
	{
		editmode = EM_CSURFACE;
	}
	else if ( mode == EM_CPOLY )
	{
		editmode = EM_CPOLY;
	}

	redrawViews();
}

void Wired::drawSelf( void )
{
	vec3d_t		v;

	// checker
	v[0] = 0;
	v[1] = ychecker;
	v[2] = 0;

	yview_i->startDraw();
	yview_i->setColor( coloryellow_i );
	yview_i->draw3dHLine( v );       
	yview_i->endDraw();

	v[0] = 0;
	v[1] = 0;
	v[2] = zchecker;

	xzview_i->startDraw();
	xzview_i->setColor( coloryellow_i );
	xzview_i->draw3dHLine( v );       
	xzview_i->endDraw();

	// camera
	xzview_i->startDraw();
	yview_i->startDraw();
	xzview_i->setColor( colorviolet_i );
	yview_i->setColor( colorviolet_i );
	xzview_i->draw3dLine( cameraorigin, cameralookat );
	yview_i->draw3dLine( cameraorigin, cameralookat );
	xzview_i->draw3dCross( cameraorigin );
	yview_i->draw3dCross( cameraorigin );
	xzview_i->endDraw();
	yview_i->endDraw();

	cursor3d_i->drawSelf();	
}

void Wired::updateViews( void )
{
	stat_updatebrushnum = 0;

	updatestate = UPDATE_START;
//	extrapass = false; // draw selected brushes
//	ptbrush = wwm->getFirstBrush();
	paintThreadSlot();
	paintArcheTypeThreadSlot();

	// fix me:
	// test TestBox, CSurface

	xzview_i->startDraw();
	yview_i->startDraw();
	cameraview_i->startDraw();

	TestBox		*list;
	list = wwm_i->getFirstTestBox();
	for ( ; list ; list=list->getNext() )
		this->drawTestBox( (TestBox*)list );

	xzview_i->endDraw();
	yview_i->endDraw();
	cameraview_i->endDraw();

	wwm_i->checkConsistency();

	//
	// use new Draw3d
	//

	draw3d_i->startDraw( VIEW_XZ | VIEW_Y | VIEW_CAMERA );

	// CSurface
	CSurfaceIterator	csiter( wwm_i->getFirstCSurface() );
	CSurface		*cs;
	for ( csiter.reset(); (cs = csiter.getNext()); )
		this->drawCSurface( cs );

	// CPoly
	CPolyIterator		cpi( wwm_i->getFirstCPoly() );
	CPoly			*cp;
	for ( cpi.reset(); ( cp = cpi.getNext() ); )
		this->drawCPoly( cp );

	draw3d_i->endDraw();

	wwm_i->checkConsistency();
}

void Wired::redrawViews( void )
{
	xzview_i->drawSelf();
	yview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	updateViews();
	drawSelf();	
}

void Wired::layoutChanged( void )
{

	int	rowl, rowr;
//	int	col0, col1;

	rowl = 80;
	rowr = width()-304;


	qpb_file->setGeometry( 0, 0, 80, 25 );
	qpb_save->setGeometry( 0, 25, 80, 25 );

	qcb_zoom->setGeometry( 20, 75, 60, 25 );
	qlb_zoom->setGeometry( 2, 75, 20-2, 25 );
	qcb_grid->setGeometry( 20, 100, 60, 25 );
	qlb_grid->setGeometry( 2, 100, 20-2, 25 );

	qcb_rotstep->setGeometry( 0, 125, 80, 25 );

//	xzview->setGeometry( rowl, 0, rowr-rowl, (int)(height()*panratio) );
//	yview->setGeometry( rowl, (int)(height()*panratio), rowr-rowl, (int)(height()-height()*panratio-16.0) );
	qsp_editviews->setGeometry( rowl, 0, rowr - rowl, height()-16 );

	cameraview->setGeometry( rowr, height()/2, width()-rowr, height()/2-16 );

	atbrowser->setGeometry( rowr, 0, width()-rowr, height()/2 );
	texbrowser->setGeometry( rowr, 0, width()-rowr, height()/2 - 75 );	
//	texdef->setGeometry( rowr, height()/2 - 75, width()-rowr, 75 );

	qlb_comment->setGeometry( rowl, height()-16, width()-rowl, 16 );

	redrawViews();
}


void Wired::zoomChanged( void )
{
	const float	zoom[] = { 4.0, 2.0, 1.0, 0.5, 0.25, 0.125 };
	
	xzview->setZoom( zoom[zoomindex], 100, 10, 5 );
	yview->setZoom( zoom[zoomindex], 100, 10, 5 );

	setViewBounds();
	updateViews();
}

void Wired::gridChanged( void )
{
	const int	grid[] = { 0, 2, 4, 8, 16, 32, 64, 128, 256, 1024 };
	
	xzview->setGrid( grid[gridindex] );
	yview->setGrid( grid[gridindex] );

	wwm_i->allUpdateFlagsTrue();
	updateViews();
}


void Wired::setViewBounds( void )
{
	vec2d_t		min, max;

	xzview->getViewBounds( min, max );
	viewbounds[0][0] = min[0];
	viewbounds[0][2] = min[1];
	viewbounds[1][0] = max[0];
	viewbounds[1][2] = max[1];
		
	yview->getViewBounds( min, max );
	viewbounds[0][1] = min[1];
	viewbounds[1][1] = max[1];

	wwm_i->viewBoundsChanged( viewbounds[0], viewbounds[1] );
}


void Wired::loadConfig()
{
	char*	string;
	int	posx, posy, width, height;
	int	saveint;


//	CDB_StartUp( 0 );

	wiresavedir = new QDir( "./", "*.wire" );
	wireloaddir = new QDir( "./", "*.wire" );
	// file dialogs
	if( CDB_GetIntValue( "wired/conf" ) )
	{
	
	
		string = CDB_GetString( "wired/wire/load_dir" );
		if( string != NULL )
			wireloaddir->setPath( string );

	
		string = CDB_GetString( "wired/wire/save_dir" );
		if( string != NULL )
			wiresavedir->setPath( string );
		
		saveint = CDB_GetIntValue( "wired/wire/as_intervall" );
		if( !saveint )
		{
			printf( "no auto save\n" );
		} else
		{
			qt_autosave->start( saveint * 1000, FALSE );
			printf( "setting as intervall to %d sec\n", saveint );
		}
	
		posx = CDB_GetIntValue( "wired/main/xpos" );
		posy = CDB_GetIntValue( "wired/main/ypos" );
		width = CDB_GetIntValue( "wired/main/width" );
		height = CDB_GetIntValue( "wired/main/height" );
		setGeometry( posx, posy, width, height );

		string = CDB_GetString( "wired/main/panratio" );
		if( string != NULL )
			panratio = atof( string );
	} else
	{
		WOSMessage( "no Wired configuration in CDB.\nusing defaults." );
	}
}
	

void Wired::saveConfig()
{
	char	panstring[256];
	CDB_ChangeStringEntry( "wired/wire/load_dir", (char*) wireloaddir->path().latin1() );
	CDB_ChangeStringEntry( "wired/wire/save_dir", (char*) wiresavedir->path().latin1() );
	CDB_ChangeIntEntry( "wired/main/xpos", x() );
	CDB_ChangeIntEntry( "wired/main/ypos", y() );
	CDB_ChangeIntEntry( "wired/main/width", width() );
	CDB_ChangeIntEntry( "wired/main/height", height() );
	sprintf( panstring, "%f", panratio );
	CDB_ChangeStringEntry( "wired/main/panratio", panstring );
	CDB_ChangeIntEntry( "wired/conf", 1 );
	CDB_Save();
}	


float Wired::getYChecker( void )
{
	return ychecker;
}

float Wired::getZChecker( void )
{
	return zchecker;
}

void Wired::getCameraOrigin( vec3d_t v )
{
	Vec3dCopy( v, cameraorigin );
}

void Wired::getCameraLookAt( vec3d_t v )
{
	Vec3dCopy( v, cameralookat );
}

void Wired::printComment( const char *text )
{
	qlb_comment->setText( text );
}

//void Wired::getCurrentTexDef( texturedef_t *copyto )
//{
//	memcpy( copyto, &currenttexdef, sizeof( texturedef_t ) );
//}


/*
  ==================================================
  actions

  ==================================================
*/

void Wired::doSwapWorld( void )
{
	brush_t		*b;
	face_t		*f;
	float		temp, temp2;
	vec2d_t		vtemp;
	

	//
	// swap brushes
	//
//		ProcessBrushes( wwm_i->getFirstBrush() );
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next ) {
		for ( f = b->faces; f ; f=f->next ) {
			if ( !xzswapflag ) {
				temp = f->plane.norm[2];
				f->plane.norm[2] = f->plane.norm[0];
				f->plane.norm[0] = -temp;
			}
			else {
				temp = f->plane.norm[0];
				f->plane.norm[0] = f->plane.norm[2];
				f->plane.norm[2] = -temp;	
			}
//				f->plane.dist = -f->plane.dist;
		}
		FreeBrushPolygons( b );
		CleanUpBrush( b );
	}
	


	//
	// swap CSurface
	//
	CSurfaceIterator		iter( wwm_i->getFirstCSurface() );
	CSurface		*cs;

	for ( iter.reset(); ( cs = iter.getNext() ); )
	{
		CtrlPointIterator	cpi( cs->getFirstCtrlPoint() );
		CtrlPoint		*cp;
		for ( cpi.reset(); ( cp = cpi.getNext() ); )
		{
			if ( !xzswapflag ) 
			{
				vec3d_t		tmp;
				int		u, v;
				cp->getPos( tmp, &u, &v );
				temp = tmp[2];
				tmp[2] = tmp[0];
				tmp[0] = -temp;
				cp->setPos( tmp, u, v );
			}
			else
			{
				vec3d_t		tmp;
				int		u, v;
				cp->getPos( tmp, &u, &v );
				temp = tmp[0];
				tmp[0] = tmp[2];
				tmp[2] = -temp;
				cp->setPos( tmp, u, v );				
			}		       
		}		
		cs->calcBB();
	}


	// 
	// swap CPoly
	//
	CPolyIterator		iter2( wwm_i->getFirstCPoly() );
	CPoly		*cpoly;
	
	for ( iter2.reset(); ( cpoly = iter2.getNext() ); )
	{
		CtrlPointIterator	cpi( cpoly->getFirstCtrlPoint() );
		CtrlPoint		*cp;
		for ( cpi.reset(); ( cp = cpi.getNext() ); )
		{
			if ( !xzswapflag )
			{
				vec3d_t		tmp;
				int		u, v;
				cp->getPos( tmp, &u, &v );
				temp = tmp[2];
				tmp[2] = tmp[0];
				tmp[0] = -temp;
				cp->setPos( tmp, u, v );
			}
			else
			{
				vec3d_t		tmp;
				int		u, v;
				cp->getPos( tmp, &u, &v );
				temp = tmp[0];
				tmp[0] = tmp[2];
				tmp[2] = -temp;
				cp->setPos( tmp, u, v );				
			}
		}

		// swap plane
		if ( !xzswapflag )
		{
			vec3d_t		norm;
			float		dist;
			cpoly->getPlane( norm, &dist );
			temp = norm[2];
			norm[2] = norm[0];
			norm[0] = -temp;
			cpoly->setPlane( norm, dist );
		}
		else {
			vec3d_t		norm;
			float		dist;
			cpoly->getPlane( norm, &dist );			
			temp = norm[0];
			norm[0] = norm[2];
			norm[2] = -temp;	
			cpoly->setPlane( norm, dist );
		}

		cpoly->calcBB();
	}

	//
	// swap utils
	//
	if ( !xzswapflag ) {			
		xzswapflag = true;
		xzview_i->getOrigin( vtemp );
		temp = vtemp[1];
		vtemp[1] = vtemp[0];
		temp2 = vtemp[0] = -temp;
		xzview_i->setOrigin( vtemp );
		
		yview_i->getOrigin( vtemp );
		vtemp[0] = temp2;
		yview_i->setOrigin( vtemp );
	}
	else {
		xzswapflag = false;
		xzview_i->getOrigin( vtemp );
		temp = vtemp[0];
		temp2 = vtemp[0] = vtemp[1];
		vtemp[1] = -temp;
		xzview_i->setOrigin( vtemp );
		
		yview_i->getOrigin( vtemp );
		vtemp[0] = temp2;
		yview_i->setOrigin( vtemp );			
	}
//		xzview_i->drawSelf();
//		yview_i->drawSelf();
	cameraview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
	
}

/*
  ==============================
  doMoveWorld

  ==============================
*/
void Wired::doMoveWorld( vec3d_t dir )
{
	vec3d_t		from = {0, 0, 0 };
	
	printf( "doMoveWorld\n" );

	// move all brushes
	brush_t		*b;
	face_t		*f;
	vec3d_t		norm;
	fp_t		dist;
	vec3d_t		origin;
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		for ( f = b->faces; f ; f=f->next ) {
			Vec3dCopy( norm, f->plane.norm); //norm = f->plane.norm;
			dist = f->plane.dist;
			
			Vec3dScale( origin, dist, norm );
			Vec3dAdd( origin, origin, dir );
			
			dist = Vec3dDotProduct( origin, norm );

			Vec3dCopy( f->plane.norm, norm ); // f->plane.norm = norm;
			f->plane.dist = dist;
			
			// free old polygons
			FreePolygon( f->polygon );
		}
		ClipBrushFaces( b );
	}

	// move archetypes
	arche_t		*a;
	vec3d_t		v;
	kvpair_t	*pair;	
	
	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		pair = AT_GetPair( a, "origin" );
		if ( !pair )
		{
			printf( "ATBrowser::moveArches can't find key \"origin\"\n" );
			return;
		}	
		AT_CastValueToVec3d( v, pair->value );
		
		Vec3dAdd( v, dir, v );

		AT_CastVec3dToValue( pair->value, v );
	}	

	// move curved surfaces
	CSurfaceIterator		*csurf_iter;
	csurf_iter = new CSurfaceIterator( wwm_i->getFirstCSurface() );
	EAL_Move( csurf_iter, from, dir );

	// move curved poly
	CPolyIterator		*cpoly_iter;
	cpoly_iter = new CPolyIterator( wwm_i->getFirstCPoly() );
	EAL_Move( cpoly_iter, from, dir );

	cameraview_i->drawSelf();
	wwm_i->allUpdateFlagsTrue();
	wired_i->updateViews();
	wired_i->drawSelf();
	
}

void Wired::doSelectBrush( vec3d_t start, vec3d_t dir )
{
	brush_t		*b;

	wwm_i->selectByRay( start, dir );

	for ( b = wwm_i->getFirstBrush(); b ;b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			brushsetting_wdg_i->setBrushContents( b->contents );
			break;
		}
	}

	wired_i->updateViews();	
}


void Wired::doSetBrushSettingWdg( vec3d_t start, vec3d_t dir )
{
	int		i;

	printf( "Copy brush/face settings to brushsetting_wdg.\n" );
	// copy ray face to current texdef
	brush_t *b;
	face_t *f;
	wwm_i->findBestBrushForRay( start, dir, &b, &f );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found.\n" );
	}
	else
	{
		if ( f->polygon ) {
			
		CameraStartDraw();
		CameraColor( colorred_i );
		
		for ( i = 0; i < f->polygon->pointnum; i++ ) {
			CameraDrawLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
		}
		CameraEndDraw();
	}	


		brushsetting_wdg_i->setBrushContents( b->contents );
		brushsetting_wdg_i->setSurfaceContents( f->contents );
		brushsetting_wdg_i->setTexDef( &f->texdef );
	}	
}


void Wired::doGetBrushSettingWdg( vec3d_t start, vec3d_t dir )
{
	int		i;

	printf( "Copy brushsetting_wdg to face.\n" );
	brush_t *b;
	face_t *f;
	wwm_i->findBestBrushForRay( start, dir, &b, &f );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found.\n" );
	}
	else
	{
		if ( f->polygon ) {
			
			CameraStartDraw();
			CameraColor( coloryellow_i );
			
			for ( i = 0; i < f->polygon->pointnum; i++ ) {
				CameraDrawLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
			}
			CameraEndDraw();
		}	

//		b->contents = brushsetting_wdg_i->getBrushContents( );
		f->contents = brushsetting_wdg_i->getSurfaceContents( );
		brushsetting_wdg_i->getTexDef( &f->texdef );

		// the polygons have to be clipped new, for the new texdef !!
		FreeBrushPolygons( b );
		CleanUpBrush( b );
	}		
}

void Wired::doSetSplitPlaneFromFace( vec3d_t start, vec3d_t dir )
{
	int		i;
	brush_t		*b;
	face_t		*f;
	vec3d_t		center;

	printf( "Set split plane of clipper tool to plane of clicked face.\n" );

	// search face only in selected brush
	wwm_i->findBestBrushForRay( start, dir, &b, &f, true );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found.\n" );
		return;
	}

	if ( !f->polygon )
		return;

	PolygonCenter( f->polygon, center );
	clippertool_i->setSplitPlane( &f->plane, center );
	
	CameraStartDraw();
	CameraColor( coloryellow_i );
	
	for ( i = 0; i < f->polygon->pointnum; i++ ) {
			CameraDrawLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
	}
	CameraEndDraw();	
}

void Wired::doExtrudeBrushFromFace( vec3d_t start, vec3d_t dir )
{
	int		i;
	brush_t		*b;
	face_t		*f;
	brush_t		*bnew;
	face_t		*fnew;
	
	int		pointnum;
	polygon_t	*ground, *flip;
	polygon_t	*top;

	printf( "Wired::doExtrudeBrushFromFace\n" );

	// search face only in selected brush
	wwm_i->findBestBrushForRay( start, dir, &b, &f, true );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found.\n" );
		return;
	}

	if ( !f->polygon )
		return;

	bnew = NewBrush();
	bnew->id = wwm_i->getID();
	bnew->contents = b->contents;

	pointnum = f->polygon->pointnum;
	ground = NewPolygon( pointnum );
	ground->pointnum = pointnum;
	for ( i = 0; i < pointnum; i++ )
	{
		Vec3dCopy( ground->p[i], f->polygon->p[i] );
	}


	top = NewPolygon( pointnum );
	top->pointnum = pointnum;
	for ( i = 0; i < pointnum; i++ )
	{
		Vec3dMA( top->p[i], 64.0, f->plane.norm, f->polygon->p[i] );
	}

	for ( i = 0; i < pointnum; i++ )
	{
		fnew = NewFace();
		memcpy( fnew, f, sizeof( face_t ) );
		fnew->id = wwm_i->getID();

		fnew->polygon = NewPolygon( 4 );
		fnew->polygon->pointnum = 4;

		Vec3dCopy( fnew->polygon->p[0], top->p[(i+1)%pointnum] );
		Vec3dCopy( fnew->polygon->p[1], top->p[i] );
		Vec3dCopy( fnew->polygon->p[2], ground->p[i] );
		Vec3dCopy( fnew->polygon->p[3], ground->p[(i+1)%pointnum] );
		
		fnew->next = bnew->faces;
		bnew->faces = fnew;
	}

	// flip ground
	flip = NewPolygon( pointnum );
	flip->pointnum = pointnum;
	for ( i = 0; i < pointnum; i++ )
	{
		Vec3dCopy( flip->p[-1+pointnum-i], ground->p[i] );
	}
	FreePolygon( ground );

	// insert top and flip to brush
	fnew = NewFace();
	memcpy( fnew, f, sizeof( face_t ) );
	fnew->id = wwm_i->getID();
	fnew->polygon = flip;
	fnew->next = bnew->faces;
	bnew->faces = fnew;

	fnew = NewFace();
	memcpy( fnew, f, sizeof( face_t ) );
	fnew->id = wwm_i->getID();
	fnew->polygon = top;
	fnew->next = bnew->faces;
	bnew->faces = fnew;

	// finish faces
	for ( f = bnew->faces; f ; f=f->next )
	{
		PlaneFromPolygon( f->polygon, f->plane.norm, &f->plane.dist );
		FreePolygon( f->polygon );
	}
	
	CleanUpBrush( bnew );
	wwm_i->addBrush( bnew, true );
	updateViews();			
}

void Wired::doScaleFace( vec3d_t start, vec3d_t dir, float scale )
{
	int		i, j, k;
	brush_t		*b;
	face_t		*f;
	
	vec3d_t		center;
	face_t		*f2;

	printf( "Wired::doScaleFace\n" );

	// search face only in selected brush
	wwm_i->findBestBrushForRay( start, dir, &b, &f, true );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found.\n" );
		return;
	}

	if ( !f->polygon )
		return;
	
	PolygonCenter( f->polygon, center );

	for ( f2 = b->faces; f2; f2=f2->next )
	{
		if ( f == f2 )
			continue;

		for ( i = 0; i < f->polygon->pointnum; i++ )
		{
			for ( j = 0; j < f2->polygon->pointnum; j++ )
			{
				for ( k = 0; k < 3; k++ )
				{
					if ( fabs(f->polygon->p[i][k]-f2->polygon->p[j][k]) > 0.5 )
						break;
				}
				if ( k != 3 )
					continue;

				{
					vec3d_t		vec;
					Vec3dSub( vec, f->polygon->p[i], center );
					Vec3dUnify( vec );
					Vec3dScale( vec, scale, vec );
					Vec3dAdd( f2->polygon->p[j], f2->polygon->p[j], vec );
//					Vec3dPrint( vec );
				}
			}
		}
		PlaneFromPolygon( f2->polygon, f2->plane.norm, &f2->plane.dist );
		FreePolygon( f2->polygon );
	}

	CleanUpBrush( b );
	redrawViews();		
}



void Wired::doRotateSplitPlane( float roll, float pitch, float yaw )
{
	matrix3_t	rotmat;
	vec3d_t		center;
	plane_t		pl;
	float		d1, d2;
	vec3d_t		v;

	cursor3d_i->get( center );

	Matrix3SetupRotate( rotmat, roll, pitch, yaw );

	clippertool_i->getSplitPlane( &pl );
	d1 = Vec3dDotProduct( center, pl.norm ) - pl.dist;
	
	Matrix3Vec3dRotate( pl.norm, pl.norm, rotmat );

	d2 = Vec3dDotProduct( center, pl.norm );

	pl.dist = -d1 + d2;

	Vec3dScale( v, -d1, pl.norm );
	Vec3dAdd( v, center, v );
	clippertool_i->setSplitPlane( &pl, v );
}

void Wired::doCreateBrush( void )
{
	brush_t		*bnew;
	vec3d_t		v;

	bnew = brushtool_i->createBrush();
	cursor3d_i->get( v );
	brushtool_i->moveBrush( bnew, v );

	CleanUpBrush( bnew );	
	wwm_i->addBrush( bnew, true );
	updateViews();	
}

void Wired::doCopyBrush( void )
{
	face_t		*f;
	brush_t		*b;
	brush_t		*bnew;
	
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			CopyBrush( &bnew, b );
			CleanUpBrush( bnew );
			bnew->id = wwm_i->getID();
			for ( f = bnew->faces; f ; f=f->next )
			{
				f->id = wwm_i->getID();
			}
			wwm_i->addBrush( bnew, true );	// select the copy brush
			updateViews();
			break;
		}
	}
}

void Wired::doDeleteBrush( void )
{
	brush_t		*b, *bnext;

	for ( b = wwm_i->getFirstBrush(); b ; b=bnext )
	{
		bnext=b->next;
		if ( b->select&SELECT_BLUE )
		{
			wwm_i->removeBrush( b );
			FreeBrushFaces( b );
			FreeBrush( b );
			break;
		}
	}
	redrawViews();
}

void Wired::doDeleteArche( void )
{
	arche_t		*a, *anext;
		
	for ( a = wwm_i->getFirstArche(); a ; a=anext )
	{
		anext = a->next;
		if ( a->select&SELECT_BLUE )
		{
			wwm_i->removeArche( a );
 			AT_FreePairList( a->pairs );
			AT_FreeArche( a );
			break;
		}
	}
	redrawViews();		
}

void Wired::doCopyBrushToClipboard( void )
{
	brush_t		*b;
	
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( !(b->select&SELECT_BLUE) )
			continue;

		clipboard_i->copyBrush( b );
	}
	wired_i->printComment( "Copy brush to clipboard." );
}

void Wired::doInsertBrushFromClipboard( void )
{
	brush_t		*b;

	b = clipboard_i->pasteBrush();

	if ( !b )
	{
		wired_i->printComment( "No brush in clipboard." );
		return;
	}
      
	wwm_i->addBrush( b, true );	// select the copy brush
	updateViews();
}

void Wired::doRotateBrush( float roll, float pitch, float yaw )
{
	matrix3_t	rotmat;
	vec3d_t		center;
	brush_t		*b;
	face_t		*f;
	polygon_t	*poly;
	vec3d_t	v;
	int		i;

	cursor3d_i->get( center );

	Matrix3SetupRotate( rotmat, roll, pitch, yaw );
	
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( !(b->select&SELECT_BLUE) )
			continue;

		for ( f = b->faces; f ; f=f->next )
		{
			if ( !f->polygon )
				continue;

			poly = f->polygon;

			for ( i = 0; i < poly->pointnum; i++ )
			{
				Vec3dSub( v, poly->p[i], center );
				Matrix3Vec3dRotate( v, v, rotmat );
				Vec3dAdd( poly->p[i], v, center );
			}
			
			PlaneFromPolygon( poly, f->plane.norm, &f->plane.dist );
			FreePolygon( poly );
		}
		CleanUpBrush( b );
	}

	redrawViews();
}

void Wired::doOverrideBrushContents( void )
{
	unsigned int	contents;
	brush_t		*b;

	contents = brushsetting_wdg_i->getBrushContents();

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&(SELECT_BLUE|SELECT_RED) )
		{
			b->contents = contents;
			b->select|=SELECT_UPDATE;
		}
	}
	redrawViews();
}

void Wired::doOverrideSurfaceContents( void )
{
	unsigned int	contents;
	brush_t		*b;
	face_t		*f;

	contents = brushsetting_wdg_i->getSurfaceContents();

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&(SELECT_BLUE|SELECT_RED) )
		{
			for ( f = b->faces; f ; f=f->next )
				f->contents = contents;
		}
		b->select|=SELECT_UPDATE;	
	}
	redrawViews();
}

void Wired::doAddToRed( void )
{
	brush_t		*b;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			b->select&=~(SELECT_NORMAL|SELECT_GREEN|SELECT_BLUE);
			b->select|=(SELECT_RED|SELECT_UPDATE);
			updateViews();
			break;
		}
	}
}	

void Wired::doAddToGreen( void )
{
	brush_t		*b;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			b->select&=~(SELECT_NORMAL|SELECT_RED|SELECT_BLUE);
			b->select|=(SELECT_GREEN|SELECT_UPDATE);
			updateViews();
			break;
		}
	}
}

void Wired::doAddToNormal( void )
{
	brush_t		*b;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			b->select&=~(SELECT_GREEN|SELECT_RED|SELECT_BLUE);
			b->select|=(SELECT_NORMAL|SELECT_UPDATE);
			updateViews();
			break;
		}
	}
}

void Wired::doAllAddToNormal( void )
{
	brush_t		*b;

	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&(SELECT_BLUE|SELECT_RED|SELECT_GREEN) )
		{
			b->select&=~(SELECT_GREEN|SELECT_RED|SELECT_BLUE);
			b->select|=(SELECT_NORMAL|SELECT_UPDATE);
		}
	}	
	updateViews();
}

void Wired::doApplyTexture( void )
{
	brush_t		*b;
	face_t		*f;
	texdef_t	td;

	brushsetting_wdg_i->getTexDef( &td );
	for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
	{
		if ( b->select&SELECT_BLUE )
		{
			for ( f = b->faces; f ; f=f->next )
				strcpy( f->texdef.ident, td.ident );		
			break;
		}
	}
}


void Wired::doSelectArche( vec3d_t start, vec3d_t dir )
{
	arche_t		*a;

	wwm_i->selectArcheByRay( start, dir );
	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( a->select&SELECT_BLUE )
		{
			atbrowser_i->setCurrent( a );
			break;
		}
	}

	wired_i->updateViews();		
}

void Wired::doCreateArche( void )
{
	arche_t		*anew;
	vec3d_t		v;
	char		tt[256];

	cursor3d_i->get( v );

	anew = atbrowser_i->createArche( v );

	//
	// get unique name
	//
	sprintf( tt, "#%u", wwm_i->getID() );
	AT_SetPair( anew, "STRING", "name", tt );

	atbrowser_i->setCurrent( anew );

	wwm_i->addArche( anew, true );
	updateViews();
}

void Wired::doCopyArche( void )
{
	arche_t		*anew;	
	arche_t		*a;
	
	char		tt[256];

	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( a->select&SELECT_BLUE )
		{
			anew = AT_NewArcheFromArche( a );
			
			//
			// get unique name
			//
			sprintf( tt, "#%u", wwm_i->getID() );
			AT_SetPair( anew, "STRING", "name", tt );
			
			wwm_i->addArche( anew, true );
			atbrowser_i->setCurrent( anew );
			break;
		}
	}
	updateViews();
}

void Wired::doGetUniqueNameForArche( void )
{
	arche_t		*a;
	
	char		tt[256];

	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( a->select&SELECT_BLUE )
		{
			//
			// get unique name
			//
			sprintf( tt, "#%u", wwm_i->getID() );
			AT_SetPair( a, "STRING", "name", tt );
			
			atbrowser_i->setCurrent( a );
			break;
		}
	}
	updateViews();	
}

void Wired::doCreateCSurfaceFromFace( vec3d_t start, vec3d_t dir )
{
	brush_t		*b;
	face_t		*f;
	vec3d_t		p;

	uvmesh_t		*m22;
	uvmesh_t		*m33;
	int		u, v;

	printf( "Create CSurface from clicked face.\n" );

	// search face only in selected brush
	wwm_i->findBestBrushForRay( start, dir, &b, &f, true );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found." );
		return;
	}

	if ( !f->polygon )
		return;

	//
	// only 4 point polygons can create a mesh
	//
	if ( f->polygon->pointnum != 4 )
	{
		wired_i->printComment( "The selected face ain't got four points." );
		return;
	}

	m22 = NewUVMesh( 2, 2 );
	SetUVMeshPoint( m22, 0, 0, f->polygon->p[0] );
	SetUVMeshPoint( m22, 1, 0, f->polygon->p[1] );
	SetUVMeshPoint( m22, 1, 1, f->polygon->p[2] );
	SetUVMeshPoint( m22, 0, 1, f->polygon->p[3] );
	
	m33 = Subdivied2U2VMeshTo3U3V( m22 );
	// shift mid-point for a blob
	GetUVMeshPoint( m33, 1, 1, p );
//	Vec3dMA( p, 64.0, f->plane.norm, p );
	SetUVMeshPoint( m33, 1, 1, p );

	// build CSurface
	CSurface	*cs = new CSurface( 3, 3 );
	CtrlPoint	*cp;
	for ( u = 0; u < 3; u++ )
		for( v = 0; v < 3; v++ )
		{
			GetUVMeshPoint( m33, u, v, p );
			cp = new CtrlPoint( p );
			cp->setPos( p, u, v );
			cs->insertCtrlPoint( cp );
		}

	// init texelctrlpoints
#if 1
	cs->setTexelCtrlPoint( 0, 0, &f->polygon->p[0][3] ); // uaarghh !!
	cs->setTexelCtrlPoint( 1, 0, &f->polygon->p[1][3] );
	cs->setTexelCtrlPoint( 1, 1, &f->polygon->p[2][3] );
	cs->setTexelCtrlPoint( 0, 1, &f->polygon->p[3][3] );
//#else
	cstexdef_t		td;
	Vec2dCopy( td.shift, &f->polygon->p[0][3] );
	Vec2dSub( td.vecs[0], &f->polygon->p[1][3], td.shift );
	Vec2dSub( td.vecs[1], &f->polygon->p[2][3], td.shift );
	Vec2dSub( td.vecs[2], &f->polygon->p[3][3], td.shift );

	Vec2dRint( td.shift, td.shift );
	Vec2dRint( td.vecs[0], td.vecs[0] );
	Vec2dRint( td.vecs[1], td.vecs[1] );
	Vec2dRint( td.vecs[2], td.vecs[2] );

	Vec2dPrint( td.shift );
	Vec2dPrint( td.vecs[0] );
	Vec2dPrint( td.vecs[1] );
	Vec2dPrint( td.vecs[2] );

	td.scale[0] = 1.0;
	td.scale[1] = 1.0;

	strcpy( td.ident, f->texdef.ident );	
	
	brushsetting_wdg_i->setCSTexDef( &td );
	cs->setTexdef( &td );
#endif

	wwm_i->insertCSurface( cs, true );	
	updateViews();

	FreeUVMesh( m22 );
	FreeUVMesh( m33 );
}



/*
  ====================
  doSelectCSurfaceCtrlPoint

  ====================
*/
void Wired::doSelectCSurfaceCtrlPoint( vec3d_t start, vec3d_t dir )
{
	CSurfaceIterator	csiter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface		*cs;

	for ( csiter.reset(); (cs = csiter.getNext()); )
	{
		CtrlPointIterator	cpiter( cs->getFirstCtrlPoint() );
		EAL_BlueRaySelector( &cpiter, start, dir );
	}

	updateViews();
}

/*
  ====================
  doSelectCSurface

  ====================
*/
void Wired::doSelectCSurface( vec3d_t start, vec3d_t dir )
{
	CSurfaceIterator	iter( wwm_i->getFirstCSurface() );

	EAL_BlueRaySelector( &iter, start, dir );
	updateViews();

	doApplyCSurfaceToTexdefWdg();
}




/*
  ====================
  doApplyTextureToCSurface

  ====================
*/
void Wired::doApplyTextureToCSurface( void )
{
	cstexdef_t	td;
	CSurfaceIterator	iter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface	*cs;
	
	brushsetting_wdg_i->getCSTexDef( &td );
	
	for ( iter.reset(); (cs = iter.getNext()); )
	{
		cstexdef_t	td2;
		cs->getTexdef( &td2 );
		strcpy( td2.ident, td.ident );
		cs->setTexdef( &td2 );
	}	
}



/*
  ====================
  doApplyTexdefWdgToCSurface

  ====================
*/
void Wired::doApplyTexdefWdgToCSurface( void )
{
	cstexdef_t	td;
	CSurfaceIterator	iter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface	*cs;
	
	brushsetting_wdg_i->getCSTexDef( &td );
	

	for ( iter.reset(); (cs = iter.getNext()); )
	{
//		cstexdef_t	td2;
//		cs->getTexdef( &td2 );
//		strcpy( td2.ident, td.ident );
		cs->setTexdef( &td );
	}	
}



/*
  ====================
  doApplyCSurfaceToTexdefWdg

  ====================
*/
void Wired::doApplyCSurfaceToTexdefWdg( void )
{
	cstexdef_t	td;
	CSurfaceIterator	iter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface	*cs;

	iter.reset();
	cs = iter.getNext();

	if ( !cs )
	{
		wired_i->printComment( "No CSurface selected." );
		return;
	}

	cs->getTexdef( &td );
	brushsetting_wdg_i->setCSTexDef( &td );
}



/*
  ====================
  doRotateCSurface

  ====================
*/
void Wired::doRotateCSurface( float roll, float pitch, float yaw )
{
	vec3d_t		center;
	matrix3_t	rotmat;

	cursor3d_i->get( center );

	Matrix3SetupRotate( rotmat, roll, pitch, yaw );

	CSurfaceIterator	csi( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface		*cs;

	for ( csi.reset(); ( cs = csi.getNext() ); )
	{
		CtrlPointIterator	cpi( cs->getFirstCtrlPoint() );
		CtrlPoint		*cp;
		
		for ( cpi.reset(); ( cp = cpi.getNext() ); )
		{
			vec3d_t		tmp;
			int		u, v;
			
			cp->getPos( tmp, &u, &v );
			Vec3dSub( tmp, tmp, center );
			Matrix3Vec3dRotate( tmp, tmp, rotmat );
			Vec3dAdd( tmp, tmp, center );
			cp->setPos( tmp, u, v );
		}
		
		cs->calcBB();
	}
	
	redrawViews();
}



/*
  ====================
  doCopyCSurface

  ====================
*/
void Wired::doCopyCSurface( void )
{
	CSurfaceIterator	csi( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface		*cs;
	CSurface		*csnew;

	// get blue
	csi.reset();
	cs = csi.getNext();

	if ( !cs )
	{
		wired_i->printComment( "No CSurface selected." );
		return;
	}

	csnew = cs->copySelf();
	wwm_i->insertCSurface( csnew, true );

	updateViews();
}



/*
  ====================
  doDeleteCSurface

  ====================
*/
void Wired::doDeleteCSurface( void )
{
	CSurfaceIterator	csi( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
	CSurface		*cs;

	// get blue
	csi.reset();
	cs = csi.getNext();

	if ( !cs )
	{
		wired_i->printComment( "No CSurface selected." );
		return;
	}

	wwm_i->removeCSurface( cs );
	delete cs;

	redrawViews();
}



/*
  ==================================================
  CPoly stuff

  ==================================================
*/

/*
  ====================
  doCreateCPolyFromFace

  ====================
*/
void Wired::doCreateCPolyFromFace( vec3d_t start, vec3d_t dir )
{
	int		i;
	brush_t		*b;
	face_t		*f;
//	vec3d_t		p;
	
//	uvmesh_t		*m22;
//	uvmesh_t		*m33;
	vec3d_t		center;


	printf( "Create CPoly from clicked face.\n" );

	// search face only in selected brush
	wwm_i->findBestBrushForRay( start, dir, &b, &f, true );
	if ( !b || !f )
	{
		wired_i->printComment( "No brush and face found." );
		return;
	}

	if ( !f->polygon )
		return;
	

	CPoly		*cpoly = new CPoly( f->polygon->pointnum );
	cpoly->setPlane( f->plane.norm, f->plane.dist );
	//
	// get center of face as CtrlPoint 0
	//
	PolygonCenter( f->polygon, center );
	
	//
	// generate CtrlPoints
	//
	for ( i = 0; i < f->polygon->pointnum; i++ )
	{
		float	*p1, *p2;
		vec3d_t		tmp;
		p1 = f->polygon->p[i];
		p2 = f->polygon->p[(i+1)%f->polygon->pointnum];
		Vec3dAdd( tmp, p1, p2 );
		Vec3dScale( tmp, 0.5, tmp );

		Vec3dPrint( p1 );
		Vec3dPrint( tmp );
		
		CtrlPoint	*cp;
		cp = new CtrlPoint( p1 );
		cp->setPos( p1, i*2, 0 );
		cpoly->insertCtrlPoint( cp );
		cp = new CtrlPoint( tmp );
		cp->setPos( tmp, i*2+1, 0 );
		cpoly->insertCtrlPoint( cp );
	}

//	cptexdef_t	td;
	brushsetting_wdg_i->getTexDef( &f->texdef );
	cpoly->setTexdef( &f->texdef );
	
	wwm_i->insertCPoly( cpoly, true );
	updateViews();
}


/*
  ====================
  doSelectCPoly

  ====================
*/
void Wired::doSelectCPoly( vec3d_t start, vec3d_t dir )
{
	CPolyIterator		iter( wwm_i->getFirstCPoly() );

	EAL_BlueRaySelector( &iter, start, dir );
	updateViews();
}


/*
  ====================
  doSelectCPolyCtrlPoint

  ====================
*/
void Wired::doSelectCPolyCtrlPoint( vec3d_t start, vec3d_t dir )
{
	CPolyIterator		iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly			*cpoly;

	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		CtrlPointIterator	cpiter( cpoly->getFirstCtrlPoint() );
		EAL_BlueRaySelector( &cpiter, start, dir );	
	}

	updateViews();
}



/*
  ====================
  doApplyTextureToCPoly

  ====================
*/
void Wired::doApplyTextureToCPoly( void )
{
	cptexdef_t	td;
	CPolyIterator	iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cpoly;

	brushsetting_wdg_i->getTexDef( &td );

	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		cptexdef_t	td2;
		cpoly->getTexdef( &td2 );
		strcpy( td2.ident, td.ident );
		cpoly->setTexdef( &td2 );
	}
}



/*
  ====================
  doApplyTexdefWdgToCPoly

  ====================
*/
void Wired::doApplyTexdefWdgToCPoly( void )
{
	cptexdef_t	td;
	CPolyIterator	iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cpoly;

	brushsetting_wdg_i->getTexDef( &td );

	for ( iter.reset(); ( cpoly = iter.getNext() ); )
	{
		cpoly->setTexdef( &td );
	}
}



/*
  ====================
  doApplyCPolyToTexdefWdg

  ====================
*/
void Wired::doApplyCPolyToTexdefWdg( void )
{
	cptexdef_t	td;
	CPolyIterator	iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cpoly;

	iter.reset();
	cpoly = iter.getNext();

	if ( !cpoly )
	{
		wired_i->printComment( "No CPoly selected." );
		return;
	}

	cpoly->getTexdef( &td );
	brushsetting_wdg_i->setTexDef( &td );
}


/*
  ==============================
  doRotateCPoly

  ==============================
*/
void Wired::doRotateCPoly( float roll, float pitch, float yaw )
{
	printf( "doRotateCPoly: not yet, shame!\n" );
}

/*
  ==============================
  doCopyCPoly

  ==============================
*/
void Wired::doCopyCPoly( void )
{
	CPolyIterator	cpi( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cp;
	CPoly		*cpnew;

	// get blue
	cpi.reset();
	cp = cpi.getNext();

	if ( !cp )
	{
		wired_i->printComment( "No CPoly selected." );
		return;
	}

	cpnew = cp->copySelf();
	wwm_i->insertCPoly( cpnew, true );

	updateViews();
}

/*
  ==============================
  doDeleteCPoly

  ==============================
*/
void Wired::doDeleteCPoly( void )
{
	CPolyIterator	cpi( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cp;

	// get blue
	cpi.reset();
	cp = cpi.getNext();

	if ( !cp )
	{
		wired_i->printComment( "No CPoly selected." );
		return;
	}

	wwm_i->removeCPoly( cp );
	delete cp;

	redrawViews();
}


/*
  ==============================
  doCPolyPlaneFromClipper

  ==============================
*/
void Wired::doCPolyPlaneFromClipper( void )
{
	CPolyIterator	iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cpoly;
	plane_t		pl;

	iter.reset();
	cpoly = iter.getNext();

	if ( !cpoly )
	{
		wired_i->printComment( "No CPoly selected." );
		return;
	}

	if ( !clippertool_i->isPlaneValid() )
	{
		wired_i->printComment( "No valid plane defined by clipper tool." );	
		return;
	}

	clippertool_i->getSplitPlane( &pl );
	cpoly->setPlane( pl.norm, pl.dist );
	
	redrawViews();	
}

/*
  ==============================
  doCPolyPlaneToClipper

  ==============================
*/
void Wired::doCPolyPlaneToClipper( void )
{
	CPolyIterator	iter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
	CPoly		*cpoly;
	plane_t		pl;
	CtrlPoint *cp;
	vec3d_t		center;
	int		tmp_int;

	iter.reset();
	cpoly = iter.getNext();

	if ( !cpoly )
	{
		wired_i->printComment( "No CPoly selected." );
		return;
	}

	cpoly->getPlane( pl.norm, &pl.dist );	

	cp = cpoly->getFirstCtrlPoint();
	
	if ( !cp )
		return;

	cp->getPos( center, &tmp_int, &tmp_int );

	clippertool_i->setSplitPlane( &pl, center );
	
	redrawViews();	
}

/*
  ==============================
  doSetBrushSettingWdg_CPoly

  ==============================
*/
void Wired::doSetBrushSettingWdg_CPoly( vec3d_t start, vec3d_t dir )
{
	puts( "doSetBrushSettingWdg_CPoly" );

	CPoly	*cpoly;

	wwm_i->findBestCPolyForRay( start, dir, &cpoly );
	if ( !cpoly )
	{
		wired_i->printComment( "No cpoly found." );
	}
	else
	{
		texdef_t	tmp1;
		cptexdef_t	tmp2;

		cpoly->getTexdef( &tmp2 );

		// !!! HACK !!!
		memcpy( &tmp1, &tmp2, sizeof( texdef_t ) );

		brushsetting_wdg_i->setTexDef( &tmp1 );

		draw3d_i->startDraw( VIEW_CAMERA );
		draw3d_i->setColor( colorred_i );
		drawCPoly( cpoly );
		draw3d_i->endDraw();
	}
}


/*
  ==============================
  doGetBrushSettingWdg_CPoly

  ==============================
*/
void Wired::doGetBrushSettingWdg_CPoly( vec3d_t start, vec3d_t dir )
{
	puts( "doGetBrushSettingWdg_CPoly" );

	CPoly	*cpoly;

	wwm_i->findBestCPolyForRay( start, dir, &cpoly );
	if ( !cpoly )
	{
		wired_i->printComment( "No cpoly found." );
	}
	else
	{
		texdef_t	tmp1;
		cptexdef_t	tmp2;


		brushsetting_wdg_i->getTexDef( &tmp1 );

		// !!! HACK !!!
		memcpy( &tmp2, &tmp1, sizeof( texdef_t ) );

		cpoly->setTexdef( &tmp2 );

		draw3d_i->startDraw( VIEW_CAMERA );
		draw3d_i->setColor( coloryellow_i );
		drawCPoly( cpoly );
		draw3d_i->endDraw();
	}	
}



/*
  ==================================================
  new EditAble stuff

  ==================================================
*/

/*
  ========================================
  TestBox stuff

  ========================================
*/

/*
  ====================
  doCreateTestBox

  ====================
*/
void Wired::doCreateTestBox( void )
{
	TestBox		*box;
	vec3d_t		spawn;

	cursor3d_i->get( spawn );
	box = new TestBox( spawn );
	
	wwm_i->insertTestBox( box );
	updateViews();
}

/*
  ====================
  doSelectTestBox

  ====================
*/
void Wired::doSelectTestBox( vec3d_t start, vec3d_t dir )
{
	TestBoxIterator	iter( wwm_i->getFirstTestBox() );

//	list = wwm_i->getFirstTestBox();
	EAL_BlueRaySelector( &iter, start, dir );
	updateViews();

}



/*
  ====================
  doAddCtrlPointToTestBox

  blue TestBox
  ====================
*/
void Wired::doAddCtrlPointToTestBox( void )
{
	TestBoxIterator	iter( wwm_i->getFirstTestBox() );
	TestBox		*box;
	
	for ( iter.reset(); (box=iter.getNext()); )
	{
		if ( box->testFlagSelect( SELECT_BLUE ) )
		{
			printf( "Wired::doAddCtrlPointToTestBox: found blue\n" );
		}
	}
}

/*
  ======================
  Events
  ======================
*/

void Wired::resizeEvent ( QResizeEvent * )
{
	layoutChanged();
}

void Wired::paintEvent ( QPaintEvent * )
{
	redrawViews();
}

// *****************************************************************************

/*
  ==================================================
  draw stuff

  ==================================================
*/

/*
  ====================
  drawBB

  ====================
*/

#define DRAW_LINE( _V1, _V2 ) {		\
	xzview_i->draw3dLine( _V1, _V2 );	\
	yview_i->draw3dLine( _V1, _V2 );	\
	}

#define V3INIT( _v, _x1, _x2, _x3 ) {	\
	_v[0] = _x1;			\
	_v[1] = _x2;			\
	_v[2] = _x3;			\
	}

void Wired::drawBB( vec3d_t min, vec3d_t max )
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

	DRAW_LINE( v1, v2 );
	DRAW_LINE( v2, v3 );
	DRAW_LINE( v3, v4 );
	DRAW_LINE( v4, v1 );

	DRAW_LINE( v5, v6 );
	DRAW_LINE( v6, v7 );
	DRAW_LINE( v7, v8 );
	DRAW_LINE( v8, v5 );

	DRAW_LINE( v1, v5 );
	DRAW_LINE( v2, v6 );
	DRAW_LINE( v3, v7 );
	DRAW_LINE( v4, v8 );	
}


/*
  ====================
  drawUVMesh

  ====================
*/
void Wired::drawUVMesh( uvmesh_t *mesh )
{
	int		u, v;
	vec3d_t		p, q;

	// draw u lines
	for ( v = 0; v < mesh->vpointnum; v++ )
		for ( u = 0; u < mesh->upointnum-1; u++ )
		{
			GetUVMeshPoint( mesh, u, v, p );
			GetUVMeshPoint( mesh, u+1, v, q );

//			DRAW_LINE( p, q );
//			CameraDrawLine( p, q );
			draw3d_i->drawLine( p, q );

		}

	// draw v lines
	for ( u = 0; u < mesh->upointnum; u++ )
		for ( v = 0; v < mesh->vpointnum-1; v++ )
		{
			GetUVMeshPoint( mesh, u, v, p );
			GetUVMeshPoint( mesh, u, v+1, q );

//			DRAW_LINE( p, q );
//			CameraDrawLine( p, q );
			draw3d_i->drawLine( p, q );
		}
}


/*
  ========================================
  drawTestBox

  ========================================
*/
void Wired::drawTestBox( TestBox *box )
{
	vec3d_t		min, max;



	if ( !box->testFlagSelect( SELECT_VISIBLE ) )
		return;
	if ( !box->testFlagSelect( SELECT_UPDATE ) )
		return;

	if ( box->testFlagSelect( SELECT_BLUE ) )
	{
		printf( "blue " );
		xzview_i->setColor( colorblue_i );
		yview_i->setColor( colorblue_i );
	}
	else
	{
		printf( "normal " );
		xzview_i->setColor( colorblack_i );
		yview_i->setColor( colorblack_i );		
	}

	printf( "draw TestBox\n" );

	box->getBB( min, max );

	this->drawBB( min, max );

	if ( box->testFlagSelect( SELECT_BLUE ) )
	{
		//
		// draw CtrlPoints
		//
		CtrlPointIterator	iter( box->getFirstCtrlPoint() );
		CtrlPoint		*cp;
		vec3d_t			v;
		for ( iter.reset(); (cp=iter.getNext()); )
		{
			int s, t;
			cp->getPos( v, &s, &t );
			xzview_i->draw3dCross( v );
			yview_i->draw3dCross( v );
		}
	}

	box->resetFlagSelect( SELECT_UPDATE );
}



/*
  ====================
  drawCSurface

  use Draw3d
  ====================
*/
void Wired::drawCSurface( CSurface *cs )
{
	
	if ( !cs->testFlagSelect( SELECT_VISIBLE ) )
		return;
       
	// fixme: update blue always, cause CtrlPoints can change
	if ( !cs->testFlagSelect( SELECT_BLUE ) )
		if ( !cs->testFlagSelect( SELECT_UPDATE ) )
			return;

//	printf( "Wired::drawCSurface\n" );

	if ( cs->testFlagSelect( SELECT_BLUE ) )
	{
		// surface mesh
//		xzview_i->setColor( colorgray30_i );
//		yview_i->setColor( colorgray30_i );
		draw3d_i->setColor( colorgray30_i );

		uvmesh_t *mesh = cs->generateUVMesh( 10, 10 );
		drawUVMesh( mesh );
		FreeUVMesh( mesh );
	
		// bb
		vec3d_t		min, max;
//		xzview_i->setColor( colorblue_i );
//		yview_i->setColor( colorblue_i );
		draw3d_i->setColor( colorblue_i );
		cs->getBB( min, max );
//		drawBB( min, max );	
		draw3d_i->drawBB( min, max );
	
#if 0
		// CtrlPoint mesh
		mesh = cs->generateUVMesh( 3, 3 );
//		xzview_i->setColor( colorgray30_i );
//		yview_i->setColor( colorgray30_i );
		draw3d_i->setColor( colorgray30_i );
		
		drawUVMesh( mesh );
		FreeUVMesh( mesh );
#endif

		CtrlPointIterator	cpiter( cs->getFirstCtrlPoint() );
		CtrlPoint		*cp;
		CtrlPoint		*cpblue = NULL;	// hack: paint blue CtrlPoint after all others
		vec3d_t		p;
		int		u, v;
		for ( cpiter.reset(); (cp = cpiter.getNext()); )
		{
			if ( cp->testFlagSelect( SELECT_BLUE ) )
			{
				cpblue = cp;
				continue;
			}
			
//			xzview_i->setColor( colorblack_i );
//			yview_i->setColor( colorblack_i );	
			draw3d_i->setColor( colorblack_i );

			cp->getPos( p, &u, &v );
//			xzview_i->draw3dCross( p );
//			yview_i->draw3dCross( p );
			draw3d_i->drawCross( p );

		}
		if ( cpblue )
		{			
//			xzview_i->setColor( colorblue_i );
//			yview_i->setColor( colorblue_i );
			draw3d_i->setColor( colorblue_i );

			cpblue->getPos( p, &u, &v );	

//			xzview_i->draw3dCross( p );
//			yview_i->draw3dCross( p );
			draw3d_i->drawCross( p );
		}
	}
	else
	{


		uvmesh_t *mesh = cs->generateUVMesh( 10, 10 );

//		xzview_i->setColor( colorgray30_i );
//		yview_i->setColor( colorgray30_i );
		draw3d_i->setColor( colorgray30_i );

		drawUVMesh( mesh );
		FreeUVMesh( mesh );
		// bb
		vec3d_t		min, max;

//		xzview_i->setColor( colorblack_i );
//		yview_i->setColor( colorblack_i );		
		draw3d_i->setColor( colorblack_i );

		cs->getBB( min, max );

//		drawBB( min, max );	
		draw3d_i->drawBB( min, max );
	}
	       

	cs->resetFlagSelect( SELECT_UPDATE );
}

void Wired::drawCPoly( CPoly *cpoly )
{
//	printf( "Wired::drawCPoly\n" );

	if ( !cpoly->testFlagSelect( SELECT_VISIBLE ) )
		return;

	// fixme: update blue always, cause CtrlPoints can change
	if ( !cpoly->testFlagSelect( SELECT_BLUE ) )
		if ( !cpoly->testFlagSelect( SELECT_UPDATE ) )
			return;
	
	

	CtrlPointIterator		cpi( cpoly->getFirstCtrlPoint() );
	CtrlPoint			*cp;

	if ( cpoly->testFlagSelect( SELECT_BLUE ) )
	{
		draw3d_i->setColor( colorblue_i );
	}
	else
	{
		draw3d_i->setColor( colorblack_i );
	}

	vec3d_t min, max;
	cpoly->getBB( min, max );
	draw3d_i->drawBB( min, max );	

	int		edgenum = cpoly->getEdgeNum();
	for ( int i = 0; i < edgenum; i++ )
	{
		uvmesh_t *curve = cpoly->generateCurveOfEdge( i, 10 );
		drawUVMesh( curve );
		FreeUVMesh( curve );
	}

	for ( cpi.reset(); ( cp = cpi.getNext() ); )
	{
		vec3d_t		p;
		int		u, v;
		cp->getPos( p, &u, &v );
		if ( cp->testFlagSelect( SELECT_BLUE ) )
		{
			draw3d_i->setColor( colorblue_i );
		}
		else
		{
			draw3d_i->setColor( colorblack_i );
		}
		draw3d_i->drawCross( p );
	}

	cpoly->resetFlagSelect( SELECT_UPDATE );
}

void Wired::paintArcheTypeThreadSlot()
{
	arche_t		*a;
	kvpair_t	*pair;
	kvpair_t	*pair2;
	vec3d_t		v;
	
	QColor		*framecolor;
	QPixmap		*atpix;

	xzview_i->startDraw();
	yview_i->startDraw();

	xzview_i->setColor( colorblack_i );
	yview_i->setColor( colorblack_i );

	for ( a = wwm_i->getFirstArche(); a ; a=a->next )
	{
		if ( !(a->select&SELECT_VISIBLE) )
			continue;

		if ( !(a->select&SELECT_UPDATE) )			
			continue;

		pair = AT_GetPair( a, "origin" );
		if ( !pair )
		{
			printf( "Wired::paintArcheTypeThreadSlot: can't find key \"origin\"\n" );
			continue;
		}

		pair2 = AT_GetPair( a, "type" );
		if ( !pair2 )
		{
			printf( "Wired::paintArcheTypeThreadSlot: can't find key \"type\"\n" );
			continue;		
		}

		AT_CastValueToVec3d( v, pair->value );
	       

		a->select&=~SELECT_UPDATE;

		if ( editmode == EM_ARCHETYPE )
		{
			if ( a->select&SELECT_BLUE )
			{
				framecolor = colorblue_i;
			}
			else
			{
				framecolor = colorblack_i;
			}

			if ( strstr( pair2->value, "spot" ) )
			{
				atpix = xpm_atspot_i;
			}
			else if ( strstr( pair2->value, "dlight" ) )
			{
				atpix = xpm_atdlight_i;
			}
			else if ( strstr( pair2->value, "light" ) )
			{
				atpix = xpm_atlight_i;
			}
			else if ( strstr( pair2->value, "flood" ) )
			{
				atpix = xpm_atflood_i;
			}
			else if ( strstr( pair2->value, "switch" ) )
			{
				atpix = xpm_atswitch_i;
			}
			else
			{
				// default pixmap
				atpix = xpm_arche_i;
			}

			xzview_i->draw3dQPixmap( v, atpix, framecolor );
			yview_i->draw3dQPixmap( v, atpix, framecolor );
			
		}
		else
		{
			xzview_i->draw3dCross( v );
			yview_i->draw3dCross( v );
		}
					       
	}

	xzview_i->endDraw();
	yview_i->endDraw();
}

void Wired::paintArchetypeToLinks( arche_t *a )
{
	kvpair_t		*p, *tmp;
	arche_t		*from;

	vec3d_t		v1, v2;

	draw3d_i->startDraw( VIEW_XZ | VIEW_Y | VIEW_CAMERA );
	draw3d_i->setColor( colorblack_i );

	for ( p = a->pairs; p ; p=p->next )
	{
		if ( !strcasecmp( p->type, "clsref" ) )
		{
			from = wwm_i->searchArche( p->value );

			if ( !from )
			{
				continue;
			}
			
			tmp = AT_GetPair( a, "origin" );
			if ( !tmp )
			{
				continue;
			}
			AT_CastValueToVec3d( v1, tmp->value );
			
			tmp = AT_GetPair( from, "origin" );
			if ( !tmp )
			{
				continue;
			}
			AT_CastValueToVec3d( v2, tmp->value );	

			draw3d_i->drawLine( v1, v2 );
		}
	}

	draw3d_i->endDraw();
}

void Wired::paintArchetypeFromLinks( arche_t *a )
{
	arche_t		*to;
	kvpair_t		*name;

	kvpair_t		*p, *tmp;

	vec3d_t		v1, v2;

	draw3d_i->startDraw( VIEW_XZ | VIEW_Y | VIEW_CAMERA );
	draw3d_i->setColor( colorblack_i );


	// go to all archetypes and search for a clsref to archetype 'a'

	name = AT_GetPair( a, "name" );

	if ( !name )
		return;

	for ( to = wwm_i->getFirstArche(); to ; to=to->next )
	{
		if ( to == a )
			continue;

		for ( p = to->pairs; p ; p=p->next )
		{
			if ( !strcasecmp( p->type, "clsref" ) )
			{
				if ( !strcmp( p->value, name->value ) )
				{
					tmp = AT_GetPair( a, "origin" );
					if ( !tmp )
					{
						continue;
					}
					AT_CastValueToVec3d( v1, tmp->value );
					
					tmp = AT_GetPair( to, "origin" );
					if ( !tmp )
					{
						continue;
					}
					AT_CastValueToVec3d( v2, tmp->value );	
					
					draw3d_i->drawLine( v1, v2 );	
					break;
				}
			}				
		}
	}

	draw3d_i->endDraw();
}

void Wired::paintThreadSlot()
{

	int	i, brushcnt;
//	brush_t		*b;
	face_t		*f;


	if ( updatestate == UPDATE_START )
	{
		updatestate = UPDATE_NORMAL;
		ptbrush = wwm->getFirstBrush();
//		updatestate = UPDATE_NORMAL;
	}

	if ( updatestate == UPDATE_NORMAL )
	{
		printf( "UPDATE_NORMAL\n" );
		xzview_i->startDraw();
		yview_i->startDraw();
		cameraview_i->startDraw();
		
		xzview_i->setColor( qc_brushblack );
		yview_i->setColor( qc_brushblack );
		cameraview_i->setColor( qc_brushblack );
		
		for ( brushcnt = 50; ptbrush && brushcnt > 0; ptbrush=ptbrush->next  ) {
			
			if ( !(ptbrush->select&SELECT_VISIBLE) )
				continue;
			
			if ( !(ptbrush->select&SELECT_UPDATE) )			
				continue;
			
			if ( (ptbrush->select&7) != SELECT_NORMAL) // only if really normal 
				continue;
			
			ptbrush->select&=~SELECT_UPDATE;
			
			stat_updatebrushnum++;
			brushcnt--;
			
			for ( f=ptbrush->faces; f ; f=f->next ) {
				
				// polygon clipped away ?
				if ( !f->polygon )
					continue;
				
				xzview_i->draw3dFace( f );
				yview_i->draw3dFace( f );
				
				for ( i = 0; i < f->polygon->pointnum; i++ ) {
//				XZDrawLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
//					yview_i->draw3dLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
					cameraview_i->draw3dLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
				}
			}
			
			// draw special-contents brushes shifted

			if ( ptbrush->contents != BRUSH_CONTENTS_SOLID )
			{
				if ( ptbrush->contents == BRUSH_CONTENTS_LIQUID )
				{
					xzview_i->setColor( qc_brushliquid );
					yview_i->setColor( qc_brushliquid );
					cameraview_i->setColor( qc_brushliquid );
				}

				if ( ptbrush->contents == BRUSH_CONTENTS_LOCAL )
				{
					xzview_i->setColor( qc_brushlocal );
					yview_i->setColor( qc_brushlocal );
					cameraview_i->setColor( qc_brushlocal );
				}
				
				if ( ptbrush->contents == BRUSH_CONTENTS_DECO )
				{
					xzview_i->setColor( qc_brushdeco );
					yview_i->setColor( qc_brushdeco );
					cameraview_i->setColor( qc_brushdeco );					
				}

				for ( f=ptbrush->faces; f ; f=f->next ) {
					
				// polygon clipped away ?
					if ( !f->polygon )
						continue;
					
					xzview_i->draw3dFace( f, 2 );
					yview_i->draw3dFace( f, 2 );
				}

				xzview_i->setColor( qc_brushblack );
				yview_i->setColor( qc_brushblack );
				cameraview_i->setColor( qc_brushblack );
			}
		}
		
		if ( ptbrush ) {
			qt_paintthread->start( 1, true );
		} 
		else
		{
			updatestate = UPDATE_SELECT;
			ptbrush = wwm_i->getFirstBrush();
		}
		cameraview_i->endDraw();
		xzview_i->endDraw();
		yview_i->endDraw();
	}
	
	if ( updatestate == UPDATE_SELECT )
	{
		printf( "UPDATE_SELECT\n" );
		xzview_i->startDraw();
		yview_i->startDraw();
		cameraview_i->startDraw();

		for ( brushcnt = 50; ptbrush && brushcnt > 0; ptbrush=ptbrush->next  ) {
				
//			if ( !b->visible )
			if ( !(ptbrush->select&SELECT_VISIBLE) )
				continue;
	
//			if ( abs(b->status) <= BS_NORMAL )
//				break;
	
			if ( !(ptbrush->select&SELECT_UPDATE ) )
				continue;
	
			if ( ptbrush->select&SELECT_NORMAL )
				continue;

			ptbrush->select&=~SELECT_UPDATE;
		
//			if ( b->status < 0 )
//				b->status = -b->status;
//			else
//				continue;
			
//			switch( b->status ) {
			if ( ptbrush->select&SELECT_BLUE )
			{
				xzview_i->setColor( qc_brushblue );
				yview_i->setColor( qc_brushblue );
				cameraview_i->setColor( qc_brushblue );
			}
			else
			{
				if ( (ptbrush->select&3) == SELECT_RED )
				{
					xzview_i->setColor( qc_brushred );
					yview_i->setColor( qc_brushred );
					cameraview_i->setColor( qc_brushred );
				}
				if ( (ptbrush->select&3) == SELECT_GREEN )
				{
					xzview_i->setColor( qc_brushgreen );
					yview_i->setColor( qc_brushgreen );
					cameraview_i->setColor( qc_brushgreen );
				}
			}
			
			stat_updatebrushnum++;
			
			for ( f=ptbrush->faces; f ; f=f->next ) {
				
				// polygon clipped away ?
				if ( !f->polygon )
					continue;

				xzview_i->draw3dFace( f );
				yview_i->draw3dFace( f );

				for ( i = 0; i < f->polygon->pointnum; i++ ) {
//					yview_i->draw3dLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
					cameraview_i->draw3dLine( f->polygon->p[i], f->polygon->p[(i+1)%f->polygon->pointnum] );
				}
			}
		}
		
		if ( ptbrush )
		{
			qt_paintthread->start( 1, true );
		}
		else
		{
			updatestate = UPDATE_FINISH;
			ptbrush = wwm_i->getFirstBrush();			
		}
		cameraview_i->endDraw();
		xzview_i->endDraw();
		yview_i->endDraw();
	}		

	if ( updatestate == UPDATE_FINISH )
	{
		printf( "UPDATE_FINISH\n" );
		// finish update
		drawSelf();
		clippertool_i->drawSelf();

		printf(" stat_updatebrushnum = %d\n", stat_updatebrushnum );

	}
}

/* 
  =======================================================
  Slots

  =======================================================
*/


void Wired::xzviewOriginChangedSlot()
{
//	vec2d_t		min, max;
	vec2d_t		xzorigin;
	vec2d_t		yorigin;

	printf("Wired::xzviewOriginChangedSlot.\n");

	xzview->getOrigin( xzorigin );

	origin[0] = xzorigin[0];
	origin[2] = xzorigin[1];

	yview->getOrigin( yorigin ); // update yview
	yorigin[0] = xzorigin[0];
	yview->setOrigin( yorigin );

	setViewBounds();

//	Vec3dPrint( viewbounds[0] );	Vec3dPrint( viewbounds[1] );

	updateViews();
}

void Wired::yviewOriginChangedSlot()
{
//	vec2d_t		min, max;
	vec2d_t		yorigin;

	xzview->getOrigin( yorigin );
	xzview->setOrigin( yorigin ); // trigger drawSelf

	origin[1] = yorigin[1];

	setViewBounds();

//	Vec3dPrint( viewbounds[0] );	Vec3dPrint( viewbounds[1] );

	updateViews();
}


void Wired::xzviewPannerChangedSlot( int dx )
{
	int	realheight;
	int	h;

	printf("Wired::xzviewPannerChangedSlot. %d\n", dx );

	realheight = xzview->height();
	
	h = realheight + dx;

	if ( h > 30 && h < height()-30 ) {

		panratio = (float)h / (float)height();
		printf(" panratio = %f\n", panratio );
	}
	
//	xzview->setGeometry( 80, 10, 
	layoutChanged();
	setViewBounds();
	updateViews();
}

void Wired::CSTexDefChangedSlot( void )
{
	if ( editmode == EM_CSURFACE )
	{
		doApplyTexdefWdgToCSurface();
	}
}

void Wired::TexDefChangedSlot( void )
{
#if 0
	if ( editmode == EM_CPOLY )
	{
		doApplyTexdefWdgToCPoly();
	}
#endif
}

void Wired::texBrwAcceptSlot( char *ident )
{

	printf("Wired::texBrwAcceptSlot\n");
	printf(" ident = %s\n", ident );

	if ( editmode == EM_BRUSH )
	{
		texdef_t	td;
		brushsetting_wdg_i->getTexDef( &td );
		strcpy( td.ident, ident );
		brushsetting_wdg_i->setTexDef( &td );
		
		doApplyTexture();
	}
	else if ( editmode == EM_CSURFACE )
	{
		cstexdef_t	td;
		brushsetting_wdg_i->getCSTexDef( &td );
		strcpy( td.ident, ident );
		brushsetting_wdg_i->setCSTexDef( &td );

		doApplyTextureToCSurface();
		
	}
	else if ( editmode == EM_CPOLY )
	{
		cptexdef_t	td;
		brushsetting_wdg_i->getTexDef( &td );
		strcpy( td.ident, ident );
		brushsetting_wdg_i->setTexDef( &td );

		doApplyTextureToCPoly();
	}
	// fix me !

//	strcpy( currenttexdef.ident, ident );

//	texdef_i -> setTexDef( &currenttexdef );
#if 0
	for( b = wwm_i->getFirstBrush(); b ; b=b->next ) {
		
		if ( !(b->select&SELECT_BLUE) )
			continue;
		
		for( f = b->faces; f ; f=f->next ) {
			strcpy( f->texdef.ident, ident );
		}
	}
#endif
}

// GUI SLOTS

// KEY ACCEL

const float	w_rotstep[] = { 2*M_PI/360.0,	// 1
				2*M_PI/72.0,	// 5
				2*M_PI/36.0,	// 10
				2*M_PI/12.0,	// 30
				2*M_PI/8.0,	// 45
				2*M_PI/4.0,	// 90
				2*M_PI/64.0,	// 5.625
				2*M_PI/32.0,	// 11.25
				2*M_PI/16.0,	// 22.5
				2*M_PI/13.55163962 // atan 0.5
};

void Wired::guiKeySlot( int item )
{
	int keycode;
	vec3d_t		v; // copy of cursor3d, some functions need

	keycode = accel->key( item );

	switch( keycode ) {

	case Key_T:
		if ( editmode == EM_ARCHETYPE )
		{
			// draw archetype to-links
			if ( atbrowser->getCurrentArche() )
			{
				paintArchetypeToLinks( atbrowser->getCurrentArche() );
			}
		}
		break;

	case Key_M:
		printf( "m) Mesh mode, CSurface.\n" );
		this->changeEditMode( EM_CSURFACE );
		break;

	case Key_M+SHIFT:
		printf( "M) Mesh mode, CPoly.\n" );
		this->changeEditMode( EM_CPOLY );
		break;

	case Key_B+SHIFT:
		printf( "B) set blue+red to BrushContentsWdg.\n" );
		doOverrideBrushContents();
		break;

	case Key_F+SHIFT:
		printf( "F) set blue+red to SurfaceContentsWdg.\n" );
		doOverrideSurfaceContents();
		break;

		//
		// hack
		//
	case Key_F+SHIFT+CTRL:
		printf( "F+SHIFT+CTRL) hack: set all surfaces of solid or deco brushes to closed+textured.\n" );
		{
		  brush_t		*b;
		  face_t		*f;
		  
		  for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
		    {
		      if ( b->contents & BRUSH_CONTENTS_SOLID ||
			   b->contents & BRUSH_CONTENTS_DECO )
			{
			  for ( f = b->faces; f ; f=f->next )
			    {
			      f->contents = SURFACE_CONTENTS_CLOSE | SURFACE_CONTENTS_TEXTURE;
			    }
			}
		    }
		}
		redrawViews();
		break;

	case Key_F+CTRL:
		printf( "F+CTRL hack: set selected cpoly to clipper tool plane\n" );
		doCPolyPlaneFromClipper();

		break;

	case Key_G+CTRL:
		printf( "G+CTRL hack: set clipper plane tool from cpoly plane\n" );
		doCPolyPlaneToClipper();

		break;

	case Key_W:
	{
//		int		i;
		unique_t	id;
		brush_t		*b;
		
		printf( "w) where is unique brush ?\n" );
		printf( "enter id: ");

		int num = scanf( "%u", &id );

		if ( num == 1 ) {
		
			printf( "id: %d\n", id );



			wwm_i->deselectBrushes();

			for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
			{
				if ( b->id == id )
				{
					b->select|=(SELECT_UPDATE|SELECT_BLUE);
					break;
				}
			}
			redrawViews();
		}
		break;
	}
	
	case Key_F1:
		printf(" f1) render cameraview.\n");
		cameraview_i->render();
		
		break;

	case Key_F2:
		printf(" f2) update all views.\n");
		// update views...
		xzview_i->drawSelf();
		yview_i->drawSelf();
		cameraview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();
		break;

	case Key_F3:
		printf(" f3) swap xz ...\n");	
		doSwapWorld();
		break;

	case Key_F4:
		//
		// read portal file and draw it
		//
		Draw_Trace( prodir );
		break;

	case Key_F5:
		//
		// render debug csg-faces
		//
		printf(" f5) render cameraview debug-csg-faces.\n");
		cameraview_i->render_csg();
		
		break;

	case Key_V:
		printf( "v) set camera viewport.\n" );
		cursor3d_i->get( cameraorigin );
		cursor3d_i->getTo( cameralookat );
		cameraview_i->setCamera( cameraorigin, cameralookat );

		xzview_i->drawSelf();
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();	

		break;

	case Key_O:
		printf( "o) copy cursor3d origin to currentpair.\n" );
		cursor3d_i->get( v );
		atbrowser_i->changeCurrentValueVec3d( v );

		break;

	case Key_P:
		printf( "p) copy cursor3d delta to currentpair.\n" );
		cursor3d_i->getDelta( v );
		atbrowser_i->changeCurrentValueVec3d( v );

		break;

	case Key_E:
		printf( " e) brush tool mode: extrude\n" );
		this->changeEditMode( EM_BRUSH );
		submode_brush = EM_BRUSH_EXTRUDE;
		break;

	case Key_E+SHIFT:
		printf( " E) brush tool mode: face scale\n" );
		this->changeEditMode( EM_BRUSH );
		submode_brush = EM_BRUSH_FACESCALE;
		break;

	case Key_C:
		printf(" c) clipper tool with reset.\n" );
		this->changeEditMode( EM_BRUSH );
//		this->changeEditMode( EM_CLIPPER );
		submode_brush = EM_BRUSH_CLIPPER;
		clippertool_i->reset();
		break;

	case Key_C+SHIFT:
		printf(" C) clipper tool without reset.\n" );
		this->changeEditMode( EM_BRUSH );
//		this->changeEditMode( EM_CLIPPER );
		submode_brush = EM_BRUSH_CLIPPER;
		break;


	case Key_B:
		printf(" b) edit brushes.\n" );
		this->changeEditMode( EM_BRUSH );
		submode_brush = EM_BRUSH_NORMAL;
		break;

	case Key_A:
		printf(" a) edit archetypes.\n" );
		this->changeEditMode( EM_ARCHETYPE );
		break;

	case Key_Space:
		printf(" space) tool dependend action.\n" );
		
		if ( editmode == EM_BRUSH /*&& submode_brush == EM_BRUSH_CLIPPER*/ ) 
		{
			clippertool_i -> clipBrushes();
			redrawViews();
		}
		else if ( editmode == EM_TESTBOX )
 		{
			doAddCtrlPointToTestBox();
		}
		else if ( editmode == EM_ARCHETYPE )
		{
			// set link wdg to the current archetype
			atbrowser->getLinkFromCurrent();
		}

		break;

	case Key_Space+SHIFT:
		printf(" space) second tool dependend action.\n" );
		
		if ( editmode == EM_BRUSH /*&& submode_brush == EM_BRUSH_CLIPPER*/ ) 
		{
			clippertool_i -> splitBrushes();
			redrawViews();
		}
		else if ( editmode == EM_ARCHETYPE )
		{
			// get link wdg into the current archetype
			atbrowser->setLinkOfCurrent();
		}
		
		break;

	case Key_Escape:
		printf(" esc) tool dependend reset.\n" );

		if ( editmode == EM_BRUSH /*&& submode_brush == EM_BRUSH_CLIPPER*/ ) 
		{
			clippertool_i -> reset();
		}

		break;

	case Key_G:
		printf(" g) add blue to green.\n");
		doAddToGreen();
		break;

	case Key_R:
		printf(" r) add blue to red.\n");
		doAddToRed();
		break;

	case Key_U:
		printf(" u) brush status to normal.\n");
		doAddToNormal();
		break;

	case Key_U+SHIFT:
		printf(" U) all brushes to normal status ( deselect all ).\n");
		doAllAddToNormal();
		break;

	case Key_L:
		if ( editmode == EM_BRUSH )
		{
			printf(" l) delete singlebrush.\n");
			doDeleteBrush();
		}
		else if ( editmode == EM_ARCHETYPE )
		{
			printf( " l) delete arche.\n" );
			doDeleteArche();
		}
		else if ( editmode == EM_CSURFACE )
		{
			printf( " l) delete CSurface.\n" );
			doDeleteCSurface();
		}
		else if ( editmode == EM_CPOLY )
		{
			printf( " l) delete CPoly.\n" );
			doDeleteCPoly();
		}
		break;

	case Key_L+SHIFT:
		printf(" L) delete singlebrush and red-brushes.\n");
		break;

	case Key_K:
		if ( editmode == EM_BRUSH )
		{
			printf(" k) copy blue brush.\n");
			doCopyBrush();
		}
		else if ( editmode == EM_ARCHETYPE )
		{
			printf( "k) copy blue archetype.\n" );
			doCopyArche();
		}
		else if ( editmode == EM_CSURFACE )
		{
			printf( "k) copy CSurface.\n" );
			doCopyCSurface();
		}
		else if ( editmode == EM_CPOLY )
		{
			printf( "k) copy CPoly.\n" );
			doCopyCPoly();
		}
			
		break;

	case Key_K+CTRL:
		if ( editmode == EM_BRUSH )
		{
			printf( " k+ctrl) copy brush to clipboard.\n" );
			doCopyBrushToClipboard();
		}
		break;

	case Key_I:
		if ( editmode == EM_BRUSH )
		{
			printf( " i) insert clipboard.\n" );
			doInsertBrushFromClipboard();
		}
		break;

	case Key_F:

		if ( editmode == EM_ARCHETYPE )
		{
			// draw archetyp from-links
			if ( atbrowser->getCurrentArche() )
			{
				paintArchetypeFromLinks( atbrowser->getCurrentArche() );
			}
		}
		else
		{
			printf(" f) flip clipperplane normal.\n");
			clippertool_i->flip();
		}
		break;

// revision
	case Key_N:

		//
		// new objects
		//

		if ( editmode == EM_BRUSH )
		{
			doCreateBrush();
		}
		else if ( editmode == EM_ARCHETYPE )
		{
			doCreateArche();
		}
		else if ( editmode == EM_TESTBOX )
		{
			// new TestBox
			doCreateTestBox();
		}
		break;
		

	case Key_N+SHIFT:
		if ( editmode == EM_ARCHETYPE )
		{
			doGetUniqueNameForArche();
		}
		break;

	case Key_Y:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		break;

	case Key_Y+SHIFT:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( -w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( -w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( -w_rotstep[qcb_rotstep->currentItem()], 0.0, 0.0 );
		}
		break;

	case Key_X:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( 0.0, w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( 0.0, w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( 0.0, w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		break;

	case Key_X+SHIFT:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( 0.0, -w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( 0.0, -w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( 0.0, -w_rotstep[qcb_rotstep->currentItem()], 0.0 );
		}
		break;

	case Key_Z:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( 0.0, 0.0, w_rotstep[qcb_rotstep->currentItem()] );
		} 
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( 0.0, 0.0, w_rotstep[qcb_rotstep->currentItem()] );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( 0.0, 0.0, w_rotstep[qcb_rotstep->currentItem()] );
		}
		break;

	case Key_Z+SHIFT:
		if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_NORMAL )
		{
			doRotateBrush( 0.0, 0.0, -w_rotstep[qcb_rotstep->currentItem()] );
		}
		else if ( editmode == EM_BRUSH && submode_brush == EM_BRUSH_CLIPPER )
		{
			doRotateSplitPlane( 0.0, 0.0, -w_rotstep[qcb_rotstep->currentItem()] );
		}
		else if ( editmode == EM_CSURFACE )
		{
			doRotateCSurface( 0.0, 0.0, -w_rotstep[qcb_rotstep->currentItem()] );
		}
		break;


	case Key_S:
		clippertool_i->csgBrushes();
		redrawViews();
		break;


		// test & development stuff

	case Key_P+CTRL:	// pump blue brush
		if ( editmode == EM_BRUSH )
		{
			brush_t		*b;
			face_t		*f;
			
			for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
			{
				if ( ! (b->select&SELECT_BLUE) )
					continue;
				for ( f = b->faces; f ; f=f->next )
				{
					vec3d_t		p;
					Vec3dScale( p, f->plane.dist, f->plane.norm );
					Vec3dMA( p, 16.0, f->plane.norm, p );
					f->plane.dist = Vec3dDotProduct( f->plane.norm, p );

					if ( f->polygon )
						FreePolygon( f->polygon );
				}
				CleanUpBrush( b );
			}
			redrawViews();
		}
		break;

	case Key_F10+CTRL:
		if ( editmode == EM_BRUSH )
		{
			printf( "F10+CTRL: all brushes/faces get new ID\n" );
			
			brush_t		*b;
			face_t		*f;
			
			for ( b = wwm_i->getFirstBrush(); b ; b=b->next )
			{
				b->id = wwm_i->getID();
				for ( f = b->faces; f ; f=f->next )
				{
					f->id = wwm_i->getID();
				}
			}
		}
		break;

	case Key_F9+CTRL:
	{
		vec3d_t		dir;

		cursor3d_i->getDelta( dir );
		doMoveWorld( dir );
	}
	break;

	default:
		printf(" default case.\n");
		break;
	}
}

// MouseEvents from views

void Wired::xzMouseEventSlot( int type, int state, Vec2 _snap, Vec2 _pos )
{
	char		text[256];
	vec3d_t		v;
	vec3d_t		snap3;

	vec2d_t		snap;
	vec2d_t		pos;

	_snap.get(snap);
	_pos.get(pos);

	printf("Wired::xzMouseEventSlot\n");
	printf(" type = %d\n", type );
	printf(" state = %d\n", state );
	Vec2dPrint( pos );

	snap3[0] = snap[0];
	snap3[1] = 0;
	snap3[2] = snap[1];

	// start edit-cycle ( press-drag-release ), the button state is kept ( shift-alt-ctrl )
	// to avoid odd behavior if button state changes during a cycle
	if ( type == 0 )
		buttonstate = state;


	// brush mode actions
	if ( editmode == EM_BRUSH ) 
	{
		
		// all submodes can select
		if ( type == 0 && buttonstate == RightButton )
		{
//			selecttool_i -> pressSlot( pos );
			vec3d_t		start, dir;
			start[0] = pos[0];
			start[1] = 8000.0;
			start[2] = pos[1];
			dir[0] = 0.0;
			dir[1] = -1.0;
			dir[2] = 0.0;
			doSelectBrush( start, dir );
		}
		
		if ( submode_brush == EM_BRUSH_NORMAL )
		{
			
			if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> xzPressSlot( snap, SELECT_BLUE );
			
			if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> xzDragSlot( snap, SELECT_BLUE );
			
			if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> xzReleaseSlot( snap, SELECT_BLUE );
			
			
			if ( type == 0 && buttonstate == (LeftButton | ControlButton | ShiftButton ) )
				movetool_i -> xzPressSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 1 && buttonstate == (LeftButton | ControlButton | ShiftButton ) )
				movetool_i -> xzDragSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 2 && buttonstate == (LeftButton | ControlButton | ShiftButton ) )
				movetool_i -> xzReleaseSlot( snap, SELECT_BLUE | SELECT_RED );
			
			
			if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_xzPressSlot( snap, SELECT_BLUE );
			
			if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_xzDragSlot( snap, SELECT_BLUE );
			
			if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_xzReleaseSlot( snap, SELECT_BLUE );
			
			if ( type == 0 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_xzPressSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 1 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_xzDragSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 2 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_xzReleaseSlot( snap, SELECT_BLUE | SELECT_RED );
		}
		
#if 0
		if ( submode_brush == EM_BRUSH_CLIPPER )
		{
			if ( type == 0 && buttonstate == LeftButton )
				clippertool_i -> xzPressSlot( snap );
		}
#endif
		if ( submode_brush == EM_BRUSH_NORMAL )
		{
			// test set plane point with left+shift
			if ( type == 0 && buttonstate == (LeftButton | ShiftButton ) )
				clippertool_i -> xzPressSlot( snap );
		}
			
	}
	else if ( editmode == EM_ARCHETYPE ) 
	{
		if ( type == 0 && buttonstate == RightButton )
		{
//			atbrowser_i -> xzSelectSlot( pos );
			vec3d_t		start, dir;
			start[0] = pos[0];
			start[1] = 8000.0;
			start[2] = pos[1];
			dir[0] = 0.0;
			dir[1] = -1.0;
			dir[2] = 0.0;
			doSelectArche( start, dir );
		}

		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> pressSlot( snap3 );

		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> dragSlot( snap3 );

		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> releaseSlot( snap3 );
	}
	else if ( editmode == EM_TESTBOX )
	{
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], 8000.0, pos[1] );
			Vec3dInit( dir, 0.0, -1.0, 0.0 );
			doSelectTestBox( start, dir );
		}

		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			TestBoxIterator		*iter;
			iter = new TestBoxIterator( wwm_i->getFirstTestBox(), new TestBoxChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}

		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}


	}
	else if ( editmode == EM_CSURFACE )
	{

		// CSurface stuff
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], 8000.0, pos[1] );
			Vec3dInit( dir, 0.0, -1.0, 0.0 );
			doSelectCSurface( start, dir );
		}
		
		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			CSurfaceIterator		*iter;
			iter = new CSurfaceIterator( wwm_i->getFirstCSurface(),new CSurfaceChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}

		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}
		
		// CtrlPoint stuff
		
		if ( type == 0 && buttonstate == ( RightButton | ControlButton ) )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], 8000.0, pos[1] );
			Vec3dInit( dir, 0.0, -1.0, 0.0 );
			doSelectCSurfaceCtrlPoint( start, dir );			
		}

		if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
		{
			// search blue CSurface
			CSurfaceIterator	csiter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
			CSurface		*cs;
			
			csiter.reset();
			cs = csiter.getNext(); // there should be only one blue
			
			// get CtrlPointIterator
			CtrlPointIterator	*cpiter;
			if ( cs )
				cpiter = new CtrlPointIterator( cs->getFirstCtrlPoint(), new CtrlPointChecker_select( SELECT_BLUE ) );
			else
				cpiter = new CtrlPointIterator( NULL ); // arrrgh

			movetool_i->startMoveCycle( cpiter, snap3 );
		}

		if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
		{
			movetool_i->dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
		{			
			// recalc bb
			CSurfaceIterator	csi( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
			CSurface		*cs;
			csi.reset();
			cs = csi.getNext();
			if ( cs )
				cs->calcBB();
			else
				printf( "WARNING: internal inconcistancy. can't finish move cycle.\n" );
			
			movetool_i->finishMoveCycle( snap3 );
		}
	}
	else if ( editmode == EM_CPOLY )
	{
		//
		// CPoly select
		//
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], 8000.0, pos[1] );
			Vec3dInit( dir, 0.0, -1.0, 0.0 );
			doSelectCPoly( start, dir );
		}

		//
		// CPoly move
 		//
		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			CPolyIterator		*iter;
			iter = new CPolyIterator( wwm_i->getFirstCPoly(),new CPolyChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}

		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}
		
		//
		// CtrlPoint select
		//
		if ( type == 0 && buttonstate == ( RightButton | ControlButton ) )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], 8000.0, pos[1] );
			Vec3dInit( dir, 0.0, -1.0, 0.0 );
			doSelectCPolyCtrlPoint( start, dir );			

		}
		
		//
		// CtrlPoint move
		//
		if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
		{
			// search blue CPoly
			CPolyIterator	csiter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
			CPoly		*cs;
			
			csiter.reset();
			cs = csiter.getNext(); // there should be only one blue
			
			// get CtrlPointIterator
			CtrlPointIterator	*cpiter;
			if ( cs )
				cpiter = new CtrlPointIterator( cs->getFirstCtrlPoint(), new CtrlPointChecker_select( SELECT_BLUE ) );
			else
				cpiter = new CtrlPointIterator( NULL ); // arrrgh

			movetool_i->startMoveCycle( cpiter, snap3 );
		}

		if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
		{
			movetool_i->dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
		{
			// recalc bb
			CPolyIterator	csi( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
			CPoly		*cs;
			csi.reset();
			cs = csi.getNext();
			if ( cs )
				cs->calcBB();
			else
				printf( "WARNING: internal inconcistancy. can't finish move cycle.\n" );

			movetool_i->finishMoveCycle( snap3 );
		}

	}
	//
	// cursor3d is mode independet
	//

	// origin
	if ( type == 0 && buttonstate == LeftButton )
	{
		printf( "set cursor\n" );
		cursor3d_i->setX( snap[0] );
		cursor3d_i->setZ( snap[1] );

		cursor3d_i->get( v );
		sprintf( text, "Cursor: %f %f %f", 
			 v[0],
			 v[1],
			 v[2] );
		printComment( text );

		xzview_i->drawSelf();
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();
	}

	// to
	if ( type == 0 && buttonstate == MidButton )
	{
		cursor3d_i->setXTo( snap[0] );
		cursor3d_i->setZTo( snap[1] );

		xzview_i->drawSelf();
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();		
	}
#if 0	
	// checker is mode independent
	if ( type == 0 && buttonstate == (LeftButton | ShiftButton ) ) {
		zchecker = snap[1];
		xzview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		updateViews();
	}       
#endif

#if 0	
	// camera is mode independent
	if ( type == 0 && buttonstate == ( MidButton | ShiftButton ) ) {
		cameraorigin[0] = snap[0];
		cameraorigin[1] = getYChecker();
		cameraorigin[2] = snap[1];
		cameraview_i->setCamera( cameraorigin, cameralookat );		
		xzview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		updateViews();
		sprintf( text, "Camera origin at: %f %f %f", 
			 cameraorigin[0],
			 cameraorigin[1],
			 cameraorigin[2] );
		printComment( text );
	}
	if ( type == 2 && buttonstate == ( RightButton | ShiftButton ) ) {
		cameralookat[0] = snap[0];
		cameralookat[1] = getYChecker();
		cameralookat[2] = snap[1];
		cameraview_i->setCamera( cameraorigin, cameralookat );
		xzview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		updateViews();
	}
#endif	

	// finish edit cycle
	if ( type == 2 )
		buttonstate = 0;
	
}

void Wired::yMouseEventSlot( int type, int state, Vec2 _snap, Vec2 _pos )
{
	char		text[256];
	vec3d_t		snap3;
	vec3d_t		v;

	vec2d_t snap;
	vec2d_t pos;

	_snap.get(snap);
	_pos.get(pos);

	printf("Wired::yMouseEventSlot\n");
	printf(" type = %d\n", type );
	printf(" state = %d\n", state );
	Vec2dPrint( pos );

	snap3[0] = snap[0];
	snap3[1] = snap[1];
	snap3[2] = 0.0;

	// start edit-cycle ( press-drag-release ), the button state is kept ( shift-alt-ctrl )
	// to avoid odd behavior if button state changes during a cycle
	if ( type == 0 )
		buttonstate = state;


	if ( editmode == EM_BRUSH ) {

		// all submodes can select
		if ( type == 0 && buttonstate == RightButton )
		{
//			selecttool_i -> pressSlot( pos );
			vec3d_t		start, dir;
			start[0] = pos[0];
			start[1] = pos[1];
			start[2] = 8000.0;
			dir[0] = 0.0;
			dir[1] = 0.0;
			dir[2] = -1.0;
			doSelectBrush( start, dir );
		}

		if ( submode_brush == EM_BRUSH_NORMAL )
		{

			
			if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> yPressSlot( snap, SELECT_BLUE );
			
			if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> yDragSlot( snap, SELECT_BLUE  );
			
			if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
				movetool_i -> yReleaseSlot( snap, SELECT_BLUE  );
			
			if ( type == 0 && buttonstate == (LeftButton | ControlButton | ShiftButton ) )
				movetool_i -> yPressSlot( snap, SELECT_BLUE | SELECT_RED  );
			
			if ( type == 1 && buttonstate == (LeftButton | ControlButton  | ShiftButton) )
				movetool_i -> yDragSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 2 && buttonstate == (LeftButton | ControlButton | ShiftButton ) )
				movetool_i -> yReleaseSlot( snap, SELECT_BLUE | SELECT_RED );
			
			
			if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_yPressSlot( snap, SELECT_BLUE );
			
			if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_yDragSlot( snap, SELECT_BLUE );
			
			if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
				movetool_i -> Face_yReleaseSlot( snap, SELECT_BLUE );
			
			if ( type == 0 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_yPressSlot( snap, SELECT_BLUE | SELECT_RED );
			
			if ( type == 1 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_yDragSlot( snap, SELECT_BLUE  | SELECT_RED);
			
			if ( type == 2 && buttonstate == ( MidButton | ControlButton | ShiftButton ) )
				movetool_i -> Face_yReleaseSlot( snap, SELECT_BLUE | SELECT_RED );
		}
	
#if 0
		if ( submode_brush == EM_BRUSH_CLIPPER ) 
		{
			if ( type == 0 && buttonstate == LeftButton )
				clippertool_i -> yPressSlot( snap );
		}
#endif
		if ( submode_brush == EM_BRUSH_NORMAL ) 
		{
			if ( type == 0 && buttonstate == (LeftButton | ShiftButton ) )
				clippertool_i -> yPressSlot( snap );
		}
	}
	
	else if ( editmode == EM_ARCHETYPE ) 
	{
		if ( type == 0 && buttonstate == RightButton )
		{
//			atbrowser_i -> SelectSlot( pos );
			vec3d_t		start, dir;
			start[0] = pos[0];
			start[1] = pos[1];
			start[2] = 8000.0;
			dir[0] = 0.0;
			dir[1] = 0.0;
			dir[2] = -1.0;
			doSelectArche( start, dir );
		}

		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> pressSlot( snap3 );

		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> dragSlot( snap3 );

		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
			atbrowser_i -> releaseSlot( snap3 );
	}
	else if ( editmode == EM_TESTBOX )
	{
		
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], pos[1], 8000.0 );
			Vec3dInit( dir, 0.0, 0.0, -1.0 );
			doSelectTestBox( start, dir );
		}

		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			TestBoxIterator		*iter;
			iter = new TestBoxIterator( wwm_i->getFirstTestBox(), new TestBoxChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}
		
		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}		
	}
	else if ( editmode == EM_CSURFACE )
	{
		//
		// CSurface select
		//
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], pos[1], 8000.0 );
			Vec3dInit( dir, 0.0, 0.0, -1.0 );
			doSelectCSurface( start, dir );
		}

		//
		// CSurface move
		//
		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			CSurfaceIterator		*iter;
			iter = new CSurfaceIterator( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}
		
		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}				

		// CtrlPoints
		if ( type == 0 && buttonstate == ( RightButton | ControlButton ) )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], pos[1], 8000.0 );
			Vec3dInit( dir, 0.0, 0.0, -1.0 );
			doSelectCSurfaceCtrlPoint( start, dir );
		}

		if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
		{
			// search blue CSurface
			CSurfaceIterator	csiter( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
			CSurface		*cs;
			
			csiter.reset();
			cs = csiter.getNext(); // there should be only one blue
			
			// get CtrlPointIterator
			CtrlPointIterator	*cpiter;
			if ( cs )
				cpiter = new CtrlPointIterator( cs->getFirstCtrlPoint(), new CtrlPointChecker_select( SELECT_BLUE ) );
			else
				cpiter = new CtrlPointIterator( NULL ); // arrrgh

			movetool_i->startMoveCycle( cpiter, snap3 );
		}

		if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
		{
			movetool_i->dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
		{
			// recalc bb
			CSurfaceIterator	csi( wwm_i->getFirstCSurface(), new CSurfaceChecker_select( SELECT_BLUE ) );
			CSurface		*cs;
			csi.reset();
			cs = csi.getNext();
			if ( cs )
				cs->calcBB();
			else
				printf( "WARNING: internal inconcistancy. can't finish move cycle.\n" );

			movetool_i->finishMoveCycle( snap3 );
		}
	}
	else if ( editmode == EM_CPOLY )
	{
		//
		// CPoly select
		//
		if ( type == 0 && buttonstate == RightButton )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], pos[1], 8000.0 );
			Vec3dInit( dir, 0.0, 0.0, -1.0 );
			doSelectCPoly( start, dir );
		}

		//
		// CPoly move
		//
		if ( type == 0 && buttonstate == (LeftButton | ControlButton ) )
		{
			CPolyIterator		*iter;
			iter = new CPolyIterator( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
			movetool_i -> startMoveCycle( iter, snap3 );
		}
		
		if ( type == 1 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == (LeftButton | ControlButton ) )
		{
			movetool_i -> finishMoveCycle( snap3 );
		}				
		
		//
		// CPoly CtrlPoint select
		//
		if ( type == 0 && buttonstate == ( RightButton | ControlButton ) )
		{
			vec3d_t		start, dir;
			Vec3dInit( start, pos[0], pos[1], 8000.0 );
			Vec3dInit( dir, 0.0, 0.0, -1.0 );
			doSelectCPolyCtrlPoint( start, dir );
		}

		//
		// CtrlPoint move
		//
		if ( type == 0 && buttonstate == ( MidButton | ControlButton ) )
		{
			// search blue CPoly
			CPolyIterator	csiter( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
			CPoly		*cs;
			
			csiter.reset();
			cs = csiter.getNext(); // there should be only one blue
			
			// get CtrlPointIterator
			CtrlPointIterator	*cpiter;
			if ( cs )
				cpiter = new CtrlPointIterator( cs->getFirstCtrlPoint(), new CtrlPointChecker_select( SELECT_BLUE ) );
			else
				cpiter = new CtrlPointIterator( NULL ); // arrrgh

			movetool_i->startMoveCycle( cpiter, snap3 );
		}

		if ( type == 1 && buttonstate == ( MidButton | ControlButton ) )
		{
			movetool_i->dragMoveCycle( snap3 );
		}
		
		if ( type == 2 && buttonstate == ( MidButton | ControlButton ) )
		{
			// recalc bb
			CPolyIterator	csi( wwm_i->getFirstCPoly(), new CPolyChecker_select( SELECT_BLUE ) );
			CPoly		*cs;
			csi.reset();
			cs = csi.getNext();
			if ( cs )
				cs->calcBB();
			else
				printf( "WARNING: internal inconcistancy. can't finish move cycle.\n" );

			movetool_i->finishMoveCycle( snap3 );
		}

	}	

	//
	// cursor3d is mode independet
	//

	// origin
	if ( type == 0 && buttonstate == LeftButton )
	{
		printf( "set cursor\n" );
		cursor3d_i->setX( snap[0] );
		cursor3d_i->setY( snap[1] );

		cursor3d_i->get( v );
		sprintf( text, "Cursor: %f %f %f", 
			 v[0],
			 v[1],
			 v[2] );
		printComment( text );

		xzview_i->drawSelf();
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();
		
	}

	// to
	if ( type == 0 && buttonstate == MidButton )
	{
		cursor3d_i->setXTo( snap[0] );
		cursor3d_i->setYTo( snap[1] );

		xzview_i->drawSelf();
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		wired_i->updateViews();
		wired_i->drawSelf();		
	}

#if 0
	// checker is mode independent
	if ( type == 0 && buttonstate == (LeftButton | ShiftButton ) ) {
		ychecker = snap[1];
		yview_i->drawSelf();
		wwm_i->allUpdateFlagsTrue();
		updateViews();
	}
#endif
}

void Wired::cameraRaySlot( int type, int state, Vec3 _from, Vec3 _to )	// to = dir
{
	vec3d_t from;
	vec3d_t to;

	_from.get(from);
	_to.get(to);

	printf("Wired::cameraRaySlot\n");
	printf(" type = %d\n", type );
	printf(" state = %d\n", state );
	Vec3dPrint( from );
	Vec3dPrint( to );

	if ( type == 0 )
		buttonstate = state;

	if ( editmode == EM_BRUSH )
	{
		if ( buttonstate == ( LeftButton | ControlButton ) )
		{
			// the ray selects a single brush
			printf( "ray select\n" );
			doSelectBrush( from, to );
		}
		
		if ( submode_brush == EM_BRUSH_NORMAL && buttonstate == ( LeftButton ) )
		{
			// copy current texdef to ray face
//		texdef_i->currentTexDefToRayFaceSlot( from, to );
			doGetBrushSettingWdg( from, to );
		}

		if ( submode_brush == EM_BRUSH_EXTRUDE && buttonstate == ( LeftButton ) )
		{
			doExtrudeBrushFromFace( from, to );
		}

		if ( submode_brush == EM_BRUSH_FACESCALE && buttonstate == ( LeftButton ) )
		{
			doScaleFace( from, to, 10.0 );
		}

		if ( submode_brush == EM_BRUSH_FACESCALE && buttonstate == ( RightButton ) )
		{
			doScaleFace( from, to, -10.0 );
		}
		
		if ( submode_brush == EM_BRUSH_NORMAL && buttonstate == ( RightButton ) )
		{
			doSetBrushSettingWdg( from, to );
		}
		
		if ( submode_brush == EM_BRUSH_NORMAL && buttonstate == ( RightButton | ControlButton ) )
		{
			doSetSplitPlaneFromFace( from, to );
		}
	}

	else if ( editmode == EM_CSURFACE )
	{
		if ( buttonstate == ( RightButton | ControlButton ) )
		{
			// create csurface from face
			doCreateCSurfaceFromFace( from, to );
		}
	}

	else if ( editmode == EM_CPOLY )
	{
		if ( buttonstate == ( RightButton | ControlButton ) )
		{
			// create cpoly from face
			doCreateCPolyFromFace( from, to );
		}

		if ( buttonstate == ( RightButton ) )
		{
			doSetBrushSettingWdg_CPoly( from, to );
		}

		if ( buttonstate == ( LeftButton ) )
		{
			doGetBrushSettingWdg_CPoly( from, to );
		}
	}	
}


void Wired::newWireProject( const char *arg_wirename )
{
	char	tmp[256];	
	char	*ptr;
	char	relname[256];
	
	strcpy( wirename, arg_wirename );
	strcpy( ( char * )tmp, ( const char * )wiresavedir->path() );

	printf( "new project: dir: %s wire: %s\n", tmp, wirename );

	// make project subdir
	
	if( wirename[0] == '/' )
	{
		ptr = strrchr( wirename, '/' );
		strcpy( relname, ptr + 1 );  // get all after last /
	} else
		strcpy( relname, wirename );
		
	ptr = strstr( relname, ".wire" ); // search suffix
	if( !ptr )
	{
		WOSMessage( "wirename %s is not valid.\n( .wire not found )", relname );
		return;
	}

	strcpy( ptr, ".d" ); // replace it
	printf( "project subdir will be: %s\n", relname );
					
	int r = chdir( tmp ); // go to where the project lies
	if ( r != 0 ) {
		Error("chdir '%s' failed\n", tmp);
	}
	mkdir( relname, 511 ); // make subdir from it. 

	strcpy( prodir, relname );
	strcpy( outdir, relname );
	printf( "%s %s\n", prodir, outdir );
}
	
static int exists(const char *fname)
{
    FILE *file = fopen(fname, "r");
    if ( file )
    {
        fclose(file);
        return 1;
    }
    return 0;
}
	
void Wired::loadWire( const char *arg_wirename )
{
	tokenstream_t	*ts;
	char	caption[256];	

	char	tmp[256];
	

	if( !arg_wirename )
	{
		printf( "null pointer\n" );
		return;
	}
	strcpy( wirename, arg_wirename );
	strcpy( ( char * )tmp, ( const char * )wireloaddir->path() );

	printf( ".wire path: %s, name: %s\n", tmp, wirename );

	// .wire file parsing

	int r = chdir( tmp );
	if ( r != 0 ) {
		Error( "chdir '%s' failed\n", tmp );
	}

	ts = BeginTokenStream( wirename );
	GetToken( ts );
	if( strlen( ts->token ) > 255 )
	{
		WOSMessage( "file %s broken.\nfirst token > 255\ncannot open file\n" );
		EndTokenStream( ts );
		return;
	}
	strcpy( prodir, ts->token );
	
	GetToken( ts );
	if( strlen( ts->token ) > 255 )
	{
		WOSMessage( "file %s broken.\nsecond token > 255\ncannot open file\n" );
		EndTokenStream( ts );
		return;
	}
	strcpy( outdir, ts->token );
	
	EndTokenStream( ts );
	

	
	delete wwm;
	wwm = new WWM();

	sprintf( tmp, "%s/ids", prodir );
	wwm->loadID( ( const char *)tmp );

	sprintf( tmp, "%s/sbrush", prodir );
	wwm->loadWire( ( const char * )tmp );
	
	sprintf( tmp, "%s/ats.hobj", prodir );
	bool ats_class_file_exists = exists( tmp );
	if ( ats_class_file_exists ) {
		wwm->loadArcheClass( ( const char * ) tmp );
	}
	
	sprintf( tmp, "%s/csurfaces.hobj", prodir );
	wwm->loadCSurfaceClass( ( const char * ) tmp );

	sprintf( tmp, "%s/cpolys.hobj", prodir );
	wwm->loadCPolyClass( ( const char * ) tmp );

	wwm->validateAllIDs();

	if ( !ats_class_file_exists ) {
		// mcb 2012-10-28: try old ats file as fallback
		sprintf( tmp, "%s/ats", prodir );
		wwm->loadArcheFromOldAts( ( const char * ) tmp );		
	}
	
	if( strlen( wirename ) < 128 )
	{
		sprintf( caption, "WirED - %s", wirename );	
		setCaption( caption );
	}
//	memcpy( &g_project, &project, sizeof( w_project_t ));
}
			
	
void Wired::saveWire( const char *arg_wirename )
{
	FILE	*h;
	
	char	tmp[256];

	strcpy( tmp, ( const char * ) wiresavedir->path());
	printf("wiresavedir: %s\n", tmp );
	int r = chdir( tmp );

//	char cwd[256];
//	getcwd( cwd, 255 );
//	printf("cwd: %s\n", cwd);

	if ( r != 0 ) {
		WOSMessage( "chdir '%s' failed\n", wiresavedir );
		return;
	}

	if( !arg_wirename )
	{
		printf( "null pointer\n" );
		return;
	}
	if( arg_wirename[0] != '/' )
	{
		WOSMessage( "not an absolute path: %s\n", arg_wirename );
		return;
	}
	h = fopen( arg_wirename, "wb" );
	if( !h )
	{
		WOSMessage( "cannot open file %s\n", arg_wirename );
		return;
	}

	wwm->validateAllIDs();
	
	printf( "prodir: %s, outdir: %s\n", prodir, outdir );
	
	fprintf( h, "%s\n", prodir );
	fprintf( h, "%s\n", outdir );

	sprintf( tmp, "%s/ids", prodir );
	wwm->saveID( tmp );
	
	sprintf( tmp, "%s/sbrush", prodir );
	wwm->saveWire( tmp );
	
	sprintf( tmp, "%s/ats.hobj", prodir );
	wwm->saveArcheClass( tmp );

	sprintf( tmp, "%s/csurfaces.hobj", prodir );
	wwm->saveCSurfaceClass( tmp );

	sprintf( tmp, "%s/cpolys.hobj", prodir );
	wwm->saveCPolyClass( tmp );
	
	fclose( h );

	// write standard makefile, using template from MTemplate.hh

	sprintf( tmp, "%s/Makefile", prodir );
	h = fopen( tmp, "w" );
	fwrite( TMakefile, strlen( TMakefile ), 1, h );
	fclose( h );

	printf( "prodir: %s\n", prodir );
}

// FILE POPUP

void Wired::guiFileNewSlot()
{
	const char	*filetypes[3] = {
		"Wired project files (*.wire)",
		"All files (*)",
		NULL
	};

	QString		qstr_wirename;
	//QString		wirename_tmp( 256 );
	QFileDialog*	dialog;


	dialog = new QFileDialog( NULL, NULL, TRUE );

	dialog->setFilters( filetypes );
	dialog->setDir( *wiresavedir );
	dialog->rereadDir();

	dialog->exec();

	qstr_wirename = dialog->selectedFile();
	wiresavedir = new QDir( *dialog->dir() );

	if( !qstr_wirename.isEmpty() )
	{
		newWireProject( ( const char* )qstr_wirename );
		setCaption( ( const char* )qstr_wirename );
		strcpy( wirename, ( const char* )qstr_wirename );
	}
}	

void Wired::guiFileSlot()
{
	QRect	r;
	
	r = geometry();
//	printf("Wired::guiFileSlot.\n");
	qpm_filepopup->move( r.x()+qpb_file->x()+qpb_file->width(), r.y()+qpb_file->y() );
	qpm_filepopup->show();

//	guiFileQuitSlot();

}



void Wired::guiFileQuitSlot()
{
//	printf("Wired::guiFileQuitSlot.\n");

	//delete wwm_i;

	//saveConfig();
	//QApplication::exit(0);
	WOSQuit();
}

void Wired::guiFileOpenSlot()
{
	const char	*filetypes[3] = {
		"Wired project files (*.wire)",
		"All files (*)",
		NULL
	};

	QString		qstr_wirename;
	//QString		wirename_tmp( 256 );
	QFileDialog	*dialog;

	dialog = new QFileDialog( NULL, NULL, TRUE );

	dialog->setFilters( filetypes );
	dialog->setDir( *wireloaddir );
	dialog->rereadDir();

	dialog->exec();

	qstr_wirename = dialog->selectedFile();
	wireloaddir = new QDir( *dialog->dir() );

	if( !qstr_wirename.isEmpty() )
	{
		loadWire( ( const char* )qstr_wirename );
	}
//	delete dialog;
}

void Wired::guiFileSaveSlot()
{
	printf( "save wire %s\n", wirename );
	if( !wirename[0] )
	{
		printf( "no projectname\n" );
		return;
	}
	
	wiresavedir = new QDir( *wireloaddir );

	saveWire( wirename );
}

void Wired::guiFileSaveAsSlot()
{
	const char	*filetypes[3] = {
		"Wired project files (*.wire)",
		"All files (*)",
		NULL
	};
	QString		qstr_wirename;
	//QString		wirename_tmp( 256 );
	QFileDialog*	dialog;


	dialog = new QFileDialog( NULL, NULL, TRUE );
	dialog->setMode(QFileDialog::AnyFile);
	dialog->setFilters( filetypes );
	dialog->setDir( *wiresavedir );
	dialog->rereadDir();

	dialog->exec();

	qstr_wirename = dialog->selectedFile();
	wiresavedir = new QDir( *dialog->dir() );

	if( !qstr_wirename.isEmpty() )
	{
		saveWire( ( const char* )qstr_wirename );
		setCaption( ( const char* )qstr_wirename );
		strcpy( wirename, ( const char* )qstr_wirename );
	}
}	

void Wired::autoSaveSlot()
{
	char	tmp[256];

	if( !strlen( wirename ) )
	{
		qlb_comment->setText( "autosave: no wirename" );
		return;
	}

	sprintf( tmp, "%s/sbrush.auto", prodir );
	wwm->saveWire( tmp );
	
	sprintf( tmp, "%s/ats.hobj.auto", prodir );
	wwm->saveArcheClass( tmp );

	qlb_comment->setText( "autosaved." );
}



void Wired::guiZoomChangedSlot( int index )
{

	zoomindex = index;
	zoomChanged();

}

void Wired::guiGridChangedSlot( int index )
{
	gridindex = index;
	gridChanged();

}


// wos
void Wired::WOSMessage( const char* format, ... )
{
	va_list		args;
	char		text[512];
	//QMessageBox	*mbox;

	va_start( args, format );
	vsprintf( text, format, args );
	va_end( args );
	QMessageBox::information( this, "Wired message:", text );
}

void Wired::WOSError( const char *format, ... )
{
	va_list		args;
	char		text[512];

	va_start( args, format );
        vsprintf( text, format, args );
        va_end( args );	

	switch( QMessageBox::critical( this, "Wired critial error:", text, 
					"Quit", "Save as ..." ))
	{
	case 0:
		WOSQuit();
		break;

	case 1:
		guiFileSaveAsSlot();
		break;
	}
}

void Wired::WOSQuit()
{
	printf( "quitting...\n" );
	delete wwm_i;

	saveConfig();
	QApplication::exit(0);
}
