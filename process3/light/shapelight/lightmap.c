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
			if ( p->intens_diffuse > 0.0 )
				num++;
		}
		else if ( which == PatchColor_specular )
		{
			if ( p->intens_specular > 0.0 )
				num++;
		}
	}

	return num;		
}


void DrawLightDataPixel( unsigned char *base, unsigned int width, unsigned int x, unsigned int y, fp_t intens )
{
	if ( intens > 1.0 )
		intens = 1.0;
	else if ( intens < 0.0 )
		intens = 0.0;

	base[x + width*y] = (unsigned char)(intens*255.0);
}

fp_t GetLightDataPixel( unsigned char *base, unsigned int width, unsigned int x, unsigned int y )
{
	return base[x+width*y] /255.0;
	
//	color[0] = (((pixel >> 11) & 31) << 3) / 255.0;
//	color[1] = (((pixel >> 5) & 63) << 2) / 255.0;
//	color[2] = (((pixel) & 31) << 3) / 255.0;

	
}



#if 0
void DrawPatchesIntoLightmap( unsigned short *base, unsigned int width, patchColor color, patch_t *patch_list )
{
	patch_t		*p;

	for ( p = patch_list; p ; p = p->next )
	{
		if ( color == PatchColor_diffuse )
		{
			DrawLightDataPixel( base, width, p->x, p->y, p->color );
		}
		else if ( color == PatchColor_specular )
		{
			DrawLightDataPixel( base, width, p->x, p->y, p->spec );
		}
		else
		{
			Error( "unknown patch color\n" );
		}
	}	
}
#endif

void SetupPixelMap( unsigned char *base, unsigned int width, patch_t *patch_list )
{
	patch_t		*p;       

	for ( p = patch_list; p ; p = p->next )
	{
		base[width*p->y+p->x] = 255;
	}	
}

hobj_t * BuildLightdef( face_t *f )
{
	fp_t		x, y;
	int		width, height;

	hobj_t		*lightdef;
	hpair_t		*pair;
	
	vec2d_t		min, max;
	vec2d_t		space;
	vec2d_t		shift;

	char		tt[256];

	shift[0] = f->wmin[0];
	shift[1] = f->wmin[1];
	
	for ( y = f->wmin[1], height = 0; y <= f->wmax[1]; y+=f->patchsize, height++ )
	{ }
//	height++;

	for ( x = f->wmin[0], width = 0; x <= f->wmax[0]; x+=f->patchsize, width++ )
	{ }
//	width++;

//	printf( "(%d,%d)\n", width, height );

	f->width = width;
	f->height = height;

	sprintf( tt, "#%u", HManagerGetFreeID() );
	lightdef = NewClass( "proj_lightdef", tt );

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


hobj_t * BuildUVLightdef( bsurface_t *cs )
{
	hobj_t		*texdef;	

	texdef = EasyNewClass( "uv_lightdef" );

	EasyNewInt( texdef, "width", cs->width );
	EasyNewInt( texdef, "height", cs->height );
	
	return texdef;
}

void BlurLightDataPixels( unsigned char *base, int width, int height )
{
	int	x, y;
	int		left, right, up, down;
	fp_t		sum;
	int		num;
	fp_t		img[256][256];

	if ( width < 5 || height < 5 )
		return;

	for ( x = 0; x < width; x++ )
	{
		for ( y = 0; y < height; y++ )
		{
			img[x][y] = GetLightDataPixel( base, (unsigned int)width, x, y );
		}
	}

	for ( x = 0; x < width; x++ )
	{
		for ( y = 0; y < height; y++ )
		{
			sum = img[x][y];
			num = 1;

			left = ((x-1)>=0)?1:0;
			right = ((x+1)<width)?1:0;
			up = ((y-1)>=0)?1:0;
			down = ((y+1)<height)?1:0;

			if ( left && up )
			{
				sum += img[x-1][y-1];
				num++;
			}
			if ( up )
			{
				sum += img[x-0][y-1];
				num++;
			}
			if ( right && up )
			{
				sum += img[x+1][y-1];
				num++;
			}
			if ( left )
			{
				sum += img[x-1][y];
				num++;
			}
			if ( right )
			{
				sum += img[x+1][y];
				num++;
			}
			if ( left && down )
			{
				sum += img[x-1][y+1];
				num++;
			}
			if ( down )
			{
				sum += img[x][y+1];
				num++;
			}
			if ( right && down )
			{
				sum += img[x+1][y+1];
				num++;
			}

			sum *= 1.0/(fp_t)(num);
			DrawLightDataPixel( base, width, x, y, sum );
		}
	}
}


void SetupCSurfLightMaps( bsurface_t *cs, unsigned char *c_map, unsigned char *s_map )
{
	patch_t		*p;
	
	for ( p = cs->patches; p ; p=p->next )
	{
		DrawLightDataPixel( c_map, cs->width, p->x, p->y, p->intens_diffuse );
		DrawLightDataPixel( s_map, cs->width, p->x, p->y, p->intens_specular );
	}
	BlurLightDataPixels( c_map, cs->width, cs->height );
	BlurLightDataPixels( s_map, cs->width, cs->height );
}

void GetFuzzyWorldPatch( cplane_t *pl, int x, int y, fp_t patchsize, fp_t xcov, fp_t ycov, fp_t *c_comp, fp_t *s_comp )
{
	fp_t		x1cov, y1cov;

	fp_t		total;

	x1cov = 1.0-xcov;
	y1cov = 1.0-ycov;

	if ( x1cov == 1.0 && y1cov == 1.0 )
	{
		Balance_GetWorldPatch( pl, x, y, patchsize, c_comp, s_comp );	
	}
	else
	{
		int		i;
		fp_t		s[4], c[4];
		fp_t		scale[4];
		
		*s_comp = 0.0;
		*c_comp = 0.0;

		total = 0.0;
		
		for ( i = 0; i < 4; i++ )
			scale[i] = 0.0;

		if ( Balance_GetWorldPatch( pl, x, y, patchsize, &c[0], &s[0] ) )
		{
			scale[0] = x1cov*y1cov;
			total += scale[0];
		}
		
		if ( Balance_GetWorldPatch( pl, x, y+1, patchsize, &c[1], &s[1] ) )
		{
			scale[1] = x1cov*ycov;
			total += scale[1];
		}

		if ( Balance_GetWorldPatch( pl, x+1, y, patchsize, &c[2], &s[2] ) )
		{
			scale[2] = xcov*y1cov;
			total += scale[2];
		}		
		
		if ( Balance_GetWorldPatch( pl, x+1, y+1, patchsize, &c[3], &s[3] ) )
		{
			scale[3] = xcov*ycov;
			total += scale[3];
		}	

		if ( total > 0.0 )
		{
			total = 1.0/total;

			for ( i = 0; i < 4; i++ )
			{
				if ( scale[i] != 0.0 )
				{
					scale[i] *= total;
					c[i] *= scale[i];
					s[i] *= scale[i];
					*c_comp += c[i];
					*s_comp += s[i];
				}
			}
		}
	}
}

void SetupLightMaps( face_t *f, unsigned char *c_map, unsigned char *s_map )
{
	fp_t		x, y;
	int		lx, ly;
	int		width, height;
	int		xx, yy;
	fp_t		c_comp, s_comp;
	
	fp_t		x_ofs;
	fp_t		y_ofs;

	x_ofs = f->wmin[0] - floor( f->wmin[0]/f->patchsize )*f->patchsize;
	y_ofs = f->wmin[1] - floor( f->wmin[1]/f->patchsize )*f->patchsize;

	x_ofs /= f->patchsize;
	y_ofs /= f->patchsize;

//	printf( "%f, %f\n", x_ofs, y_ofs );

	for ( ly = 0, y = f->wmin[1]; y < f->wmax[1]; y+=f->patchsize, ly++ )
	{
		for ( lx = 0, x = f->wmin[0]; x < f->wmax[0]; x+=f->patchsize, lx++ )
		{
			xx = (int) floor(x/f->patchsize);
			yy = (int) floor(y/f->patchsize);
			
			GetFuzzyWorldPatch( f->pl, xx, yy, f->patchsize, x_ofs, y_ofs, &c_comp, &s_comp );
			
			DrawLightDataPixel( c_map, f->width, lx, ly, c_comp );					
			DrawLightDataPixel( s_map, f->width, lx, ly, s_comp );	
		}
	}
}

void FaceListBuildLightmaps( u_list_t *face_list, hobj_t *source )
{
	u_list_iter_t		iter;
	face_t			*f;

	int		i;

	int		diffuse_num = 0;
	int		specular_num = 0;

	unsigned char	c_map[128*128+256];	// keeps the colors
	unsigned char	s_map[128*128+256];	// keeps the colors
	unsigned char	pixel_map[128*128];	// 0 = no pixel set, 255 = pixel set by DrawPatchesIntoLightmap


	U_ListIterInit( &iter, face_list );
	for ( ; ( f = U_ListIterNext( &iter ) ) ; )
	{	
		memset( c_map, 0, f->width*f->height );
		memset( s_map, 0, f->width*f->height );
		SetupLightMaps( f, c_map, s_map );

		for ( i = 0; i < f->width*f->height; i++ )
		{
			if ( c_map[i] != 0 )
			{
				Lightmap_Add( (void*)(c_map), f->width*f->height, f->shape, source, 0 ); 
				diffuse_num++;
				break;
			}
		}
		
		for ( i = 0; i < f->width*f->height; i++ )
		{
			if ( s_map[i] != 0 )
			{
				Lightmap_Add( (void*)(s_map), f->width*f->height, f->shape, source, 1 ); 
				specular_num++;	
				break;
			}
		}
	}	
}

void CSurfListBuildLightmaps( u_list_t *csurf_list, hobj_t *source )
{
	u_list_iter_t		iter;
	bsurface_t		*cs;

	int		i;

	int		diffuse_num = 0;
	int		specular_num = 0;

	unsigned char	c_map[128*128+256];	// keeps the colors
	unsigned char	s_map[128*128+256];	// keeps the colors

	U_ListIterInit( &iter, csurf_list );
	for ( ; ( cs = U_ListIterNext( &iter ) ) ; )
	{
		memset( c_map, 0, cs->width*cs->height );
		memset( s_map, 0, cs->width*cs->height );

		SetupCSurfLightMaps( cs, c_map, s_map );

		for ( i = 0; i < cs->width*cs->height; i++ )
		{
			if ( c_map[i] != 0 )
			{
				Lightmap_Add( (void*)(c_map), cs->width*cs->height, cs->shape, source, 0 ); 
				diffuse_num++;
				break;
			}
		}
		
		for ( i = 0; i < cs->width*cs->height; i++ )
		{
			if ( s_map[i] != 0 )
			{
				Lightmap_Add( (void*)(s_map), cs->width*cs->height, cs->shape, source, 1 ); 
				specular_num++;	
				break;
			}
		}			
	}
}

void FaceListBuildLightdefs( face_t *f_head )
{
	face_t		*f;
	hobj_t		*lightdef;

	printf( "build projective lightdefs ...\n" );

	for ( f = f_head; f ; f=f->next )
	{
		if ( f->mat.no_light || f->mat.self_light )
			continue;

		lightdef = BuildLightdef( f );
		InsertClass( f->shape, lightdef );
		f->lightdef = lightdef;
	}
}

void CSurfListBuildLightdefs( bsurface_t *head )
{
	bsurface_t	*cs;
	hobj_t		*lightdef;

	printf( "build uv lightdefs ...\n" );
	
	for ( cs = head; cs ; cs=cs->next )
	{
		if ( cs->mat.no_light || cs->mat.self_light )
			continue;

		lightdef = BuildUVLightdef( cs );
		InsertClass( cs->shape, lightdef );
		cs->lightdef = lightdef;
	}
}


/*
  =============================================================================
  lightmap binary

  =============================================================================
*/

static FILE	*g_h = NULL;
static FILE	*g_h2 = NULL;
static hobj_t	*g_lightmap;
static int	 g_ofs = 0;

void Lightmap_Begin( char *bin_name, char *cls_name )
{
	g_h = fopen( bin_name, "w" );
	if ( !g_h )
		Error( "Lightmap_Begin: binary open failed\n" );

	g_h2 = fopen( cls_name, "w" );
	if ( !g_h2 )
		Error( "Lightmap_Begin: class open failed\n" );
	
	g_lightmap = NewClass( "lightmaps", "lightmaps0" );

	g_ofs = 0;
}

void Lightmap_Add( void *data, int size, hobj_t *shape, hobj_t *source, int type )
{
	int		ofs;
	hobj_t		*cls;
	char		tt[256];

	size = (size&(~3))+4;

	ofs = g_ofs; 

	fwrite( data, size, 1, g_h );
	g_ofs += size;

	sprintf( tt, "#%u", HManagerGetFreeID() );
	cls = NewClass( "lightmap", tt );
	InsertClass( g_lightmap, cls );
	EasyNewInt( cls, "ofs", ofs );
	EasyNewInt( cls, "type", type );
	EasyNewClsref( cls, "shape_id", shape );
	EasyNewClsref( cls, "source_id", source );
}

void Lightmap_End( void )
{
	fclose( g_h );
	
	WriteClass( g_lightmap, g_h2 );
	fclose( g_h2 );
}
