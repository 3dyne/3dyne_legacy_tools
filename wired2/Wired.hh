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



// Wired.hh

#ifndef __Wired_included
#define __Wired_included

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <unistd.h>

#include <qwidget.h>
#include <qbutton.h>
#include <qpushbutton.h>
#include <qpopupmenu.h>
#include <qdir.h>
#include <qkeycode.h>
#include <qcombobox.h>
#include <qaccel.h>
#include <qpixmap.h>
#include <qlabel.h>
#include <qtimer.h>
#include <qsplitter.h>

#include "VecMath.hh"

class Wired;
class Cursor3d;
class BrushSettingWdg;
class BrushTool;
class ClipBoard;
class Draw3d;

//
// global instances
//
extern Wired		*wired_i;
extern BrushSettingWdg	*brushsetting_wdg_i;
extern BrushTool	*brushtool_i;
extern Cursor3d		*cursor3d_i;
extern ClipBoard	*clipboard_i;
extern Draw3d		*draw3d_i;

#include "Customize.hh"

#include "WWM.hh"
#include "XZView.hh"
#include "YView.hh"
#include "CameraView.hh"
#include "TexBrowser.hh"
#include "MoveTool.hh"
#include "ClipperTool.hh"
#include "ATBrowser.hh"
#include "wire.h"
#include "BrushEdit.hh"

#include "EditAble.hh"
#include "TestBox.hh"
#include "lib_mesh.h"

#include "Draw3d.hh"

/*
  =============================================================================
  class: Cursor3d
  =============================================================================
*/

class Cursor3d
{
public:
	Cursor3d();
	~Cursor3d();

	void	drawSelf( void );
	
	void	setX( float );
	void	setY( float );
	void	setZ( float );

	void	setXTo( float );
	void	setYTo( float );
	void	setZTo( float );

	float	getX( void );
	float	getY( void );
	float	getZ( void );

	void	get( vec3d_t );		// return origin
	void	getTo( vec3d_t );	// return to
	void	getDelta( vec3d_t );	// return to-origin
	void	getDir( vec3d_t );	// return Vec3dUnify( to - origin );

private:
	vec3d_t		origin;
	vec3d_t		to;
};

/*
  ==================================================
  class: ClipBoard

  hold copies
  and make copies
  ==================================================
*/
class ClipBoard
{
public:
	ClipBoard();
	~ClipBoard();

	void		copyBrush( brush_t *b );
	brush_t*	pasteBrush( void );

private:
	brush_t		*currentbrush;
	
};


// edit modes
#define EM_BRUSH	( 0 )
//#define EM_CLIPPER	( 1 )
#define EM_ARCHETYPE	( 2 )
#define EM_TESTBOX	( 3 )
#define EM_CSURFACE	( 4 )
#define EM_CPOLY	( 5 )

// brush mode tools
#define EM_BRUSH_NORMAL		( 0 )
#define EM_BRUSH_CLIPPER	( 1 )
#define EM_BRUSH_EXTRUDE	( 2 )
#define EM_BRUSH_FACESCALE	( 3 )

class Wired : public QWidget
{
	Q_OBJECT

public:
	Wired( QWidget* parent = NULL, char* name = NULL );
	virtual ~Wired();

	void		updateViews( void );
	void		redrawViews( void );

	void		layoutChanged( void );
	void		zoomChanged( void );
	void		gridChanged( void );
	void		setViewBounds( void );

	void		drawSelf( void ); // draws checker 
	void		printComment( const char * );
	void            changeEditMode( int mode );

	float		getZChecker( void );
	float		getYChecker( void );
	void		getCameraOrigin( vec3d_t );
	void		getCameraLookAt( vec3d_t );
//	void		getCurrentTexDef( texturedef_t *copyto ); // copies the currenttexdef to 'copyto'
       
	void		enableAccel( void );
	void		disableAccel( void );

	char*		getProjectDir( void ) { return prodir; }

// configuration
	void		loadConfig();
	void		saveConfig();

// project management
	void		loadWire( const char* );
	void		saveWire( const char* );
	void		newWireProject( const char* );
// wos
	void		WOSMessage( const char*, ... );
	void		WOSError( const char*, ... );
	void		WOSQuit();

protected:
	virtual void	resizeEvent ( QResizeEvent * );
	virtual void	paintEvent ( QPaintEvent * );

private:
	//
	// actions
	//
	void		doSwapWorld( void );
	void		doMoveWorld( vec3d_t dir );

	void		doSelectBrush( vec3d_t start, vec3d_t dir );
	void		doSetBrushSettingWdg( vec3d_t start, vec3d_t dir ); // from brush -> BrushSettingWdg
	void		doGetBrushSettingWdg( vec3d_t start, vec3d_t dir ); // from BrushSettingWdg -> face

	void		doSetSplitPlaneFromFace( vec3d_t start, vec3d_t dir ); // set split plane of clipper tool
	void		doExtrudeBrushFromFace( vec3d_t start, vec3d_t dir );
	void		doScaleFace( vec3d_t start, vec3d_t dir, float scale );

	// to the plane of the clicked face
	
	void		doRotateSplitPlane( float roll, float pitch, float yaw );
	
	void		doCreateBrush( void );
	void		doCopyBrush( void );
	void		doDeleteBrush( void );
	
	void		doCopyBrushToClipboard( void );
	void		doInsertBrushFromClipboard( void );
	
	void		doRotateBrush( float roll, float pitch, float yaw );
	void		doOverrideBrushContents( void );
	void		doOverrideSurfaceContents( void );

	void		doAddToRed( void );
	void		doAddToGreen( void );
	void		doAddToNormal( void );
	void		doAllAddToNormal( void );

	void		doApplyTexture( void );

	void		doSelectArche( vec3d_t start, vec3d_t dir );
	void		doCreateArche( void );
	void		doCopyArche( void );
	void		doDeleteArche( void );
	void		doGetUniqueNameForArche( void );

	// new EditAble stuff
	
	// TestBox stuff
	void		doCreateTestBox( void );
	void		doSelectTestBox( vec3d_t start, vec3d_t dir );
	void		doAddCtrlPointToTestBox( void );

	// CSurface stuff
	void		doCreateCSurfaceFromFace( vec3d_t start, vec3d_t dir );
	void		doSelectCSurface( vec3d_t start, vec3d_t dir );
	void		doSelectCSurfaceCtrlPoint( vec3d_t start, vec3d_t dir );
	void		doApplyTextureToCSurface( void );
	void		doApplyTexdefWdgToCSurface( void );
	void		doApplyCSurfaceToTexdefWdg( void );
	void		doRotateCSurface( float roll, float pitch, float yaw );
	void		doCopyCSurface( void );
	void		doDeleteCSurface( void );

	// CPoly stuff
	void		doCreateCPolyFromFace( vec3d_t start, vec3d_t dir );
	void		doSelectCPoly( vec3d_t start, vec3d_t dir );
	void		doSelectCPolyCtrlPoint( vec3d_t start, vec3d_t dir );
	void		doApplyTextureToCPoly( void );
	void		doApplyTexdefWdgToCPoly( void );
	void		doApplyCPolyToTexdefWdg( void );

	void		doRotateCPoly( float roll, float pitch, float yaw );
	void		doCopyCPoly( void );
	void		doDeleteCPoly( void );

	void		doCPolyPlaneFromClipper( void );
	void		doCPolyPlaneToClipper( void );

	void		doSetBrushSettingWdg_CPoly( vec3d_t start, vec3d_t dir ); // from cpoly -> BrushSettingWdg
	void		doGetBrushSettingWdg_CPoly( vec3d_t start, vec3d_t dir ); // from BrushSettingWdg -> cpoly


	// new EditAble draw stuff
	void		drawTestBox( TestBox * );
	void		drawCSurface( CSurface * );
	void		drawCPoly( CPoly * );
	void		drawBB( vec3d_t min, vec3d_t max );
	void		drawUVMesh( uvmesh_t *mesh );

private slots:
	// GUI
	void		guiFileSlot();
	void		guiFileQuitSlot();
	void		guiFileOpenSlot();
	void		guiFileSaveSlot();
	void		guiFileSaveAsSlot();
	void		guiFileNewSlot();
	
	

	void		guiZoomChangedSlot( int );
	void		guiGridChangedSlot( int );

	void		guiKeySlot( int );
	void		xzMouseEventSlot( int, int, Vec2, Vec2 );
	void		yMouseEventSlot( int, int, Vec2, Vec2 );
	void		cameraRaySlot( int, int, Vec3, Vec3 );

	void		xzviewOriginChangedSlot();
	void		xzviewPannerChangedSlot( int );
	void		yviewOriginChangedSlot();

	void		texBrwAcceptSlot( char * );
	void		CSTexDefChangedSlot( void );
	void		TexDefChangedSlot( void );

	void		paintThreadSlot();
	void		paintArcheTypeThreadSlot();

	void		paintArchetypeToLinks( arche_t *a );
	void		paintArchetypeFromLinks( arche_t *a );

	void		autoSaveSlot();

private:
	// GUI
	QPushButton	*qpb_file;
	QPushButton	*qpb_save;

	QPopupMenu	*qpm_filepopup;
	QComboBox	*qcb_zoom;
	QLabel		*qlb_zoom;
	QComboBox	*qcb_grid;
	QLabel		*qlb_grid;
	QComboBox	*qcb_rotstep;
	QLabel		*qlb_comment;
	QAccel*		accel;
	char		wirename[256];
	char		prodir[256];
	char		outdir[256];


	// paintthread
	QColor		*qc_brushblack;	// the colors are mode dependent / just names for diffrent selections
	QColor		*qc_brushblue;
	QColor		*qc_brushred;
	QColor		*qc_brushgreen;
	QColor		*qc_brushliquid; // shifted
	QColor		*qc_brushlocal;	// shifted
	QColor		*qc_brushdeco; // shifted
	QColor		*qc_arche;
	QTimer		*qt_paintthread;
	brush_t		*ptbrush;

	enum { UPDATE_START, UPDATE_NORMAL, UPDATE_SPECIAL, UPDATE_SELECT, UPDATE_FINISH } updatestate;

	int		stat_updatebrushnum;

	WWM			*wwm;
	XZView		*xzview;
	YView		*yview;
	CameraView	*cameraview;


	TextureBrowser	*texbrowser;
	MoveTool	*movetool;

	ClipperTool	*clippertool;
//	TexDef		*texdef;
	ATBrowser	*atbrowser;

	QSplitter	*qsp_editviews;

	vec3d_t		origin;		// 3d origin combined from XZView and YView
	float		zchecker;
	float		ychecker;
	vec3d_t		cameraorigin;
	vec3d_t		cameralookat;

	QDir*		wireloaddir;
	QDir*		wiresavedir;
	QTimer*		qt_autosave;


	vec3d_t		viewbounds[2];
	int		zoomindex;
	int		gridindex;
	int		rotstepindex;
	float		panratio;

	int		editmode;
	int		submode_brush;

	int		buttonstate;

	bool		xzswapflag; // false = original, true = xz-swap

	// fix me: this shouldn't happend here
	texturedef_t	currenttexdef;
	w_project_t	g_project;
};

#endif // __Wired_included
