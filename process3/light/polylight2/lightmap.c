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



// lightmap.c

#include "light.h"


/*
  ==================================================
  image stuff

  ==================================================
*/

#define IMAGE_WIDTH		( 256 )
#define IMAGE_HEIGHT		( 256 )

#define PIXEL_SIZE		( 16.0 )

#define PIXELMODE_REPLACE	( 1 )
#define PIXELMODE_MIX		( 2 )

typedef struct pixel_s {
	vec3d_t		color;
	int		count;
	int		kind;	// 0 = normal, 1 = by clamp
} pixel_t;

typedef struct image_s {
	int		width, height;	// in pixel
	int		wmask, hmask;
	int		ps;	// pixel_size
	int		psmask;
	fp_t		wreal, hreal;	// in pixel*pixel_size
	vec2d_t		min, max;
	int		clearnum;
	pixel_t		pixels[256];	// variable
} image_t;


#define _PIXEL_SIZE	( 16 )
#define _IMAGE_WIDTH	( 256 )
#define _IMAGE_HEIGHT	( 256 )

image_t* NewImage( int width, int height, int pixel_size )
{
	int		size;
	image_t		*img;

	size = (int)&(((image_t *)0)->pixels[width*height]);
	img = (image_t *) malloc( size );

	memset( img, 0, size );
	
	img->width = width;
	img->height = height;
	img->wmask = width-1;
	img->hmask = height-1;
	img->ps = pixel_size;
	img->psmask = pixel_size-1;
	img->wreal = width*pixel_size;
	img->hreal = height*pixel_size;
	img->clearnum = 1;

	Vec2dInitBB( img->min, img->max, 999999.9 );

	return img;
}

void ClearImage( image_t *img )
{
	Vec2dInitBB( img->min, img->max, 999999.9 );
	img->clearnum++;
}

void FreeImage( image_t *img )
{
	free( img );
}


void Vec2dToPixelPos( image_t *img, int *x, int *y, vec2d_t pos )
{
	*x = (int)(floor( pos[0] / /*img->ps*/_PIXEL_SIZE ) );
	*y = (int)(floor( pos[1] / /*img->ps*/_PIXEL_SIZE ) );
}

void PixelPosToVec2d( image_t *img, vec2d_t pos, int x, int y )
{
	pos[0] = (fp_t)( x * /*img->ps*/_PIXEL_SIZE + /*img->ps*/_PIXEL_SIZE/2.0 );
	pos[1] = (fp_t)( y * /*img->ps*/_PIXEL_SIZE + /*img->ps*/_PIXEL_SIZE/2.0 );
}



fp_t PixelCovering( image_t *img, int x1, int y1, int x2, int y2 )
{
	int		dx, dy;

	dx = abs( x2 - x1 );
	dy = abs( y2 - y1 );

	if ( dx > /*img->ps*/_PIXEL_SIZE )
		return 0.0;

	if ( dy > /*img->ps*/_PIXEL_SIZE )
		return 0.0;

	dx = img->ps - dx;
	dy = img->ps - dy;

	return (fp_t)dx*dy / (fp_t)(/*img->ps*img->ps*/_PIXEL_SIZE*_PIXEL_SIZE);
}


void SetPixel( image_t *img, vec2d_t pos, vec3d_t color, int mode )
{
	int		x, y;
	int		w;
	vec3d_t		c;

//	Vec2dPrint( pos );
	// check valid image size
	Vec2dAddToBB( img->min, img->max, pos );
#if 1
	if ( img->max[0] - img->min[0] >= img->wreal ||
	     img->max[1] - img->min[1] >= img->hreal )
		//	Error( "SetPixel: out of area.\n" );
		Vec3dInit( color, 1.0, 0, 0 );
#endif	

	Vec2dToPixelPos( img, &x, &y, pos );
	
	w = img->width;
	x = x & (_IMAGE_WIDTH-1) /*img->wmask*/;
	y = y & (_IMAGE_HEIGHT-1) /*img->hmask*/;

	// first accsess


	if ( mode == PIXELMODE_REPLACE )
	{
		if ( img->pixels[x+y*_IMAGE_WIDTH].count != img->clearnum )
		{
			img->pixels[x+y*_IMAGE_WIDTH].count = img->clearnum;
			img->pixels[x+y*_IMAGE_WIDTH].kind = 0;
//			Vec3dInit( img->pixels[x+y*img->width].color, 0.0, 0.0, 0.0 );
		}
		Vec3dCopy( img->pixels[x+y*_IMAGE_WIDTH].color, color );
	}

	if ( mode == PIXELMODE_MIX )
	{
		if ( img->pixels[x+y*_IMAGE_WIDTH].count != img->clearnum ||
		     img->pixels[x+y*_IMAGE_WIDTH].kind == 1 )
		{
			// pixel-color by clamp or pixel not used => replace
			Vec3dCopy( img->pixels[x+y*_IMAGE_WIDTH].color, color );
		}
		else
		{
			// mix
			Vec3dAdd( c, color, img->pixels[x+y*_IMAGE_WIDTH].color );
			Vec3dScale( img->pixels[x+y*_IMAGE_WIDTH].color, 0.5, c );
		}

		img->pixels[x+y*_IMAGE_WIDTH].kind = 0;
		img->pixels[x+y*_IMAGE_WIDTH].count = img->clearnum;
	}
}

void GetPixel_old( image_t *img, vec3d_t color, vec2d_t pos )
{
	int		x, y, w;

	Vec2dToPixelPos( img, &x, &y, pos );

	w = img->width;
	x = x & img->wmask;
	y = y & img->hmask;

	// first accsess
	if ( img->pixels[x+y*img->width].count != img->clearnum )
	{	
		Vec3dInit( color, 0, 0, 0 );
		return;
	}

	Vec3dCopy( color, img->pixels[x+y*img->width].color );
}

void GetPixel( image_t *img, vec3d_t color, vec2d_t pos )
{
	int		x, y, w;
	int		ofs[8] = { 0, 0,
				    0, 1,
				    1, 0,
				    1, 1 };

	int		x1, y1, x2, y2, tx, ty;
	int		i;
	fp_t		scale;

	Vec2dToPixelPos( img, &x, &y, pos );
	Vec3dInit( color, 0,0,0 );

	w = img->width;


	x1 = (int)(((int)floor( pos[0] ))) & /*img->psmask*/ (_PIXEL_SIZE-1);
	y1 = (int)(((int)floor( pos[1] ))) & /*img->psmask*/ (_PIXEL_SIZE-1);

	x = x & /*img->wmask*/ (_IMAGE_WIDTH-1);
	y = y & /*img->hmask*/ (_IMAGE_HEIGHT-1);
//	x1 = y1 = 8;
	
//	printf( "%d,%d ", x1, y1 );

	for ( i = 0; i < 4; i++ )
	{
		tx = ( x + ofs[i*2] ) & /*img->wmask*/ (_IMAGE_WIDTH-1);
		ty = ( y + ofs[i*2+1] ) & /*img->hmask*/ (_IMAGE_HEIGHT-1);

		x2 = ofs[i*2]*16;
		y2 = ofs[i*2+1]*16;	       

		if ( img->pixels[tx+ty*_IMAGE_WIDTH].count != img->clearnum )
			continue;

		scale = PixelCovering( img, x1, y1, x2, y2 );
//		printf( "%f ", scale );
		Vec3dMA( color, scale, img->pixels[tx+ty*_IMAGE_WIDTH].color, color );
			
//		if ( img->pixels[tx+ty*img->width].count != img->clearnum )
//			continue;

	}
//	printf( "\n" );

}

void ClampImage( image_t *img )
{
	int		x, y;
	int		tx, ty;
	int		i;
	int		count;
	vec3d_t		color;
	fp_t		*c;
	fp_t		scale;
	int		pxl, pxl2;
	int		ofs[16] = { -1, -1,
				    0, -1,
				    1, -1,
				    -1, 0,
				    1, 0,
				    -1, 1,
				    0, 1,							    
				    1, 1 };

	int	xmin, xmax;
	int	ymin, ymax;

	Vec2dToPixelPos( img, &xmin, &ymin, img->min );
	Vec2dToPixelPos( img, &xmax, &ymax, img->max );


	if ( img->width != _IMAGE_WIDTH || img->height != _IMAGE_HEIGHT || img->ps != 16 )
		Error( "ClampImage: sorry, optimized for 64x64 at 16 pixelsize.\n" );
	

	for ( x = xmin-2; x <= xmax+2; x++ )
	{
		for ( y = ymin-2; y < ymax+2; y++ )
		{
			pxl = (x&(_IMAGE_WIDTH-1))+(y&(_IMAGE_HEIGHT-1))*_IMAGE_WIDTH;

			if ( img->pixels[pxl].count == img->clearnum )
				continue;

//			Vec3dInit( img->pixels[x+y*img->width].color, 0.5, 0.0, 0.0 );
//			img->pixels[x+y*img->width].count = img->clearnum;
//			continue;

			// check neighborhood
			_Vec3dInit( color, 0,0,0 );
			count = 0;
			for( i = 0; i < 8; i++ )
			{
				tx = (x+ofs[i*2]) & (_IMAGE_WIDTH-1);
				ty = (y+ofs[i*2+1]) & (_IMAGE_HEIGHT-1);
				pxl2 = tx+ty*_IMAGE_WIDTH;
				if ( img->pixels[pxl2].count != img->clearnum ||
				     img->pixels[pxl2].kind == 1 )
					continue;
				c = img->pixels[pxl2].color;
				_Vec3dAdd( color, color, c /*img->pixels[pxl2].color*/ );
				count++;
			}
			if ( count < 1 )
				continue;

			scale = 1.0/(fp_t)count;
			_Vec3dScale( color, scale, color );
			c = img->pixels[pxl].color;
			_Vec3dCopy( c, color );
//			img->pixels[x+y*img->width].color[0] = 0.5;
			img->pixels[pxl].count = img->clearnum;
			img->pixels[pxl].kind = 1;
		}
	}
}

/*
  ==================================================
  better image stuff

  ==================================================
*/
typedef enum
{
	ImgCellType_none = 0,
	ImgCellType_set,
	ImgCellType_pump
} imgCellType;

typedef struct imgcell_s
{
	int		x, y;
	vec3d_t		color;
	imgCellType	type;
	struct imgcell_s		*next;	// hash
} imgcell_t;

#define IMGMAP_HASHSIZE		( 512 )

typedef struct imgmap_s
{
	int		totalpixelnum;
	int		pixelnum[IMGMAP_HASHSIZE];
	imgcell_t	*hash[IMGMAP_HASHSIZE];
} imgmap_t;

imgmap_t * NewImgMap( void )
{
	imgmap_t	*img;
	img = NEW( imgmap_t );
	return img;	
}

void FreeImgMap( imgmap_t *img )
{
	free( img );
}

imgcell_t * NewImgCell( void )
{
	imgcell_t	*c;
	c = NEW( imgcell_t );
	return c;
}

void FreeImgCell( imgcell_t *c )
{
	free( c );
}

// low level imgmap stuff

void ImgMap_Clear( imgmap_t *img )
{
	int		i;
	imgcell_t		*c, *cnext;

	for ( i = 0; i < IMGMAP_HASHSIZE; i++ )
	{
		for ( c = img->hash[i]; c ; c=cnext )
		{
			cnext = c->next;
			FreeImgCell( c );
		}
		img->hash[i] = NULL;
	}
}

int ImgMap_CalcHashKey( int x, int y )
{
	int		key;
	key = x + ( y << 4 );
	key &= (IMGMAP_HASHSIZE-1);
	return key;
}

void DumpImgMap( imgmap_t *img )
{
	int		i;
	for ( i = 0; i < IMGMAP_HASHSIZE; i++ )
		printf( "%d ", img->pixelnum[i] );
	printf( "\n" );
}

imgcell_t * ImgMap_FindCell( imgmap_t *img, int x, int y )
{
	int		key;
	imgcell_t	*c;

	key = ImgMap_CalcHashKey( x, y );
	for ( c = img->hash[key]; c ; c=c->next )
	{
		if ( c->x == x && c->y == y )
			return c;
	}
	return NULL;
}

void ImgMap_InsertCell( imgmap_t *img, imgcell_t *c )
{
	int		key;
	key = ImgMap_CalcHashKey( c->x, c->y );
	c->next = img->hash[key];
	img->hash[key] = c;
	img->pixelnum[key]++;
	img->totalpixelnum++;
}

// imgmap pixel stuff

fp_t ImgMap_PixelCovering( vec2d_t pos, vec2d_t snap )
{
	vec2d_t		d;

	d[0] = fabs( pos[0]-snap[0] );
	d[1] = fabs( pos[1]-snap[1] );

	if ( d[0] >= 1.0 || d[1] >= 1.0 )
		return 0.0;

	return d[0]*d[1];
}

void ImgMap_SetPixel( imgmap_t *img, int x, int y, vec3d_t color )
{
	imgcell_t	*c;
	c = ImgMap_FindCell( img, x, y );
	if ( !c )
	{
		c = NewImgCell();
		c->x = x;
		c->y = y;
		c->type = ImgCellType_set;
		Vec3dCopy( c->color, color );
		ImgMap_InsertCell( img, c );
	}
	else
	{
		Vec3dAdd( c->color, c->color, color );
	}
}

bool_t ImgMap_GetPixel( imgmap_t *img, int x, int y, vec3d_t color )
{
	imgcell_t	*c;
	c = ImgMap_FindCell( img, x, y );
	if ( !c )
	{
		Vec3dInit( color, 0, 0, 0 );
		return false;
	}
	else
	{
		Vec3dCopy( color, c->color );
		return true;
	}
}

void ImgMap_SetFuzzyPixel( imgmap_t *img, vec2d_t pos, vec3d_t color )
{
	vec2d_t		snap;
	int		sx, sy;
	fp_t		cover;
	vec3d_t		color2;
	
	snap[0] = floor( pos[0] );
	snap[1] = floor( pos[1] );

	sx = (int)snap[0];
	sy = (int)snap[1];

	cover = ImgMap_PixelCovering( pos, snap );
	Vec3dScale( color2, cover, color );
	ImgMap_SetPixel( img, sx, sy, color2 );
	
	if ( cover != 1.0 )
	{
		// upper right
		snap[0]+=1.0;
		sx++;
		cover = ImgMap_PixelCovering( pos, snap );
		Vec3dScale( color2, cover, color );
		ImgMap_SetPixel( img, sx, sy, color2 );

		// lower right
		snap[1]+=1.0;
		sy++;
		cover = ImgMap_PixelCovering( pos, snap );
		Vec3dScale( color2, cover, color );
		ImgMap_SetPixel( img, sx, sy, color2 );

		// lower left
		snap[0]-=1.0;
		sx--;
		cover = ImgMap_PixelCovering( pos, snap );
		Vec3dScale( color2, cover, color );
		ImgMap_SetPixel( img, sx, sy, color2 );
	}
}

void ImgMap_GetFuzzyPixel( imgmap_t *img, vec2d_t pos, vec3d_t color )
{
	vec2d_t		snap;
	int		sx, sy;
	fp_t		cover;
	vec3d_t		color2;
	
	snap[0] = floor( pos[0] );
	snap[1] = floor( pos[1] );

	sx = (int)snap[0];
	sy = (int)snap[1];

	cover = ImgMap_PixelCovering( pos, snap );
	ImgMap_GetPixel( img, sx, sy, color2 );
	Vec3dScale( color, cover, color2 );
	
	if ( cover != 1.0 )
	{
		// upper right
		snap[0]+=1.0;
		sx++;
		cover = ImgMap_PixelCovering( pos, snap );
		ImgMap_GetPixel( img, sx, sy, color2 );
		Vec3dMA( color, cover, color2, color );

		// lower right
		snap[1]+=1.0;
		sy++;
		cover = ImgMap_PixelCovering( pos, snap );
		ImgMap_GetPixel( img, sx, sy, color2 );
		Vec3dMA( color, cover, color2, color );

		// lower left
		snap[0]-=1.0;
		sx--;
		cover = ImgMap_PixelCovering( pos, snap );
		ImgMap_GetPixel( img, sx, sy, color2 );
		Vec3dMA( color, cover, color2, color );
	}	
}

void ImgMap_CreatePumpPixel( imgmap_t *img, int x, int y )
{
	int		i, count;
	vec3d_t		color;
	imgcell_t	*c;

	int	ofs[16] = { -1, -1,
			    0, -1,
			    1, -1,
			    -1, 0,
			    1, 0,
			    -1, 1,
			    0, 1,
			    1, 1 };

	Vec3dInit( color, 0, 0, 0 );
	count = 0;
	for ( i = 0; i < 8; i++ )
	{
		c = ImgMap_FindCell( img, x+ofs[i*2], y+ofs[i*2+1] );
		if ( !c )
			continue;
		if ( c->type == ImgCellType_set )
		{
			Vec3dAdd( color, color, c->color );
			count++;
		}
	}

	c = NewImgCell();
	c->x = x;
	c->y = y;
	c->type = ImgCellType_pump;
	if ( count == 0 )
		Vec3dInit( c->color, 0, 0, 0 );
	else
		Vec3dScale( c->color, 1.0/(fp_t)count, color ); 
	
	ImgMap_InsertCell( img, c );
}

void ImgMap_Pump( imgmap_t *img )
{
	int		i, j;
	imgcell_t	*c;

	fprintf( stderr,".");

	for( i = 0; i < IMGMAP_HASHSIZE; i++ )
	{
	restart_line:
		for( c = img->hash[i]; c ; c=c->next )
		{
			int	x, y;
			int	ofs[16] = { -1, -1,
					    0, -1,
					    1, -1,
					    -1, 0,
					    1, 0,
					    -1, 1,
					    0, 1,
					    1, 1 };
			x = c->x;
			y = c->y;

			for ( j = 0; j < 8; j++ )
			{
//				n = ImgMap_FindCell( img, x+ofs[i*2], y+ofs[i*2+1] );
				if ( !ImgMap_FindCell( img, x+ofs[j*2], y+ofs[j*2+1] ) )
				{
					ImgMap_CreatePumpPixel( img, x+ofs[j*2], y+ofs[j*2+1] );
					goto restart_line;
				}
				
			}			
		}
	}
}

/*
  ==================================================
  lightmap

  ==================================================
*/

typedef enum
{
	PatchColor_diffuse,
	PatchColor_specular
} patchColor;


int CheckPatchValues( patch_t *list, patchColor which )
{
	patch_t		*p;
	int		num;

	num = 0;

	for ( p = list; p ; p=p->next )
	{
		if ( which == PatchColor_diffuse )
		{
			if ( p->color[0] > 0.0 || p->color[1] > 0.0 || p->color[2] > 0.0 )
				num++;
		}
		else if ( which == PatchColor_specular )
		{
			if ( p->spec[0] > 0.0 || p->spec[1] > 0.0 || p->spec[2] > 0.0 )
				num++;
		}
	}

	return num;		
}


void DrawLightDataPixel( unsigned short *base, unsigned int width, unsigned int x, unsigned int y, vec3d_t color )
{
	unsigned short  *ptr;
	unsigned int    r, g, b;

	ptr = base;

	r = (unsigned int)(color[0] * 255.0 );
	g = (unsigned int)(color[1] * 255.0 );
	b = (unsigned int)(color[2] * 255.0 );

	r >>= 3;
	g >>= 2;
	b >>= 3;

	r <<= 11;
	g <<= 5;

	ptr[x + width*y] = r | g | b;
}

void GetLightDataPixel( unsigned short *base, unsigned int width, unsigned int x, unsigned int y, vec3d_t color )
{
	unsigned short	*ptr;
	unsigned short	pixel;

	pixel = base[x+width*y];
	
	color[0] = (((pixel >> 11) & 31) << 3) / 255.0;
	color[1] = (((pixel >> 5) & 63) << 2) / 255.0;
	color[2] = (((pixel) & 31) << 3) / 255.0;
}

void BlurLightDataPixels( unsigned short *base, int width, int height )
{
	int	x, y;
	int		left, right, up, down;
	vec3d_t		sum;
	int		num;
	vec3d_t		img[256][256];

	if ( width < 5 || height < 5 )
		return;

	for ( x = 0; x < width; x++ )
	{
		for ( y = 0; y < height; y++ )
		{
			GetLightDataPixel( base, (unsigned int)width, x, y, img[x][y] );
		}
	}

	for ( x = 0; x < width; x++ )
	{
		for ( y = 0; y < height; y++ )
		{
			Vec3dCopy( sum, img[x][y] );
			num = 1;

			left = ((x-1)>=0)?1:0;
			right = ((x+1)<width)?1:0;
			up = ((y-1)>=0)?1:0;
			down = ((y+1)<height)?1:0;

			if ( left && up )
			{
				Vec3dAdd( sum, sum, img[x-1][y-1] );
				num++;
			}
			if ( up )
			{
				Vec3dAdd( sum, sum, img[x-0][y-1] );
				num++;
			}
			if ( right && up )
			{
				Vec3dAdd( sum, sum, img[x+1][y-1] );
				num++;
			}
			if ( left )
			{
				Vec3dAdd( sum, sum, img[x-1][y] );
				num++;
			}
			if ( right )
			{
				Vec3dAdd( sum, sum, img[x+1][y] );
				num++;
			}
			if ( left && down )
			{
				Vec3dAdd( sum, sum, img[x-1][y+1] );
				num++;
			}
			if ( down )
			{
				Vec3dAdd( sum, sum, img[x][y+1] );
				num++;
			}
			if ( right && down )
			{
				Vec3dAdd( sum, sum, img[x+1][y+1] );
				num++;
			}

			Vec3dScale( sum, 1.0/num, sum );
			DrawLightDataPixel( base, width, x, y, sum );
		}
	}
}

hobj_t * BuildLightdef( face_t *f, hobj_t *lightdefs )
{
	fp_t		x, y;
	int		width, height;

	hobj_t		*lightdef;
	hpair_t		*pair;
	
	vec2d_t		min, max;
	vec2d_t		space;
	vec2d_t		shift;

	char		tt[256];

	Vec2dInit( space, f->patchsize/2.0, f->patchsize/2.0 );
	Vec2dSub( min, f->min2d, space );
	Vec2dAdd( max, f->max2d, space );

	shift[0] = f->min2d[0] - f->patchsize/2.0;
	shift[1] = f->min2d[1] - f->patchsize/2.0;

	for ( y = min[1], height = 0; y <= max[1]; y+=f->patchsize, height++ )
	{ }

	for ( x = min[0], width = 0; x <= max[0]; x+=f->patchsize, width++ )
	{ }


	sprintf( tt, "#%u", HManagerGetFreeID() );
	lightdef = NewClass( "lightdef", tt );
	InsertClass( lightdefs, lightdef );
	pair = NewHPair2( "ref", "lightdef", tt );
	InsertHPair( f->self, pair );

	sprintf( tt, "%f", f->patchsize );
	pair = NewHPair2( "float", "patchsize", tt );
	InsertHPair( lightdef, pair );

	sprintf( tt, "%d", width );
	pair = NewHPair2( "int", "width", tt );
	InsertHPair( lightdef, pair );

	sprintf( tt, "%d", height );
	pair = NewHPair2( "int", "height", tt );
	InsertHPair( lightdef, pair );

	sprintf( tt, "%f %f", shift[0], shift[1] );
	pair = NewHPair2( "vec2d", "shift", tt );
	InsertHPair( lightdef, pair );

	sprintf( tt, "%d", f->projection );
	pair = NewHPair2( "int", "projection", tt );
	InsertHPair( lightdef, pair );

	return lightdef;
}

void BuildLightmap( face_t *f, hobj_t *lightdef, /*imgmap_t*/image_t *img, char *lightmap_name )
{

	vec2d_t		v;
	fp_t		x, y;
	int		width, height;
	int		w, h;

	hpair_t		*pair;
	
	vec2d_t		min, max;
	vec2d_t		space;
	vec2d_t		shift;
	vec3d_t		c;

	fp_t		scale;

	unsigned short	lightmap[256*256];

	// hack, image needs pixelsize 1.0
	scale = 16.0 / f->patchsize;

	Vec2dInit( space, f->patchsize/2.0, f->patchsize/2.0 );
	Vec2dSub( min, f->min2d, space );
	Vec2dAdd( max, f->max2d, space );

	shift[0] = f->min2d[0] - f->patchsize/2.0;
	shift[1] = f->min2d[1] - f->patchsize/2.0;


	ClampImage( img );
//	ImgMap_Pump( img );

	for ( y = min[1], height = 0; y <= max[1]; y+=f->patchsize, height++ )
	{ }

	for ( x = min[0], width = 0; x <= max[0]; x+=f->patchsize, width++ )
	{ }

	memset( lightmap, 0x00, height*width*2 );
	
	for ( y = min[1], h = 0; y <= max[1]; y+=f->patchsize, h++ )
	{
		for ( x = min[0], w = 0; x <= max[0]; x+=f->patchsize, w++ )
		{
			v[0] = x*scale;
			v[1] = y*scale;
//			ImgMap_GetFuzzyPixel( img, v, c );
			GetPixel( img, c, v );
			DrawLightDataPixel( lightmap, width, w, h, c );
		}
	}
			

	
	pair = NewHPair2( "bstring", lightmap_name, "x" );
	BstringCastToHPair( lightmap, width*height*2, pair );
	InsertHPair( lightdef, pair );

}

void BuildLightmap_curved_surface( bsurface_t *cs )
{
	patch_t		*p;
	hpair_t		*pair;
	char		tt[256];
	unsigned short	lightmap[256*256];

	
	sprintf( tt, "%d", cs->width );
	pair = NewHPair2( "int", "width", tt );
	InsertHPair( cs->self, pair );

	sprintf( tt, "%d", cs->height );
	pair = NewHPair2( "int", "height", tt );
	InsertHPair( cs->self, pair );

	memset( lightmap, 0x00, cs->height*cs->width*2 );

	for ( p = cs->patches; p ; p=p->next )
	{
		DrawLightDataPixel( lightmap, cs->width, p->x, p->y, p->color );
	}
	BlurLightDataPixels( lightmap, cs->width, cs->height );

	pair = NewHPair2( "bstring", "diffuse", "x" );
	BstringCastToHPair( lightmap, cs->width*cs->height*2, pair );
	InsertHPair( cs->self, pair );


	if ( CheckPatchValues( cs->patches, PatchColor_specular ) )
	{
		memset( lightmap, 0x00, cs->height*cs->width*2 );
		
		for ( p = cs->patches; p ; p=p->next )
		{
			DrawLightDataPixel( lightmap, cs->width, p->x, p->y, p->spec );
		}
		BlurLightDataPixels( lightmap, cs->width, cs->height );		

		pair = NewHPair2( "bstring", "specular", "x" );
		BstringCastToHPair( lightmap, cs->width*cs->height*2, pair );
		InsertHPair( cs->self, pair );
	}
}


void DrawFacePatchesToImage( /*imgmap_t*/image_t *img, face_t *f, patchColor which )
{
	patch_t		*p;
	vec2d_t		v;

	fp_t		scale;

	// hack, image needs pixelsize 1.0
	scale = 16.0 / f->patchsize;

	for ( p = f->patches; p ; p=p->next )
	{
		ProjectVec3d( v, p->origin, f->projection );
		
		v[0]*=scale;
		v[1]*=scale;

		if ( which == PatchColor_diffuse )
//			ImgMap_SetFuzzyPixel( img, v, p->color );
			SetPixel( img, v, p->color, PIXELMODE_MIX );
		else if ( which == PatchColor_specular )
//			ImgMap_SetFuzzyPixel( img, v, p->spec );
			SetPixel( img, v, p->spec, PIXELMODE_MIX );
	}
}

/*
  ==================================================
  touch face stuff

  ==================================================
*/

#define		MAX_TOUCHFACES		( 1024 )
static int		touchnum;
static face_t		*touchfaces[MAX_TOUCHFACES];

bool_t CheckCollinearEdge( vec3d_t p1, vec3d_t p2, vec3d_t t )
{
	vec3d_t		v1, v2;

	Vec3dSub( v1, p1, t );
	Vec3dSub( v2, t, p2 );
	if ( Vec3dUnify( v1 ) < 0.01 ) return true;
	if ( Vec3dUnify( v2 ) < 0.01 ) return true;
	if ( Vec3dDotProduct( v1, v2 ) > 0.999 )
		return true;
	return false;
}

#define		COL_EPSILON	( 0.01 ) // 0.01

bool_t DoEdgesTouch( vec3d_t p1, vec3d_t p2, vec3d_t q1, vec3d_t q2 )
{
	vec3d_t		v;
	fp_t		e, l1, l2;
	int		value, value2;

	value = value2 = 0;

	Vec3dSub( v, p2, p1 );
	e = Vec3dLen( v );

	//
	// test q1
	//
	Vec3dSub( v, p1, q1 );
	l1 = Vec3dLen( v );
	Vec3dSub( v, p2, q1 );
	l2 = Vec3dLen( v );

	if ( fabs((l1 + l2) - e) <= COL_EPSILON )
	{
		value = 0;
	}
	else if ( l2 > e && l2 > l1 )
	{
		if ( fabs( e - (l2-l1) ) > COL_EPSILON )
			return false;
		value = -1;
	} 
	else if ( l1 > e && l1 > l2 )
	{
		if ( fabs( e - (l1-l2) ) > COL_EPSILON )
			return false;
		value = 1;
	}
	else
		return false;
//		printf( "p1\n" );

//	printf( "val1 %d\n", value );
	//
	// test q2
	//
	Vec3dSub( v, p1, q2 );
	l1 = Vec3dLen( v );
	Vec3dSub( v, p2, q2 );
	l2 = Vec3dLen( v );

	if ( fabs((l1 + l2) - e) <= COL_EPSILON )
	{
		value2 = 0;
	}
	else if ( l2 > e && l2 > l1 )
	{
		if ( fabs( e - (l2-l1) ) > COL_EPSILON )
			return false;
		value2 = -1;
	} 
	else if ( l1 > e && l1 > l2 )
	{
		if ( fabs( e - (l1-l2) ) > COL_EPSILON )
			return false;
		value2 = 1;
	}
	else
		return false;
//		printf( "p2\n" );
	
//	printf( "val2 %d\n", value2 );

	if ( abs( value+value2 ) > 1 )
		return false;

	return true;
}


bool_t DoEdgesTouch_old( vec3d_t p1, vec3d_t p2, vec3d_t q1, vec3d_t q2 )
{
//	return false;
	if ( CheckCollinearEdge( p1, p2, q1 ) || CheckCollinearEdge( p1, p2, q2 ) )
	     return true;
	else if ( CheckCollinearEdge( q1, q2, p1 ) || CheckCollinearEdge( q1, q2, p2 ) )
		return true;
	return false;
}

void FindTouchFaces( face_t *f, face_t *list )
{
	face_t		*f2;
	int		i, j, k;
	fp_t		*p1, *p2, *q1, *q2;

	touchnum = 0;
	for ( f2 = list; f2 ; f2=f2->next )
	{
		if ( f2 == f )
			continue;
		if ( f2->projection != f->projection )
			continue;

		for ( j = 0; j < 3; j++ )
		{
			if ( f->min3d[j]-1.0 > f2->max3d[j] || f->max3d[j]+1.0 < f2->min3d[j] )
				break;
		}
		if ( j != 3 )
			continue;


#if 0
		touchnum++;
#else
		for ( j = 0; j < f->p->pointnum; j++ )
		{
			p1 = f->p->p[j];
			p2 = f->p->p[(j+1 == f->p->pointnum) ? 0 : j+1 ];

			for ( k = 0; k < f2->p->pointnum; k++ )
			{
				q1 = f2->p->p[k];
				q2 = f2->p->p[ (k+1 == f2->p->pointnum) ? 0 : k+1 ];

				if ( DoEdgesTouch( p1, p2, q1, q2 ) )
				{
					if ( touchnum == MAX_TOUCHFACES )
						Error( "reached MAX_TOUCHFACES.\n" );
					touchfaces[touchnum++] = f2; 
				}
			}
		}
#endif
	}
}

void DrawTouchFacesPatchesToImage( /*imgmap_t*/image_t *img, patchColor which )
{
	int		i;

	for ( i = 0; i < touchnum; i++ )		
		DrawFacePatchesToImage( img, touchfaces[i], which );			
}



void BuildLightmaps( face_t *list, char *lightdef_name )
{
	hobj_t		*lightdefcls;
	FILE		*h;
	hobj_t		*lightdef;
//	imgmap_t		*img;
	image_t		*img;
	face_t		*f;

	int		diffuse_num = 0;
	int		specular_num = 0;

	printf( "build lightmaps ...\n" );

//	img = NewImgMap();
	img = NewImage( 256, 256, _PIXEL_SIZE );

	lightdefcls = NewClass( "lightdefs", "lightdef0" );

	for ( f = list; f ; f=f->next )
	{
		if ( f->mat.no_light )
			continue;

		lightdef = BuildLightdef( f, lightdefcls );

//		if ( !CheckFacePatches( list, PatchColor_diffuse ) )
//       			continue;	// no diffuse data in patches

		FindTouchFaces( f, list );
//		printf( "%d ", touchnum );

//		ImgMap_Clear( img );
		ClearImage( img );
		DrawFacePatchesToImage( img, f, PatchColor_diffuse );
		DrawTouchFacesPatchesToImage( img, PatchColor_diffuse );
		BuildLightmap( f, lightdef, img, "diffuse" );
		diffuse_num++;

		if ( !CheckPatchValues( f->patches, PatchColor_specular ) )
       			continue;	// no spec data in patches

//		ImgMap_Clear( img );
		ClearImage( img );
		DrawFacePatchesToImage( img, f, PatchColor_specular );
		DrawTouchFacesPatchesToImage( img, PatchColor_specular );
		BuildLightmap( f, lightdef, img, "specular" );
		specular_num++;
	}

	printf( " %d diffuse lightmaps\n", diffuse_num );
	printf( " %d specular lightmaps\n", specular_num );

	h = fopen( lightdef_name, "w" );
	if ( !h )
		Error( "can't open file.\n" );
	WriteClass( lightdefcls, h );
	fclose( h );
}


void BuildLightmaps_curved_surface( bsurface_t *list )
{
	bsurface_t	*cs;

	for ( cs = list; cs ; cs=cs->next )
	{
		BuildLightmap_curved_surface( cs );
	}
}
