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



// BrushEdit.cc

#include "Customize.hh"
#include "BrushEdit.hh"

/*
  ==================================================
  BrushContentsWdg
  Class
  ==================================================
*/

/*
  ====================
  BrushContentsWdg::BrushContentsWdg

  constructor

  ====================
*/
BrushContentsWdg::BrushContentsWdg( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	int		i;
	int		brushnum;

	//
	// toplevel layouter
	//
	QBoxLayout *toplevel = new QVBoxLayout( this, 5 );
	group = new QButtonGroup( "Brush", this ); 

	toplevel->addWidget( group );

	//
	// customize from c_brushes
	//
	for ( i = 0; c_brushes[i]; i++ );
	brushnum = i/3;
	printf( " c_brushes customization string length: %d => %d\n", i, brushnum );

		
	
	QGridLayout *grid = new QGridLayout( group, brushnum+2, 3, 5 );

	grid->addRowSpacing( 0, 10 );
	grid->addColSpacing( 0, 5 );
	grid->setRowStretch( brushnum+1, 5 );
	grid->setColStretch( 2, 5 );

	QRadioButton	*rb;

	for ( i = 0; i < brushnum; i++ )
	{
		rb = new QRadioButton( c_brushes[i*3], group );
		rb->setMinimumSize( rb->sizeHint() );
		grid->addWidget( rb, i+1, 1 );
	}
	
#if 0
	rb = new QRadioButton( "liquid", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 2, 1 );

	rb = new QRadioButton( "hint", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 3, 1 );
#endif

	grid->activate();
	toplevel->activate();

	connect( group, SIGNAL( clicked( int ) ), this, SLOT( buttonGroupSlot( int ) ) );

	QSize qs = group->minimumSize();
	printf( "min size:%d, %d\n", qs.width(), qs.height() );
}


/*
  ====================
  BrushContentsWdg::~BrushContentsWdg

  destructor

  ====================
*/
BrushContentsWdg::~BrushContentsWdg()
{

}


/*
  ====================
  BrushContentsWdg::set

  public
  ====================
*/
void BrushContentsWdg::set( unsigned int arg_contents )
{
	int		i, j;
	unsigned int	c;
	QRadioButton	*b;
	const char		*text;

	contents = arg_contents;

	printf( "BrushContentsWdg::set contents: %d\n", contents );

	for ( j = 0; c_brushes[j]; j+=3 )
	{
		c = atoi( c_brushes[j+1] );

		if ( c != contents )
			continue;
		
		for ( i = 0; ( b = ((QRadioButton *)group->find( i )) ); i++ )
		{
			text = b->text();
			if ( !strcmp(text, c_brushes[j] ) )
			{
				b->setChecked( TRUE );
			}
		}
	}

#if 0
	if ( contents == BRUSH_CONTENTS_SOLID )
		((QRadioButton *)group->find( 0 ))->setChecked( TRUE );

	if ( contents == BRUSH_CONTENTS_LIQUID )
		((QRadioButton *)group->find( 1 ))->setChecked( TRUE );

	if ( contents == BRUSH_CONTENTS_HINT )
		((QRadioButton *)group->find( 2 ))->setChecked( TRUE );
#endif
}


/*
  ====================
  BrushContentsWdg::get

  public
  ====================
*/
unsigned int BrushContentsWdg::get( void )
{
	contents = 0;
	QRadioButton	*b;
	int		i, j;
	const char		*text;

	for ( i = 0; ( b = ((QRadioButton *)group->find( i )) ); i++ )
	{
		if ( !b->isChecked() )
			continue;

		text = b->text();
		for ( j = 0; c_brushes[j]; j+=3 )
		{
			if ( !strcmp( text, c_brushes[j] ) )
			{
				contents = atoi( c_brushes[j+1] );
				return contents;
			}
		}
	}

#if 0
	if ( ((QRadioButton *)group->find( 0 ))->isChecked() )
		contents = BRUSH_CONTENTS_SOLID;

	if ( ((QRadioButton *)group->find( 1 ))->isChecked() )
		contents = BRUSH_CONTENTS_LIQUID;

	if ( ((QRadioButton *)group->find( 2 ))->isChecked() )
		contents = BRUSH_CONTENTS_HINT;
#endif

	return contents;
}

/*
  ====================
  BrushContentsWdg::buttonGroupSlot

  qt slot
  ====================
*/
void BrushContentsWdg::buttonGroupSlot( int /*id*/)
{
	printf( "contents: %d\n", this->get() );

	emit changeSignal();
}



/*
  ==================================================
  SurfaceContentsWdg
  Class
  ==================================================
*/

/*
  ====================
  SurfaceContentsWdg::SurfaceContentsWdg

  constructor
  ====================
*/  
SurfaceContentsWdg::SurfaceContentsWdg( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	int		i;
	int		surfacenum;	// from customize

	//
	// toplevel layouter
	//
	QBoxLayout *toplevel = new QVBoxLayout( this, 5 );
	group = new QButtonGroup( "Surface", this ); 

	toplevel->addWidget( group );

	//
	// init surfaces from customize *c_surfaces[]
	//

	for ( i = 0; c_surfaces[i]; i++ );
	surfacenum = i/2;
	printf( " c_surfaces customization string length: %d => %d\n", i, surfacenum );


	QGridLayout *grid = new QGridLayout( group, surfacenum+2, 3, 5 );

	grid->addRowSpacing( 0, 10 );
	grid->addColSpacing( 0, 5 );
	grid->setRowStretch( surfacenum+1, 5 );
	grid->setColStretch( 2, 5 );

	QCheckBox	*rb;

	for ( i = 0; i < surfacenum; i++ )
	{
		rb = new QCheckBox( c_surfaces[i*2], group );
		rb->setMinimumSize( rb->sizeHint() );
		grid->addWidget( rb, i+1, 1 );
	}

#if 0
	rb = new QCheckBox( "close", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 2, 1 );

	rb = new QCheckBox( "texture", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 3, 1 );

	rb = new QCheckBox( "window", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 4, 1 );

	rb = new QCheckBox( "sf_close", group );
	rb->setMinimumSize( rb->sizeHint() );
	grid->addWidget( rb, 5, 1 );
#endif

	grid->activate();
	toplevel->activate();

	connect( group, SIGNAL( clicked( int ) ), this, SLOT( buttonGroupSlot( int ) ) );

}


/*
  ====================
  SurfaceContentsWdg::~SurfaceContentsWdg

  destructor
  ====================
*/
SurfaceContentsWdg::~SurfaceContentsWdg()
{

}


/*
  ====================
  SurfaceContentsWdg::

  ====================
*/
void SurfaceContentsWdg::set( unsigned int arg_contents )
{
	int		i, j;
	unsigned int	c;
	QCheckBox	*b;
	const char		*text;

	contents = arg_contents;
	printf( "SurfaceContentsWdg::set contents: %d\n", contents );

	for ( j = 0; c_surfaces[j]; j+=2 )
	{
		c = atoi( c_surfaces[j+1] );

		for ( i = 0; ( b = ((QCheckBox *)group->find( i )) ); i++ )
		{
			text = b->text();
			if ( !strcmp( text, c_surfaces[j] ) )
				b->setChecked( contents&c );
		}				
	}

#if 0
	((QCheckBox *)group->find(0))->setChecked( contents & SURFACE_CONTENTS_OPEN );
	((QCheckBox *)group->find(1))->setChecked( contents & SURFACE_CONTENTS_CLOSE );
	((QCheckBox *)group->find(2))->setChecked( contents & SURFACE_CONTENTS_TEXTURE );
	((QCheckBox *)group->find(3))->setChecked( contents & SURFACE_CONTENTS_WINDOW );
	((QCheckBox *)group->find(4))->setChecked( contents & SURFACE_CONTENTS_SF_CLOSE );
#endif
}


/*
  ====================
  SurfaceContentsWdg::

  ====================
*/
unsigned int SurfaceContentsWdg::get( void )
{
	QCheckBox	*b;
	int		i, j;
	const char		*text;
	contents = 0;

	for ( i = 0; ( b = ((QCheckBox *)group->find(i)) ); i++ )
	{
		if ( !b->isChecked() )
			continue;

		text = b->text();
		for ( j = 0; c_surfaces[j]; j+=2 )
		{
			if ( !strcmp( text, c_surfaces[j] ) )
			{
				contents|=atoi( c_surfaces[j+1] );
			}
		}
	}

	
	return contents;

#if 0
	if ( ((QCheckBox *)group->find(0))->isChecked() )	
		contents |= SURFACE_CONTENTS_OPEN;

	if ( ((QCheckBox *)group->find(1))->isChecked() )	
		contents |= SURFACE_CONTENTS_CLOSE;

	if ( ((QCheckBox *)group->find(2))->isChecked() )	
		contents |= SURFACE_CONTENTS_TEXTURE;

	if ( ((QCheckBox *)group->find(3))->isChecked() )	
		contents |= SURFACE_CONTENTS_WINDOW;

	if ( ((QCheckBox *)group->find(4))->isChecked() )	
		contents |= SURFACE_CONTENTS_SF_CLOSE;

	return contents;
#endif
}


/*
  ====================
  SurfaceContentsWdg::buttonGroupSlot

  qt slot
  ====================
*/
void SurfaceContentsWdg::buttonGroupSlot( int /*id*/ )
{
	printf( "contents: %d\n", this->get() );
	emit changeSignal();
}


/*
  ==================================================
  TexDefWdg

  ==================================================
*/

/*
  ====================
  TexDefWdg::TexDefWdg

  constructor
  ====================
*/
TexDefWdg::TexDefWdg( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	//
	// toplevel layouter
	//
	QBoxLayout *toplevel = new QVBoxLayout( this, 5 );
	QGroupBox *group = new QGroupBox( "TexDef", this );

	toplevel->addWidget( group );


 	QGridLayout *grid = new QGridLayout( group, 8, 4, 5 );

	grid->addRowSpacing( 0, 10 );
	grid->addColSpacing( 0, 5 );
	grid->setRowStretch( 7, 5 );
	grid->setColStretch( 3, 5 );

 	QLabel *l;
	l = new QLabel( group );
	l->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	l->setLineWidth( 1 );
	l->setText( "default" );
	l->setMinimumSize( l->sizeHint() );
	grid->addMultiCellWidget( l, 1, 1, 1, 2 );
	ident = l;

	l = new QLabel( "rot", group );
	l->setMinimumSize( l->sizeHint() );
	grid->addWidget( l, 2, 1 );

 	l = new QLabel( "scx", group );
 	l->setMinimumSize( l->sizeHint() );	
 	grid->addWidget( l, 3, 1 );

 	l = new QLabel( "scy", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 4, 1 );


 	l = new QLabel( "shx", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 5, 1 );

 	l = new QLabel( "shy", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 6, 1 );


	QSpinBox *s;
	s = new QSpinBox( 0, 360, 15, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 2, 2 );
	rotate = s;

	s = new QSpinBox( -500, 500, 10, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 3, 2 );
	scalex = s;

	s = new QSpinBox( -500, 500, 10, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 4, 2 );
	scaley = s;

	s = new QSpinBox( 0, 256, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 5, 2 );
	shiftx = s;

	s = new QSpinBox( 0, 256, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 6, 2 );
	shifty = s;

 	grid->activate();
	
	toplevel->activate();

	connect( rotate, SIGNAL( valueChanged( const QString & ) ), this, SLOT( anyChange( const QString & ) ) );
	connect( scalex, SIGNAL( valueChanged( const QString & ) ), this, SLOT( anyChange( const QString & ) ) );
	connect( scaley, SIGNAL( valueChanged( const QString & ) ), this, SLOT( anyChange( const QString & ) ) );
	connect( shiftx, SIGNAL( valueChanged( const QString & ) ), this, SLOT( anyChange( const QString & ) ) );
	connect( shifty, SIGNAL( valueChanged( const QString & ) ), this, SLOT( anyChange( const QString & ) ) );
}

/*
  ====================
  TexDefWdg::~TexDefWdg

  destructor
  ====================
*/
TexDefWdg::~TexDefWdg()
{

}

void TexDefWdg::set( texdef_t *arg_td )
{
	memcpy( &texdef, arg_td, sizeof( texdef_t ) );

	ident->setText( texdef.ident );
	rotate->setValue( (int)texdef.rotate );
	scalex->setValue( (int)(texdef.scale[0] * 100.0) );
	scaley->setValue( (int)(texdef.scale[1] * 100.0) );
	shiftx->setValue( (int)texdef.shift[0] );
	shifty->setValue( (int)texdef.shift[1] );
}

void TexDefWdg::get( texdef_t *arg_td )
{
	
	texdef.rotate = (float)rotate->value();
	texdef.scale[0] = scalex->value() / 100.0;
	texdef.scale[1] = scaley->value() / 100.0;
	texdef.shift[0] = (float)shiftx->value();
	texdef.shift[1] = (float)shifty->value();

	memcpy( arg_td, &texdef, sizeof( texdef_t ) );
}

void TexDefWdg::anyChange( const QString &text )
{
	printf( "TexDefWdg::anyChange %s\n", text.latin1() );
	emit changeSignal();
}

/*
  ==================================================
  class: CSTexDefWdg

  ==================================================
*/
CSTexDefWdg::CSTexDefWdg( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	//
	// toplevel layouter
	//
	QBoxLayout *toplevel = new QVBoxLayout( this, 5 );
	QGroupBox *group = new QGroupBox( "CSTexDef", this );

	toplevel->addWidget( group );


 	QGridLayout *grid = new QGridLayout( group, 8+3+2, 4, 5 );

	grid->addRowSpacing( 0, 10 );
	grid->addColSpacing( 0, 5 );
	grid->setRowStretch( 7+3+2, 5 );
	grid->setColStretch( 3, 5 );

 	QLabel *l;
	l = new QLabel( group );
	l->setFrameStyle( QFrame::Panel | QFrame::Sunken );
	l->setLineWidth( 1 );
	l->setText( "default" );
	l->setMinimumSize( l->sizeHint() );
	grid->addMultiCellWidget( l, 1, 1, 1, 2 );
	ident = l;

	l = new QLabel( "shu", group );
	l->setMinimumSize( l->sizeHint() );
	grid->addWidget( l, 2, 1 );

 	l = new QLabel( "shv", group );
 	l->setMinimumSize( l->sizeHint() );	
 	grid->addWidget( l, 3, 1 );


 	l = new QLabel( "scu", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 4, 1 );

 	l = new QLabel( "scv", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 5, 1 );

 	l = new QLabel( "u1", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 6, 1 );


 	l = new QLabel( "v1", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 7, 1 );

 	l = new QLabel( "u2", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 8, 1 );

 	l = new QLabel( "v2", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 9, 1 );

 	l = new QLabel( "u3", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 10, 1 );

 	l = new QLabel( "v3", group );
 	l->setMinimumSize( l->sizeHint() );
 	grid->addWidget( l, 11, 1 );



	QSpinBox *s;
	s = new QSpinBox( -8192, 8192, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 2, 2 );
	shiftu = s;

	s = new QSpinBox( -8192, 8192, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 3, 2 );
	shiftv = s;

	s = new QSpinBox( -500, 500, 10, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 4, 2 );
	scaleu = s;

	s = new QSpinBox( -500, 500, 10, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 5, 2 );
	scalev = s;


	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 6, 2 );
	u1 = s;

	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 7, 2 );
	v1 = s;


	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 8, 2 );
	u2 = s;

	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 9, 2 );
	v2 = s;


	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 10, 2 );
	u3 = s;

	s = new QSpinBox( -512, 512, 8, group );
	s->setMinimumSize( s->sizeHint() );
	grid->addWidget( s, 11, 2 );
	v3 = s;


 	grid->activate();
	
	toplevel->activate();

	connect( shiftu, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( shiftv, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( scaleu, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( scalev, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( v1, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( u1, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( u2, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( v2, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( u3, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );
	connect( v3, SIGNAL( valueChanged ( const QString & ) ), this, SLOT( anyChange ( const QString & ) ) );

}	

CSTexDefWdg::~CSTexDefWdg()
{
	
}


/*
  ====================
  set

  ====================
*/
void CSTexDefWdg::set( cstexdef_t *_td )
{
	memcpy( &td, _td, sizeof( cstexdef_t ) );

	ident->setText( td.ident );
	shiftu->setValue( (int) td.shift[0] );
	shiftv->setValue( (int) td.shift[1] );
	scaleu->setValue( (int) ( td.scale[0] * 100.0 ) );
	scalev->setValue( (int) ( td.scale[1] * 100.0 ) );

	u1->setValue( (int) td.vecs[0][0] );
	u2->setValue( (int) td.vecs[1][0] );
	u3->setValue( (int) td.vecs[2][0] );

	v1->setValue( (int) td.vecs[0][1] );
	v2->setValue( (int) td.vecs[1][1] );
	v3->setValue( (int) td.vecs[2][1] );
}



/*
  ====================
  get

  ====================
*/
void CSTexDefWdg::get( cstexdef_t *_td )
{
	cstexdef_t		td;

	td.shift[0] = (float) shiftu->value();
	td.shift[1] = (float) shiftv->value();
	td.scale[0] = scaleu->value() / 100.0;
	td.scale[1] = scalev->value() / 100.0;

	td.vecs[0][0] = (float) u1->value();
	td.vecs[1][0] = (float) u2->value();
	td.vecs[2][0] = (float) u3->value();

	td.vecs[0][1] = (float) v1->value();
	td.vecs[1][1] = (float) v2->value();
	td.vecs[2][1] = (float) v3->value();

	strcpy( td.ident, ident->text() );

	memcpy( _td, &td, sizeof( cstexdef_t ) );
}

void CSTexDefWdg::anyChange( const QString & text )
{
	printf( "CSTexDef::anyChange %s\n", text.latin1() );
	emit changeSignal();
}

/*
  =============================================================================

  BrushSettingWdg

  main class of this module
  =============================================================================
*/

/*
  ====================
  BrushSettingWdg

  constructor
  ====================
*/
BrushSettingWdg::BrushSettingWdg( QWidget *parent, const char *name )
	: QWidget( parent, name )
{
	
	QGridLayout *toplevel = new QGridLayout( this, 5, 2, 5 );

	toplevel->setRowStretch( 4, 10 );
	toplevel->setColStretch( 1, 10 );

	bcw = new BrushContentsWdg( this );
	scw = new SurfaceContentsWdg( this );
	tdw = new TexDefWdg( this );
	cstdw = new CSTexDefWdg( this );

	QSize qs = tdw->minimumSize();
	printf( "*min size:%d, %d\n", qs.width(), qs.height() );

//	toplevel->addColSpacing( 0, 128 );
//	toplevel->addColSpacing( 1, 128 );


//	bcw->setMinimumSize( bcw->zeHint() );

//	scw->hide();
//	tdw->hide();

//	toplevel->addMultiCellWidget( bcw, 0, 0, 0, 0 );
//	toplevel->addMultiCellWidget( scw, 0, 0, 1, 1 );
//	toplevel->addMultiCellWidget( tdw, 1, 1, 0, 1 );

	toplevel->addWidget( bcw, 0, 0 );
	toplevel->addWidget( scw, 1, 0 );
	toplevel->addWidget( tdw, 2, 0 );
	toplevel->addMultiCellWidget( cstdw, 0, 1, 1, 1 );

	toplevel->activate();


	//
	// init BrushContents, SurfaceContents, TexDef
	//

	bcw->set( BRUSH_CONTENTS_LIQUID );
	scw->set( SURFACE_CONTENTS_CLOSE | SURFACE_CONTENTS_TEXTURE );
	texdef_t tdx;
	tdx.rotate = 0.0;
	tdx.scale[0] = 1.0;
	tdx.scale[1] = 1.0;
	tdx.shift[0] = 0.0;
	tdx.shift[1] = 0.0;
	strcpy( tdx.ident, "default" );
	tdw->set( &tdx );	

	connect( bcw, SIGNAL( changeSignal() ), this, SLOT( brushContentsChangeSlot() ) );
	connect( tdw, SIGNAL( changeSignal() ), this, SLOT( TexDefChangedSlot() ) );
	connect( cstdw, SIGNAL( changeSignal() ), this, SLOT( CSTexDefChangedSlot() ) );
}

BrushSettingWdg::~BrushSettingWdg()
{

}

void BrushSettingWdg::setBrushContents( unsigned int contents )
{
	bcw->set( contents );
	initSurfaceFromBrush();
	resetTexDef();
}

unsigned int BrushSettingWdg::getBrushContents( void )
{
	return bcw->get();
}

void BrushSettingWdg::setSurfaceContents( unsigned int contents )
{
	scw->set( contents );
}

unsigned int BrushSettingWdg::getSurfaceContents( void )
{
	return scw->get();
}

void BrushSettingWdg::setTexDef( texdef_t *texdef )
{
	tdw->blockSignals( true );
	tdw->set( texdef );
	tdw->blockSignals( false );
}

void BrushSettingWdg::getTexDef( texdef_t *texdef )
{
	tdw->get( texdef );
}

void BrushSettingWdg::initSurfaceFromBrush( void )
{
	unsigned int	contents;
	unsigned int	c;
	int		i;
	
	contents = bcw->get();

	for ( i = 0; c_brushes[i]; i+=3 )
	{
		if ( (unsigned int) atoi(c_brushes[i+1]) == contents )
		{
			c = OrNumberString( c_brushes[i+2] );
			scw->set( c );
		}
	}
	
#if 0
	if ( contents == BRUSH_CONTENTS_SOLID )
		scw->set( SURFACE_CONTENTS_CLOSE | SURFACE_CONTENTS_TEXTURE );
	
	if ( contents == BRUSH_CONTENTS_LIQUID )
		scw->set( SURFACE_CONTENTS_OPEN | SURFACE_CONTENTS_TEXTURE | SURFACE_CONTENTS_WINDOW );
	
	if ( contents == BRUSH_CONTENTS_HINT )
		scw->set( SURFACE_CONTENTS_OPEN );	
#endif
}

void BrushSettingWdg::resetTexDef( void )
{
	texdef_t	td;
	
	tdw->get( &td );
	// keep ident
	td.rotate = 0.0;
	td.scale[0] = 1.0;
	td.scale[1] = 1.0;
	td.shift[0] = 0.0;
	td.shift[1] = 0.0;
	tdw->set( &td );
}

/*
  ====================
  brushContentsChangeSlot

  qt slot
  ====================
*/
void BrushSettingWdg::brushContentsChangeSlot()
{
	initSurfaceFromBrush();
	resetTexDef();
}



/*
  ====================
  setCSTexDef

  ====================
*/
void BrushSettingWdg::setCSTexDef( cstexdef_t *texdef )
{
	cstdw->blockSignals( true );	// no signals from the SpinBoxes
	cstdw->set( texdef );
	cstdw->blockSignals( false );
}



/*
  ====================
  getCSTexDef

  ====================
*/
void BrushSettingWdg::getCSTexDef( cstexdef_t *texdef )
{
	cstdw->get( texdef );
}

/*
  ====================
  CSTexDefChangedSlot

  ===================
*/
void BrushSettingWdg::CSTexDefChangedSlot( void )
{
	printf( "BrushSettingWdg::CSTexDefChangedSlot\n" );
	emit CSTexDefChanged();
}


/*
  ====================
  TexDefChangedSlot

  ====================
*/
void BrushSettingWdg::TexDefChangedSlot( void )
{
	printf( "BrushSettingWdg::TexDefChangedSlot\n" );
	emit TexDefChanged();
}



/*
  =============================================================================
  BrushTool

  - new Brush from BrushSettingsWdg
  - new Faces from BrushSettingsWdg
  =============================================================================
*/
BrushTool::BrushTool( QObject * parent, const char * name )
	: QObject( parent, name )
{
	printf( "BrushTool: constructor\n" );
}

BrushTool::~BrushTool()
{

}

/*
  ====================
  createBrush

  ====================
*/
brush_t* BrushTool::createBrush( float edgelength )
{
	face_t		*facehead;
	face_t		*f;
	brush_t		*b;
	vec3d_t		norm;
	float		dist;

	dist = edgelength*0.5;
	facehead = NULL;

	Vec3dInit( norm, -1.0, 0.0, 0.0 );
	f = createFace( norm, dist );
	f->next = facehead;
	facehead = f;

	Vec3dInit( norm, 1.0, 0.0, 0.0 );
	f = createFace( norm, dist );	
	f->next = facehead;
	facehead = f;

	Vec3dInit( norm, 0.0, -1.0, 0.0 );
	f = createFace( norm, dist );
	f->next = facehead;
	facehead = f;

	Vec3dInit( norm, 0.0, 1.0, 0.0 );
	f = createFace( norm, dist );	
	f->next = facehead;
	facehead = f;

	Vec3dInit( norm, 0.0, 0.0, -1.0 );
	f = createFace( norm, dist );
	f->next = facehead;
	facehead = f;

	Vec3dInit( norm, 0.0, 0.0, 1.0 );
	f = createFace( norm, dist );	
	f->next = facehead;
	facehead = f;

	b = NewBrush();
	b->id = wwm_i->getID();
	b->faces = facehead;
	b->contents = brushsetting_wdg_i->getBrushContents();

	return b;
}


/*
  ====================
  createFace

  ====================
*/
face_t* BrushTool::createFace( vec3d_t norm, float dist )
{
	face_t	*f;
 
	f = NewFace();
	f->id = wwm_i->getID();
	f->plane.dist = dist;
	Vec3dCopy( f->plane.norm, norm );

	brushsetting_wdg_i->getTexDef( &f->texdef );
	f->contents = brushsetting_wdg_i->getSurfaceContents();

	return f;
}


void BrushTool::moveBrush( brush_t *b, vec3d_t delta )
{
	face_t		*f;
	vec3d_t		origin;
	vec3d_t		norm;
	float		dist;

	for ( f = b->faces; f ; f=f->next ) {
		Vec3dCopy(norm, f->plane.norm); // norm = f->plane.norm;
		dist = f->plane.dist;
		
		Vec3dScale( origin, dist, norm );
		Vec3dAdd( origin, origin, delta );
		
		dist = Vec3dDotProduct( origin, norm );
		
		Vec3dCopy(f->plane.norm, norm); // f->plane.norm = norm;
		f->plane.dist = dist;
		
		// free old polygons
//		FreePolygon( f->polygon );
	}	
	
	// generate new polygons
//	ClipBrushFaces( b );
	
	b->select|=SELECT_UPDATE;	
}
