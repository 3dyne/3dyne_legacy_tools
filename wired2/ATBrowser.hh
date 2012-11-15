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



// ATBrowser.hh

#ifndef __ATBrowser
#define __ATBrowser

#include <qaccel.h>
#include <qwidget.h>
#include <qframe.h>
#include <qlabel.h>
#include <qpushbutton.h>
#include <qcombobox.h>
#include <qlistbox.h>
#include <qlineedit.h>

#include "archetype.h"
#include "VecMath.hh"

class ATBrowser;
extern ATBrowser		*atbrowser_i;

/*
  ========================================
  class: ATBrowser

  aka the archetype editor
  ========================================
*/

class ATBrowser : public QWidget
{
	Q_OBJECT

public:
	ATBrowser( QWidget *parent = 0, const char *name = 0 );
	virtual ~ATBrowser();
	void initTemplates();
	void drawSelf();

	void setCurrent( arche_t *arche );	// set focus on this archetype
	void updateCurrent( void );		// redraw pair list
	void setCurrentPair( kvpair_t *pair );	// set focus on this pair
	void applyCurrentPair( void );		// apply changes in pair to current archetype
	void deleteCurrentPair( void );
	void changeCurrentValueVec3d( vec3d_t v );	// override current value by this one	

	arche_t * getCurrentArche( void );	// USE WITH CARE, it's a fake for easy getting the current selection

	void	setArche( arche_t *arche );
	void	getArche( arche_t *arche );
       	
	arche_t* createArche( vec3d_t );	// empty -> current -> wwm.add
	void copyArche( void );		// current -> wwm.add
	void deleteArche( void );	// current -> wwm.remove

	// link wdg
	void	getLinkFromCurrent( void );
	void	setLinkOfCurrent( void );
	
public slots:
	void xzSelectSlot( Vec2 );	// xz press: select -> current
	void pressSlot( Vec3 );
	void dragSlot( Vec3 );
	void releaseSlot( Vec3 );
	void yPressSlot( Vec2 );
	void yDragSlot( Vec2 );
	void yReleaseSlot( Vec2 );



private slots:
	void	templateClickedSlot( int id );
	void	listClickedSlot( int id );
	void	applySlot();  
	void	deleteSlot();

private:
	void    moveArches();

protected:
	virtual void		resizeEvent( QResizeEvent * );	
	virtual void		enterEvent( QEvent * );
	virtual void		leaveEvent( QEvent * );

private:
	QComboBox		*qcb_attemplates;
	QListBox		*qlb_atcurrent;
	
	QLineEdit		*qle_type;
	QLabel			*ql_type;

	QLineEdit		*qle_key;
	QLabel			*ql_key;

	QLineEdit		*qle_value;
	QLabel			*ql_value;
	
	QPushButton		*qpb_apply;
	QPushButton		*qpb_add;
	QPushButton		*qpb_delete;

	QLineEdit		*qle_linkkey;
	QLabel			*ql_linkkey;
	QLineEdit		*qle_linkvalue;
	QLabel			*ql_linkvalue;

	// current archetype
	arche_t			*currentarche;

	// current pair ( copy )
	kvpair_t		currentpair;

	// archetype template list
	arche_t			*templates;
	arche_t			*currenttemplate;

	// selection stuff
	int		archenum;
	int		arche;
	arche_t		*singlearche;
	arche_t		*arches[100];
	int		laststatus;

	// move stuff
	vec3d_t		from;
	vec3d_t		delta;
	
};

#endif
