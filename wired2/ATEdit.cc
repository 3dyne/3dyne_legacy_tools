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



// ATEdit.cc

#include "ATEdit.hh"

#include "qpushbutton.h"

ATEdit::ATEdit( QWidget *parent, const char *name )
	: QScrollView( parent, name, WPaintClever )
{

#if 0
	QPushButton	*b;
	QFrame		*f;

	f = new QFrame();
	for ( int j = 0; j < 10; j++ )
		for ( int i = 0; i < 20; i++ )
		{
			b = new QPushButton( f );
			b->setText( "button" );
			b->resize( 50, 15 );
			b->move( j*60, i*30 );
			b->show();
			
		}

	f->resize( 1024,1024 );
	addChild( f );
//	setVScrollBarMode( AlwaysOn );
//	setHScrollBarMode( AlwaysOn );

	show();
#endif
}

ATEdit::~ATEdit()
{

}
