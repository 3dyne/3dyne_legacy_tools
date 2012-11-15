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



#ifndef __TEXTUREDLG_HH_INCLUDED
#define __TEXTUREDLG_HH_INCLUDED

#include <qdialog.h>
#include <qtabdialog.h>
#include <qgridview.h>
#include <qcombobox.h>
#include <qfont.h>

class TextureBrowser;

#include "Wired.hh"
#include "arr.h"
#include "pal.h"



class PageEntry
{
public:
	PageEntry( char*, char* );
	~PageEntry();
	int loadArr( pal_t* );

	QPixmap*	pixmap;
	pal_t*		pal;
	char		arrname[256];
	char		myname[32];
	bool		havearr;
	bool		isanim;
	bool	iserror;
	int		width, height;
	int		origx, origy;
};

class TDPage : public QGridView
{
	Q_OBJECT
public:
	TDPage( char* page_name, QWidget* parent = NULL, const char* name = NULL );
	~TDPage();
	void addEntry( char*, char* );
	void highlightEntry( char* );
	char* pageName();

protected:
	void paintCell( QPainter*, int, int );
	void mousePressEvent( QMouseEvent* );
	void mouseReleaseEvent( QMouseEvent* );
	void mouseDoubleClickEvent( QMouseEvent* );

private:
	int indexOf( int, int );
	char		myname[256];
	int		entrycount;
	int		curcol;
	int		currow;
	int		actrow, actcol;
	int		colnum;
	pal_t*		pal;
	PageEntry*	entries[1000];

signals:
	void acceptSignal();
	void clickSignal();

};

struct  bpage_t {
	TDPage*		widget;
	char		name[255];
};

class TextureBrowser : public QWidget
{
	Q_OBJECT
public:
	TextureBrowser( QWidget* parent = NULL, const char* name = NULL );
	~TextureBrowser();
	void readArrs();
	//void addArr( char* );
	char*	setActIdent( char* );
	void	addPage( QWidget*, char* );

private:
	int	pagenum;
	int	oldindex;
	bpage_t	pagelist[100];
	QComboBox*	cbox;

public slots:
	void cboxActivatedSlot( int );
	void acceptSlot();
	void clickSlot();

signals:
	void acceptSignal( char* );
	void clickSignal( char* );
 
protected:
	void resizeEvent( QResizeEvent* );
};

#endif
