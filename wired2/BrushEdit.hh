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



// BrushEdit.hh

#ifndef __BrushEdit
#define __BrushEdit

#include <qobject.h>
#include <qwidget.h>
#include <qlabel.h>
#include <qlayout.h>

#include <qbuttongroup.h>
#include <qcheckbox.h>
#include <qradiobutton.h>
#include <qpushbutton.h>
#include <qspinbox.h>

#include <qstring.h>

#include "texture.h"
#include "brush.h"

#include "Wired.hh"

#if 0
#define		BRUSH_CONTENTS_SOLID	( 16 )
#define		BRUSH_CONTENTS_LIQUID	( 8 )
#define		BRUSH_CONTENTS_HINT	( 4 )

#define		SURFACE_CONTENTS_OPEN		( 1 )
#define		SURFACE_CONTENTS_CLOSE		( 2 )
#define		SURFACE_CONTENTS_TEXTURE	( 4 )
#define		SURFACE_CONTENTS_WINDOW		( 8 )
#define		SURFACE_CONTENTS_SF_CLOSE	( 16 )
#endif 


typedef		texturedef_t texdef_t;

/*
  ==================================================
  BrushContentsWdg

  ==================================================
*/

class BrushContentsWdg : public QWidget
{
	Q_OBJECT

public:
	BrushContentsWdg( QWidget *parent = 0, const char *name = 0);
	virtual ~BrushContentsWdg();

	void		set( unsigned int arg_contents );
	unsigned int	get( void );

signals:
	void		changeSignal();

private slots:
	void		buttonGroupSlot( int id );


private:
	QButtonGroup	*group;
	unsigned int	contents;
};

/*
  ==================================================
  SurfaceContentsWdg

  ==================================================
*/
class SurfaceContentsWdg : public QWidget
{
	Q_OBJECT

public:
	SurfaceContentsWdg( QWidget *parent = 0, const char *name = 0);
	virtual ~SurfaceContentsWdg();

	void		set( unsigned int arg_contents );
	unsigned int	get( void );

signals:
	void		changeSignal();

private slots:
	void		buttonGroupSlot( int id );

private:
	QButtonGroup	*group;
	unsigned int	contents;

};

/*
  ==================================================
  TexDefWdg

  ==================================================
*/
class TexDefWdg : public QWidget
{
	Q_OBJECT

public:
	TexDefWdg( QWidget *parent = 0, const char *name = 0);
	virtual ~TexDefWdg();

	void	set( texdef_t *arg_td );
	void	get( texdef_t *arg_td );

private slots:
	void		anyChange( const QString& );

signals:
	void		changeSignal();

private:
	texdef_t	texdef;

	QLabel		*ident;
	QSpinBox	*rotate;
	QSpinBox	*scalex;
	QSpinBox	*scaley;
	QSpinBox	*shiftx;
	QSpinBox	*shifty;

};

/*
  ==================================================
  CSTexDefWdg

  ==================================================
*/
class CSTexDefWdg : public QWidget
{
	Q_OBJECT

public:
	CSTexDefWdg( QWidget *parent = 0, const char *name = 0 );
	virtual ~CSTexDefWdg();

	void		set( cstexdef_t *_td );
	void		get( cstexdef_t *_td );

private slots:
	void		anyChange( const QString & text );

signals:
	void		changeSignal();

private:
	cstexdef_t	td;
	QLabel		*ident;
	QSpinBox	*shiftu, *shiftv;
	QSpinBox	*scaleu, *scalev;
	QSpinBox	*u1, *v1;
	QSpinBox	*u2, *v2;
	QSpinBox	*u3, *v3;
	
};

/*
  ========================================
  BrushSettingWdg

  class
  ========================================
*/
class BrushSettingWdg : public QWidget
{
	Q_OBJECT

public:
	BrushSettingWdg( QWidget *parent = 0, const char *name = 0);
	~BrushSettingWdg();

	void		setBrushContents( unsigned int contents );
	unsigned int	getBrushContents( void );

	void		setSurfaceContents( unsigned int contents );
	unsigned int	getSurfaceContents( void );

	void		setTexDef( texdef_t *texdef );
	void		getTexDef( texdef_t *texdef );

	void		setCSTexDef( cstexdef_t *texdef );
	void		getCSTexDef( cstexdef_t *texdef );

	// these are called by brushContentsChangeSlot
	void		initSurfaceFromBrush( void );
	void		resetTexDef( void );


signals:
	void		CSTexDefChanged( void );
	void		TexDefChanged( void );

private slots: 
	void		brushContentsChangeSlot();
	void		CSTexDefChangedSlot();
	void		TexDefChangedSlot();

private:
	BrushContentsWdg	*bcw;
	SurfaceContentsWdg	*scw;
	TexDefWdg		*tdw;
	CSTexDefWdg		*cstdw;

};


/*
  =============================================================================
  BrushTool

  - new Brush from BrushSettingsWdg
  - new Faces from BrushSettingsWdg
  =============================================================================
*/
class BrushTool : public QObject
{
	Q_OBJECT

public:
	BrushTool( QObject * parent=0, const char * name=0 );
	~BrushTool();

	brush_t*	createBrush( float edgelength = 64 );
	face_t*		createFace( vec3d_t, float dist );

	void		moveBrush( brush_t *b, vec3d_t delta );

//	brush_t*	copyBrush( brush_t *b );
};


#endif
