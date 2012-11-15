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



// TexRes.cc

#include "TexRes.hh"

#include "lib_hobj.h"
#include "lib_container.h"
#include "s_mem.h"

#include "tga.h"

#include "qpainter.h"
#include "qpixmap.h"

extern TexRes	*texres_i;

texident_t * TexIdent_GetByIdent( const char *ident )
{
	texident_t		*ti;

	ti = texres_i->getTexIdentByIdent( ident );
	
	if ( !ti )
	{
		return NULL;
	}
	
	return ti;
}

static int TexIdentCompareKeys( const void *key1, const void *key2 )
{
	char	*ident1;
	char	*ident2;

	ident1 = (char *) key1;
	ident2 = (char *) key2;

	return strcmp( ident1, ident2 );
}

static void * TexIdentGetKey( const void *obj )
{
	return (void *) ((texident_t *)obj)->ident;
}

static void IdentFromResName( char *ident, char *resname )
{
	int		i;
	char		*ptr;
	
	ptr = strchr( resname, '.' );
	if ( !ptr )
	{
		ident[0] = 0;
		return;
	}
	
	ptr++;
	for ( i = 0; *ptr ; i++, ptr++ )
	{
		ident[i] = *ptr;
		if ( ident[i] == '.' )
			ident[i] = '/';
	}
	ident[i++] = 0;	
}

void TexIdent_FlipH( texident_t *ti )
{
	int		i;
	unsigned char	*in;
	unsigned char	*out;

	in = (unsigned char *) ti->image;

	out = (unsigned char *) NEWBYTES( ti->width*ti->height*4 );
	
	for ( i = 0; i < ti->height; i++ )
	{
		memcpy( &out[((ti->height-1)-i)*ti->width*4], &in[i*ti->width*4], ti->width*4 );
	}

	FREE( in );
	
	ti->image = out;
}

void TexIdentInitImageFromTGA( texident_t *ti, char *name )
{
	unsigned int		i;
	FILE		*h;
	tga_t		*tga;

	unsigned char	*image;	

	h = fopen( name, "r" );
	if ( !h )
	{		
		printf( "WARNING: can't open file '%s'\n", name );
		return;
	}

	tga = TGA_Read( h );
	fclose( h );

	if ( !tga )
		Error( "TGA read failed for '%s'\n", name );	

	if ( tga->image_type != TGA_TYPE_TRUECOLOR )
	{
		printf( "WARNING: tga type failed '%s'\n", name );
		return;
	}

	image = (unsigned char *) malloc( tga->image.pixels * 4 );
	
	for ( i = 0; i < tga->image.pixels; i++ )
	{
		image[i*4+0] = tga->image.red[i];
		image[i*4+1] = tga->image.green[i];
		image[i*4+2] = tga->image.blue[i];
		image[i*4+3] = 0;
	}

	ti->image = image;
	ti->width = tga->image_width;
	ti->height = tga->image_height;

	TGA_Free( tga );
}

void TexIdentInitImageFromMultilayer( texident_t *ti )
{
	hobj_t		*layer;
	hpair_t		*pair;
	unsigned char		*dst, *src;
	char			tt[256];

	if ( ti->type != texIdentType_multilayer )
	{
		Error( "TexIdentInitFromMultilayer: texident is not of type multilayer\n" );
	}
     
	//
	// generate an alias image for the multilayer
	//

	printf( "TexIdentInitImageFromMultilayer: generate alias image for '%s' ...\n", ti->obj->name );

	// get layer with ordinal '1' and use its ident as base image

	ti->width = 128;
	ti->height = 128;

	QPixmap		pm( ti->width, ti->height );
	QPainter	pa;
	QImage		img;

	hobj_search_iterator_t	iter;
	hobj_t		*best_l;
	int		best_o;

	pa.begin( &pm );
	pa.eraseRect( 0, 0, ti->width, ti->height );
	pa.end();


	best_l = NULL;
	best_o = 99999999;
	InitClassSearchIterator( &iter, ti->obj, "layer" );
	for ( ; ( layer = SearchGetNextClass( &iter ) ) ; )
	{
		int ordinal = atoi( layer->name );
		if ( ordinal < best_o )
		{
			best_o = ordinal;
			best_l = layer;
		}
	}

	layer = best_l;

	if ( layer )
	{
		pair = FindHPair( layer, "gltex_res" );
		if ( pair )
		{
			IdentFromResName( tt, pair->value );

			texident_t	*ti2;
			ti2 = texres_i->getTexIdentByIdent( tt, true );
			if ( ti2 )
			{
				if ( ti2->image )
				{
					QImage		*img2;				
					
					ti->width = ti2->width;
					ti->height = ti2->height;

					pm.resize( ti2->width, ti2->height );
					
					img2 = TexIdent_BuildQImage( ti2 );
					pm.convertFromImage( *img2 );
					delete img2;
				}
				else
				{
					printf( "empty image\n" );
				}
			}
			else
			{
				printf( "(null) texident\n" );
			}
		}
		else
		{
			printf( "WARNING: TexIdentInitImageFromMultilayer - missing key 'gltex_res'\n" );
		}
	}
	
	pa.begin( &pm );
//	pa.eraseRect( 0, 0, ti->width, ti->height );
	pa.setRasterOp( Qt::NotXorROP );
	pa.drawRect( 2, 2, ti->width-2, ti->height-2 );
	sprintf( tt, "multilayer" );
	pa.drawText( 4, 4, ti->width-4, ti->height-4, Qt::AlignLeft|Qt::AlignTop, tt );
	pa.end();
	
	img = pm.convertToImage(); 
	img.setAlphaBuffer( true );
	
	ti->image = (unsigned char *) malloc( ti->width*ti->height*4 );
	
	dst = (unsigned char *) ti->image;
	src = (unsigned char *) img.bits();
	
	memcpy( dst, src, ti->width*ti->height*4 );	
}

static void SetupMultilayerImage( void *obj )
{
	texident_t		*ti;

	ti = (texident_t *) obj;

	if ( ti->type != texIdentType_multilayer )
		return;

	TexIdentInitImageFromMultilayer( ti );
}

/*
  ==============================
  TexRes::TexRes

  ==============================
*/
TexRes::TexRes( const char *path )
{
	int		num_multilayer;
	int		num_gltexres;
	char				tt[256];
	hobj_search_iterator_t		iter;
	hobj_t				*multilayer;
	hobj_t				*gltexres;

	texident_t	*ti;

	texres_i = this;

	printf( "create TexRes, path is '%s'\n", path );

	//
	// load gltex_res class
	//

	sprintf( tt, "%s/res/gltex_res.hobj", path );
	printf( " gltex_res class: %s\n", tt );
	this->gltexres_hm = NewHManagerLoadClass( tt );

	//
	// load multilayer class
	//
	
	sprintf( tt, "%s/shape_config/multilayer.hobj", path );
	printf( " multilayer class: %s\n", tt );
	this->multilayer_hm = NewHManagerLoadClass( tt );
	

	U_InitMap( &this->gltexres_map, map_default, TexIdentCompareKeys, TexIdentGetKey );
	U_InitMap( &this->multilayer_map, map_default, TexIdentCompareKeys, TexIdentGetKey );
	U_InitList( &this->ident_list );
	
	// multilayer idents override gltexres idents

	// 
	// insert multilayer idents into map
	//

	num_multilayer = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( this->multilayer_hm ), "multilayer" );
	for ( ; ( multilayer = SearchGetNextClass( &iter ) ) ; )
	{

		ti = (texident_t *) U_MapSearch( &this->multilayer_map, (void *) multilayer->name );
		if ( ti )
		{
			printf( "WARNING: multilayer name still in map, ignore\n" );
			continue;
		}

		ti = NEWTYPE( texident_t );
		ti->obj = multilayer;
		ti->type = texIdentType_multilayer;
		strcpy( ti->ident, multilayer->name );

//		TexIdentInitImageFromMultilayer( ti );

		U_MapInsert( &this->multilayer_map, ti );
		U_ListInsertAtHead( &this->ident_list, ti );

		num_multilayer++;
	}

	
	//
	// insert gltexres idents into map
	//

	num_gltexres = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( this->gltexres_hm ), "resource" );
	for ( ; ( gltexres = SearchGetNextClass( &iter ) ) ; ) 
	{
		hpair_t		*pair;

		if ( strcmp( gltexres->name, "gltex" ) )
			continue;

		pair = FindHPair( gltexres, "name" );
		if ( !pair ) 
			continue;
		
		IdentFromResName( tt, pair->value );


		ti = (texident_t *) U_MapSearch( &this->gltexres_map, (void *) tt );
		if ( ti )
		{
			printf( "WARNING: multilayer overrides gltexres for ident '%s', ignore\n", tt );
			continue;
		}

		ti = NEWTYPE( texident_t );
		ti->obj = gltexres;
		ti->type = texIdentType_gltexres;
		strcpy( ti->ident, tt );
		
		pair = FindHPair( gltexres, "path" );
		if ( !pair )
		{
			Error( "missing key 'path' in gltex resource\n" );
		}
		sprintf( tt, "%s/%s", path, pair->value );
		TexIdentInitImageFromTGA( ti, tt );

		U_MapInsert( &this->gltexres_map, ti );


		if ( !U_MapSearch( &this->multilayer_map, ti->ident ) )
		{
			// not multilayer with this gltexres ident, so it's a public ident
			U_ListInsertAtHead( &this->ident_list, ti );
		}

		
		num_gltexres++;
	}

	printf( " %d multilayer idents\n", num_multilayer );
	printf( " %d gltexres idents\n", num_gltexres );
	printf( " => %d public idents\n", U_ListLength( &this->ident_list ) );

	//
	// build multilayer images
	//

	U_MapForEach( &this->multilayer_map, SetupMultilayerImage );

	//
	// final h-flip of all images, so origin is in the lower left corner
	//
	u_list_iter_t		list_iter;

	U_ListIterInit( &list_iter, &this->ident_list );
	for ( ; ( ti = (texident_t *) U_ListIterNext( &list_iter ) ) ; )
	{
		TexIdent_FlipH( ti );
	}
}


/*
  ==============================
  TexRes::~TexRes

  ==============================
*/
TexRes::~TexRes()
{
	printf( "destroy TexRes\n" );
}


/*
  ==============================
  TexIdent_BuildQImage

  ==============================
*/
QImage	* TexIdent_BuildQImage( texident_t *ti )
{
	QImage		*img;

	unsigned char	*src;
	unsigned char	*dst;

	img = new QImage( ti->width, ti->height, 32 );
	img->setAlphaBuffer( true );
	img->fill( 0x80808080 );

	src = (unsigned char *) ti->image;
	dst = (unsigned char *) img->bits();

	if ( src && dst )
	{
//		memcpy( dst, src, ti->width*ti->height*4 );
		for ( int i = 0; i < ti->width*ti->height*4; i += 4 ) {
			unsigned char r = src[i+0];
			unsigned char g = src[i+1];
			unsigned char b = src[i+2];
			
			dst[i+0] = b;
			dst[i+1] = g;
			dst[i+2] = r;
		}
	}
	else
	{
		printf( "WARINIG: buildQImageByIdent - empty image for ident '%s'\n", ti->ident );		
	}
	return img;
}

/*
  ==============================
  TexRes::initIdentListIter

  ==============================
*/
void TexRes::initIdentListIter( u_list_iter_t *list_iter )
{
	U_ListIterInit( list_iter, &this->ident_list );
}

/*
  ==============================
  TexRes::getTexIdentByIdent

  ==============================
*/
texident_t * TexRes::getTexIdentByIdent( const char *ident, bool ignore_multilayer )
{
	texident_t	*ti;

	ti = NULL;

	if ( !ignore_multilayer )
	{
		ti = (texident_t *) U_MapSearch( &this->multilayer_map, (void *) ident );
	}

	if ( !ti )
	{
		ti = (texident_t *) U_MapSearch( &this->gltexres_map, (void *) ident );
	}

	if ( !ti )
	{
		printf( "WARNING: getTexIdentByIdent - ident '%s' not found\n", ident );
	}
	else if ( !ti->image )
	{
		printf( "WARNING: getTexIdentByIdent - (null) image for ident '%s'\n", ident );
	}
	
	return ti;
}
