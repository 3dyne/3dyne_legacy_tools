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



// r_texture.c

#include "render.h"

int		r_texturenum;
texture_t	r_textures[MAX_TEXTURES];

void CompileTextureClass( hmanager_t *texturehm )
{
	hobj_search_iterator_t	iter;
	hobj_t		*texture;
	hpair_t		*pair;
	char		tt[256];

	r_texturenum = 0;
	InitClassSearchIterator( &iter, HManagerGetRootClass( texturehm ), "texture" );
	for ( ; ( texture = SearchGetNextClass( &iter ) ) ; )
	{
		if ( r_texturenum == MAX_TEXTURES )
			Error( "reached MAX_TEXTURES\n" );

		r_textures[r_texturenum].self = texture;

		// set index
		sprintf( tt, "%d", r_texturenum );
		pair = NewHPair2( "int", "index", tt );
		InsertHPair( texture, pair );

		pair = FindHPair( texture, "ident" );
		if ( !pair )
			Error( "missing 'ident' in texture '%s'.\n", texture->name );
		
		{
			FILE		*h;

			sprintf( tt, "%s/%s.arr", TEXTURE_PATH, pair->value );
			h = fopen( tt, "r" );
			if ( !h )
				Error( "can't open file '%s'\n", tt );
			
			r_textures[r_texturenum].arr = ARR_Read( h );
			
			fclose( h );			
		}	       
		
		if ( strstr( pair->value, "sky" ) )
		{
			printf( " found sky texture\n" );
			r_textures[r_texturenum].is_sky = true;
		}
		else
		{
			r_textures[r_texturenum].is_sky = false;
		}

		r_texturenum++;
	}

	printf( " %d textures\n", r_texturenum );

	SetupTextures();
}

void Texture_GenTexture( texture_t *tex )
{
	int		width;
	int		height;
	unsigned short	*image565;
	unsigned char	*image888;

	width = tex->arr->size_x;
	height = tex->arr->size_y;

	tex->inv_width = 1.0/width;
	tex->inv_height = 1.0/height;

	image565 = (unsigned short *) tex->arr->data;

	image888 = Image565ToImage888( image565, width*height );

	glGenTextures( 1, &tex->texobj );
	glBindTexture( GL_TEXTURE_2D, tex->texobj );

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);

	gluBuild2DMipmaps( GL_TEXTURE_2D, GL_RGB, width, height, GL_RGB, GL_UNSIGNED_BYTE, image888);	
}

void SetupTextures( void )
{
	int		i;

	for ( i = 0; i < r_texturenum; i++ )
	{
		Texture_GenTexture( &r_textures[i] );
	}
}
