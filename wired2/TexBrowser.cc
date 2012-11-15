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



// TexBrowser.cc

#include <qapplication.h>
#include <qdir.h>
#include <stdlib.h>
#include <qpainter.h>
#include <qpixmap.h>
#include <string.h>
#include <unistd.h>
#include <assert.h> // fixme
#include "TexRes.hh"
#include "texture.h"

#include "cdb_service.h"
#include "TexBrowser.hh"
#include <qprogressdialog.h> 
#include "tga.h"

#include "qwindowsstyle.h"

#define PMWIDTH		( 64 )
#define PMHEIGHT	( 64 )

int	allmem = 0;
char	actident[256];
static	QWidget*	mag;
static	int	magopen = 0;
static  QFont*	bfont;

extern TexRes *texres_i;

TextureBrowser::TextureBrowser( QWidget* parent, const char* name )
	: QWidget( parent, name )
{
	int	i;
	//resize( 600, 440 );
//	CDB_StartUp( 0 );
	pagenum = 0;
	setStyle( new QWindowsStyle() );
	bfont = new QFont( "Helvetica", 10, QFont::Normal );	

	memset( actident, 0, 255 );
	for( i = 0; i < 100; i++ )
	{
		memset( pagelist[i].name, 0, 255 );
	}
	cbox = new QComboBox( this );
//	cbox->setGeometry( 0, 0, 90, 30 );
	if( !cbox )
	{
		wired_i->WOSError( "TextureBrowser: cbox == NULL\n" );
	}

	connect( cbox, SIGNAL( activated( int ) ), this, SLOT( cboxActivatedSlot( int )));
	readArrs();
	oldindex = 0;
	
	resize( 300, 400 );
}

TextureBrowser::~TextureBrowser()
{
	int	i;

//	printf( "freeing %d bytes ...\n", allmem );
	for( i = 0; i < pagenum; i++ )
	{
		if( pagelist[i].widget != NULL )
			delete pagelist[i].widget;
	}
}

void TextureBrowser::readArrs()
{
#if 0
	QDir	dir;
	char	subdirs[100][256];
	char	arrname[256];
	char	ident[255];
	char*	ptr;
	QString	startuppath(256);
	char*	texture_path;
	char	label[255];

	unsigned int	i, i2;
	unsigned int	subdir_num, arr_count = 0;

	texture_path = CDB_GetString( "wired/texture_path" );
	if( texture_path == NULL )
	{
		wired_i->WOSError( "no wired/texture_path found in cdb. please add.\n" );
	}

	startuppath = dir.absPath();
	dir.setCurrent( texture_path );
	dir.setFilter( 0 );
	dir.setFilter( QDir::Dirs );
//	printf( "dirs: %i\n", dir.count() );
	for( i = 0; i < dir.count() - 2; i++ )
	{
		strcpy( subdirs[i], dir[i+2] ); //!!!warning: dir is not an array. this is a fucking operator!!!
		//strcpy( subdirs[i], dir.entryList()->at(i)->path() );
		//printf( "%i: %s\n", i, subdirs[i] );
	}
	subdir_num = dir.count() - 2;

//	printf( "progress\n" );
	
	QProgressDialog*        progress = new QProgressDialog( "Reading arrs ...", "Cancel", subdir_num, this, "progress", TRUE );
	progress->setTotalSteps( subdir_num );
	progress->setStyle( WindowsStyle ); 
	progress->setLabelText( "Reading arrs ..." );
	progress->show();

	dir.setFilter( 0 );
	dir.setFilter( QDir::Files );
	
	dir.setNameFilter( "*.tga" );

	progress->wasCancelled();
	for( i = 0; i < subdir_num; i++ )
	{
		if( !dir.cd( subdirs[i] ))
		{
			printf( "skipping dir.\n" );
			continue;
		}
		progress->setProgress( i );
		sprintf( label, "scanning %s", subdirs[i+1] );
                progress->setLabelText( label );
                if( progress->wasCancelled() )
                {
                        printf( "reading arrs was cancelled\n" );
                        break;
                }
		pagelist[pagenum].widget = new TDPage( subdirs[i], this );
		
		//pagelist[pagenum].widget->setGeometry( 0, 30, width(), height() - 30 );
		if( page )
			pagelist[page].widget->hide();
		else
			pagelist[page].widget->show();

		strcpy( pagelist[page].name, subdirs[i] );
		cbox->insertItem( subdirs[i] );
		//pagelist[pagenum].widget->addEntry( arrname, ident_name );
		connect( pagelist[pagenum].widget, SIGNAL( acceptSignal() ), this, SLOT( acceptSlot()));
		connect( pagelist[pagenum].widget, SIGNAL( clickSignal() ), this, SLOT( clickSlot()));
		
//		printf( "%s: %i\n", subdirs[i], dir.count() );
		for( i2 = 0; i2 < dir.count(); i2++ )
		{
			strcpy( arrname, dir.absFilePath( dir[i2] ));
			strcpy( ident, dir[i2] );
			//addArr( arrname );
			if( (unsigned int)(strchr( ident, '.' )- ident ) > strlen( ident ))
				continue;
			ptr = strchr( ident, '.' );
			*ptr = 0;
			pagelist[pagenum].widget->addEntry( arrname, ident );
			arr_count++;
		}
		pagenum++;
		if( !dir.cdUp() )
		{
			printf( "cannot change up dir.\n" );
			break;
		}
	}
       	progress->setProgress( subdir_num );
 	dir.setCurrent( ( const char* )startuppath );
	delete 	progress;
#endif

	u_list_iter_t	iter;
	texident_t	*ident;
	int	pagenum = 0;
	int	i;
	
	texres_i->initIdentListIter( &iter );

	for( i = 0; i < 100; i++ )
	{
		pagelist[i].widget = NULL;
	}

	for( ; ( ident = (texident_t * )U_ListIterNext( &iter )); )
	{
		int	page;
		char	type[256], name[256], text[256];
		char	*ptr;

		strcpy( text, ident->ident );

//		printf( "ident: %s\n", text );

		ptr = strchr( text, '/' );
		if( !ptr )
		{
			printf( "bad ident %s\n", text );
			continue;
		}

		*ptr = 0;

		strcpy( type, text );
		strcpy( name, ptr+1 );
	
//		printf( "type: %s name: %s\n", type, name ); 
	

		
		page = pagenum;

		for( i = 0; i < pagenum; i++ )
		{
			if( !pagelist[i].widget )
				continue;

			if( !strcmp( type, ( const char * )pagelist[i].widget->pageName() ))
			{
				page = i;
				break;
			}
		}

		if( !pagelist[page].widget )
		{
//			printf( "adding page %s\n", type );
			pagelist[page].widget = new TDPage( type, this );
			
			//pagelist[pagenum].widget->setGeometry( 0, 30, width(), height() - 30 );
			if( page )
				pagelist[page].widget->hide();
			else
				pagelist[page].widget->show();
			
			strcpy( pagelist[page].name, type );
			cbox->insertItem( type, page );
			//pagelist[pagenum].widget->addEntry( arrname, ident_name );
			connect( pagelist[page].widget, SIGNAL( acceptSignal() ), this, SLOT( acceptSlot()));
			connect( pagelist[page].widget, SIGNAL( clickSignal() ), this, SLOT( clickSlot()));


			
			pagenum++;
		}
		pagelist[page].widget->addEntry( ident->ident, name );
	}
		
	printf( "outside\n" );
	//exit( 0 );

	
}


char* TextureBrowser::setActIdent( char* arg_ident )
{
	char*	ident;
	char*	slash;
	char*	type;
	char*	name;
	int	i;

	assert( arg_ident );
	ident = ( char* )malloc( strlen( arg_ident+1 ));
	assert( ident );
	strcpy( ident, arg_ident );
	
	slash = strchr( ident, '/' );
	
	if( slash <= ident || slash > ident + strlen( ident ) )
	{
		printf( "ident has wrong format: %s %p %p\n", arg_ident, arg_ident, slash );
		free( ident );
		return NULL;
	}
	
	*slash = 0;
	type = ident;
	name = slash+1;
//	printf( "type %s name %s\n", type, name );
	for( i = 0; i < pagenum; i++ )
	{
		if( !strcmp( type, pagelist[i].name ))
		{
//			printf( "found page %i\n", i );
			cboxActivatedSlot( i );
			cbox->setCurrentItem( i );
			pagelist[i].widget->highlightEntry( name );
			break;
		}
	}

	free( ident );
	return actident;
}


void TextureBrowser::cboxActivatedSlot( int index )
{
	if( index < 0 )
		return;

	//printf( "activated page %s\n", pagelist[index].name );
	pagelist[oldindex].widget->hide();
	pagelist[index].widget->show();
	oldindex = index;
}

void TextureBrowser::acceptSlot()
{
	printf( "accept %s\n", actident );
	emit acceptSignal( actident );
}

void TextureBrowser::clickSlot()
{
	printf( "clicked %s\n", actident );
	emit clickSignal( actident );
}

void TextureBrowser::resizeEvent( QResizeEvent* event )
{
	int	i;
	cbox->setGeometry( 0, 0, 90, 25 );
	for( i = 0; i < 100; i++ )
	{
		if( pagelist[i].widget )
			pagelist[i].widget->setGeometry( 0, 25, width(), height() - 25);
	}
}

TDPage::TDPage( char* page_name, QWidget* parent, const char* name )
	:QGridView( parent, name )
{
	char*	pal_name;
	FILE*	pal_handle;

	curcol = 0;
	currow = 0;
	entrycount=0;
	actrow = actcol = 0;

	//setBackgroundMode( PaletteDark );
	setBackgroundColor( black );
//	setTableFlags( Tbl_autoVScrollBar | Tbl_autoHScrollBar /*| Tbl_smoothScrolling*/ );
	if( page_name == NULL )
	{
		printf( "page name is NULL\n" );
		return;
	}

	if( strlen( page_name ) >=256 )
		page_name[255] = '\0';

	strcpy( myname, page_name );
	colnum = 4;
	setNumCols( colnum );
//	printf( "colnum %d\n", colnum );
	setNumRows( 1 );
	setCellWidth( 72 );
	setCellHeight( 81 );
	pal_name = CDB_GetString( "wired/default_pal" );
	if( pal_name == NULL )
	{
		wired_i->WOSError( "cannot find key wired/default_pal in cdb. please add it.\n" );
	}
	pal_handle = fopen( pal_name, "rb" );
	if( pal_handle == NULL )
	{
		wired_i->WOSError( "cannot open pal %s. check path\n", pal_name );
	}
	pal = PAL_Read( pal_handle );


}

TDPage::~TDPage()
{
	int	i;
//	__named_message( "\n" );
	for( i = 0; i < entrycount; i++ )
	{
		if( entries[i] != NULL )
			delete entries[i];
	}
	if( pal != NULL )
		PAL_Free( pal );

}

void TDPage::paintCell( QPainter* painter, int row, int col )
{
	char	text[256];

	int w = cellWidth();
	int h = cellHeight();
	
	int x2 = w - 1;
	int y2 = h - 1;


	//painter->drawLine( x2, 0, x2, y2 );
	//painter->drawLine( 0, y2, x2, y2 );
//	printf( "cell painted. row %d\n", row );
	if( indexOf( row, col ) < entrycount )
	{
		if( entries[indexOf( row, col )]->iserror )
			return;

		entries[indexOf( row, col )]->loadArr( pal );
		
		if( entries[indexOf( row, col )]->iserror )
			return;
		painter->drawPixmap( ( 72 - entries[indexOf( row, col )]->width)/2, (72- entries[indexOf( row, col )]->height )/2  , *entries[indexOf( row, col )]->pixmap );
		if( !entries[indexOf( row, col )]->isanim )
			painter->setPen( white );
		else
			painter->setPen( red );

		sprintf( text, "%s"/* (%dx%d)"*/, entries[indexOf( row, col )]->myname/*, entries[indexOf( row, col )]->origx, entries[indexOf( row, col )]->origy*/ );
		painter->setFont( *bfont ); 
		painter->drawText( 1, 65, w - 2, h - 65, AlignCenter, text );
		if( ( row == actrow ) && ( col == actcol ))
		{
			painter->setPen( SolidLine );
			painter->setPen( blue );
			painter->drawRect( 0, 0, x2, y2 );
		}
	}
}

void TDPage::mousePressEvent( QMouseEvent* event )
{
	int oldrow = actrow;
	int oldcol = actcol;
	
	QPoint clickpos = event->pos();
	QPoint globpos;

	actrow = rowAt( clickpos.y());
	actcol = columnAt( clickpos.x());

	if( ((actrow == currow) && (actcol >= curcol)) || (actrow == -1) )
		return;
	
	sprintf( actident, "%s/%s", myname, entries[indexOf( actrow, actcol )]->myname );
	emit clickSignal();

	if( ( actrow != oldrow ) || ( actcol != oldcol ))
	{
		updateCell( oldrow, oldcol );
		updateCell( actrow, actcol );
	}
#if 0
	if( event->button() == RightButton )
	{
		mag = new QWidget( NULL, NULL, WStyle_Customize | WStyle_Tool | WStyle_NoBorder );
		mag->resize( entries[indexOf( actrow, actcol )]->origx, entries[indexOf( actrow, actcol )]->origy );
		globpos = mapToGlobal( clickpos );
		mag->show();
		mag->move( globpos );
		
		painter = new QPainter();
		painter->begin( mag );
		//painter->drawPixmap( 0, 0, *entries[indexOf( actrow, actcol )]->pixmap );
		arrhandle = fopen( entries[indexOf( actrow, actcol )]->arrname, "rb" );
		if( arrhandle == NULL )
		{
			delete mag;
			return;
		}
		arr = ARR_Read( arrhandle );
		fclose( arrhandle );
		if( arr == NULL )
		{
			delete mag;
			fclose( arrhandle );
			return;
		}
		for( iy = 0; iy < arr->size_y; iy++ )
			for( ix = 0; ix < arr->size_x; ix++ )
			{
				col = *((unsigned short*)(&arr->data[(iy*arr->size_x+ix)*2]));
				r = col >> 11;
				r &= 31;
				r <<= 3;
				g = col >> 5;
				g &= 63;
				g <<=2;
				b = col;
				b &= 31;
				b <<= 3;
				
				painter->setPen( QColor( r, g, b ));
//painter->setPen( QColor( pal->rgb_set[col].red, pal->rgb_set[col].green, pal->rgb_set[col].blue ));
				painter->drawPoint( ix, iy );
			}
		painter->end();
		ARR_Free( arr );
		magopen = 1;
		
	}
#endif
}
	
void TDPage::mouseReleaseEvent( QMouseEvent* event )
{
	if( magopen )
	{
		magopen = 0;
		delete mag;
	}
}

void TDPage::mouseDoubleClickEvent( QMouseEvent* event )
{
	QPoint clickpos = event->pos();

	actrow = rowAt( clickpos.y());
	actcol = columnAt( clickpos.x());
	if( ((actrow == currow) && (actcol >= curcol)) || (actrow == -1) )
		return;

	sprintf( actident, "%s/%s", myname, entries[indexOf( actrow, actcol )]->myname );
//	printf( "double click actident: %s\n", actident );
	emit acceptSignal();
}

void TDPage::addEntry( char* arrname, char* name )
{
	if( curcol == colnum )
	{
		curcol = 0;
		currow++;
		setNumRows( currow + 1 );
	}
	int	index = indexOf( currow, curcol );
	//image_tmp = new QImage( 128, 128, 8 );
	assert( arrname );
	entries[index] = new PageEntry( arrname, name );

	entrycount++;
	//entries[index]->loadArr( pal );
	curcol++;
}
	

char* TDPage::pageName()
{
	return myname;
}

int TDPage::indexOf( int row, int col )
{
	return (row * numCols()) + col;
}

PageEntry::PageEntry( char* arg_arrname, char* arg_myname )
{
	iserror = FALSE;
	havearr = FALSE;
	assert( arg_arrname );
	if( strlen( arg_arrname ) >= 256 )
	{
		arg_arrname[255] = '\0';
	}
	strcpy( arrname, arg_arrname );
	assert( arg_arrname );
	if( strlen( arg_myname ) >= 32 )
	{
		arg_myname[31] = '\0';
	}
	strcpy( myname, arg_myname );
}

PageEntry::~PageEntry()
{
	//__named_message( "%p\n", pixmap );
	if( havearr )
		delete pixmap;
}

int PageEntry::loadArr( pal_t* pal )
{
	if( havearr )
		return 0;

#if 1
	
	
	texident_t	*ident;
	QImage	*image;

	pixmap = new QPixmap();

#if 1
	int	nwidth, nheight;
	int	owidth, oheight;

	ident = texres_i->getTexIdentByIdent( arrname );

	image = TexIdent_BuildQImage( ident );
	image->setAlphaBuffer( false );
	origx = owidth = image->width();
	origy = oheight = image->height();

		
	if( owidth > PMWIDTH || oheight > PMHEIGHT )
	{
		// should work fine with 2^x
		if( owidth >= oheight )
		{
			nwidth = PMWIDTH;
			nheight = oheight / ( owidth / nwidth );
		}
		if( owidth < oheight )
		{
			nheight = PMHEIGHT;
			nwidth = owidth / ( oheight / nheight );
		}
	} else 
	{
		nwidth = owidth;
		nheight = oheight;
	}	
	
	QImage nimage( nwidth, nheight, 32 );
	nimage.fill(0x80808080);

	if( (nwidth != owidth) || ( nheight != oheight ))
	{
		nimage = image->smoothScale( nwidth, nheight );
	} else
	{
		nimage = image->copy();
	}

	pixmap->resize( nwidth, nheight );
	pixmap->convertFromImage( nimage );

	width = nwidth;
	height = nheight;

	delete image;
#endif

	havearr = TRUE;


	
	isanim = FALSE;
	iserror = FALSE;



	return 0;

#else

	FILE	*h;
	tga_t	*tga;

	QRgb	pixel;
	int	ix, iy;

	iserror = TRUE;

	h = fopen( arrname, "rb" );
	
	if( !h )
	{
		wired_i->WOSMessage( "Warning:\ncannot open file '%s'\n", arrname );
		return 0;
	}

	tga = TGA_Read( h );
	
	if( !h )
	{
		wired_i->WOSMessage( "Warning:\ncannot read file '%s'\n", arrname );
		return 0;
	}

	if( tga->image_type != TGA_TYPE_TRUECOLOR )
	{
		wired_i->WOSMessage( "Warning:\nfile is not truecolor: '%s'\n", arrname );
		return 0;
	}
		

	pixmap = new QPixmap();

	int	nwidth, nheight;
	int	owidth = tga->image_width, oheight = tga->image_height;

//	printf( "w: %d h: %d\n", owidth, oheight );
	if( owidth > PMWIDTH || oheight > PMHEIGHT )
	{
		// should work fine with 2^x
		if( owidth >= oheight )
		{
			nwidth = PMWIDTH;
			nheight = oheight / ( owidth / nwidth );
		}
		if( owidth < oheight )
		{
			nheight = PMHEIGHT;
			nwidth = owidth / ( oheight / nheight );
		}
	} else 
	{
		nwidth = owidth;
		nheight = oheight;
	}	

	QImage oimage( owidth, oheight, 32 );

	for( iy = 0; iy < oheight; iy++ )
	{	
		for( ix = 0; ix< owidth; ix++ )
		{
			int r, g, b;
			r = tga->image.red[iy*owidth+ix];
			g = tga->image.green[iy*owidth+ix];
			b = tga->image.blue[iy*owidth+ix];
			

			pixel = qRgb( r, g, b );
			oimage.setPixel( ix, iy, pixel );
		}
	}



	fclose( h );
	
	QImage nimage( nwidth, nheight, 32 );

	if( (nwidth != owidth) || ( nheight != oheight ))
	{
		nimage = oimage.smoothScale( nwidth, nheight );
	} else
	{
		nimage = oimage.copy();
	}

	pixmap->resize( nwidth, nheight );
	pixmap->convertFromImage( nimage );

	havearr = TRUE;

	width = nwidth;
	height = nheight;
	origx = tga->image_width;
	origy = tga->image_height;
	
	isanim = FALSE;
	iserror = FALSE;
	TGA_Free( tga );

	return 0;
#endif
}

void TDPage::highlightEntry( char* arg_name )
{
	int	i;
	
	assert( arg_name );
	for( i = 0; i < entrycount; i++ )
	{
		if( !strcmp( arg_name, entries[i]->myname ))
		{
			printf( "%s found in row %i col %i\n", arg_name, i/4, i%4 );
			actrow = i/4;
			actcol = i%4;

			ensureCellVisible( actrow, actcol );
//			setTopCell( actrow );
//			updateCell( actrow, actcol, TRUE );
		       
			break;
		} 
	}
      
}
