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



// texture.c

#include <stdio.h>
#include <stdlib.h>

#include "cdb_service.h"
#include "texture.h"

#define MAX_CACHEDTEXTURES	( 1024 )

#define MAX_CACHEDTGA888	( 256 )

pal_t		*t_pal;

static char*	t_texturepath;

static int	t_texturenum;
static arr_t	*t_textures[MAX_CACHEDTEXTURES];

static int	t_tga888num;
static tga_t	*t_tga888[MAX_CACHEDTGA888];
static char	t_tga888ident[MAX_CACHEDTGA888][56]; // fake tga ident

//static arr_t* T_ReadArr( char* name );


static tga_t* T_ReadTGA888( const char *ident )
{
	char	name[256];
	FILE	*h;
	tga_t	*tga;

	sprintf( name, "%s/%s.tga", t_texturepath, ident );
	printf("T_ReadTGA888: name = %s\n", name );
	
	h = fopen( name, "rb" );

	if (!h) {
		printf( "can't load tga888 : %s\n", name ); 
		return NULL;
	}

	tga = TGA_Read( h );
	fclose( h );
	
	if (!tga) {
		printf( "TGA_Read failed.\n");
		return NULL;
	}

	return tga;
}

static arr_t* T_ReadArr( const char* ident )
{
	char	name[256];
	FILE	*h;
	arr_t	*texture;

	sprintf( name, "%s/%s.arr", t_texturepath, ident );
	printf("T_ReadArr: name = %s\n", name );
	
	h = fopen( name, "rb" );

	if (!h) {
		printf( "can't load texture : %s\n", name ); 
		return NULL;
	}

	texture = ARR_Read( h );
	fclose( h );

	if (!texture) {
		printf( "ARR_Read failed.\n");
		return NULL;
	}

	return texture;
	
}


void T_InitTexCache( void )
{
	FILE		*h;
	char*	palname;
	int		x, y;

	printf("T_InitTexCache\n");

//	CDB_StartUp( 0 );
	
	t_texturepath = CDB_GetString( "wired/texture_path" );
	if (!t_texturepath) {
		printf( "key wired/texture_path not found.\n");
		exit(-1);
	}

	palname = CDB_GetString( "wired/default_pal" );
	if (!palname) {
		printf("key wired/default_pal not found.\n");
		exit(-1);
	}

	h = fopen( palname, "rb" );
	if (!h) {
		printf(" can't open pal.\n");
		exit(-1);
	}

	t_pal = PAL_Read( h );
	fclose( h );
	if (!t_pal) {
		printf(" PAL_Read failed.\n" );
		exit(-1);
	}

	printf(" texturepath = %s\n", t_texturepath );
	printf(" palname = %s\n", palname );


	// 0 is the default texture
	t_texturenum = 1;
	t_tga888num = 1;

	printf(" creating default texture ...\n");

	t_textures[0] = ARR_Create( 64, 64, 1, 1, "default", ARR_F_RGB565 );
	
	for ( x = 0; x < 64; x++ ) {
		for ( y = 0; y < 64; y++ ) {
			if ( x&8 )
				*((unsigned short*)(&t_textures[0]->data[(x+y*64)*2])) = 0;
			else
				*((unsigned short*)(&t_textures[0]->data[(x+y*64)*2])) = 0xaaaa;
		}
	}

	t_tga888[0] = TGA_Create( 64, 64, TGA_TYPE_TRUECOLOR );
	for ( x = 0; x < 64; x++ ) {
		for ( y = 0; y < 64; y++ ) {
			if ( x&8 )
			{
				t_tga888[0]->image.red[x+y*64] = 0;
				t_tga888[0]->image.green[x+y*64] = 0;
				t_tga888[0]->image.blue[x+y*64] = 0;
			}
			else
			{
				t_tga888[0]->image.red[x+y*64] = 60;
				t_tga888[0]->image.green[x+y*64] = 120;
				t_tga888[0]->image.blue[x+y*64] = 60;
			}
		}
	}	
}

void T_DumpStat( void )
{
	printf("T_DumpStat\n");
	printf(" t_texturenum = %d\n", t_texturenum );
}

tga_t* T_GetTGA888ByName( const char *ident )
{
	int		i;
	tga_t		*tga;

	for ( i = 0; i < t_tga888num; i++ )
	{
		if ( !strcmp( t_tga888ident[i], ident ) )
			return t_tga888[i];
	}

	if ( t_tga888num < MAX_CACHEDTGA888 )
	{
		tga = T_ReadTGA888( ident );
		if ( !tga )
		{
			return t_tga888[0]; // use default
		}
		else
		{
			t_tga888[t_tga888num] = tga;
			strcpy( t_tga888ident[t_tga888num], ident );
			t_tga888num++;
			printf( " cache tga888.\n" );
			return t_tga888[t_tga888num-1];
		}
	}
	else
	{
		printf(" reached MAX_CACHEDTGA888.\n");
		return t_tga888[0]; // use default
	}	
	return NULL;
}

arr_t* T_GetTextureByName( const char *ident )
{
	int		i;
	arr_t		*arr;

	for ( i = 0; i < t_texturenum; i++ )
		
		if ( !strcmp( t_textures[i]->ident, ident ) )
			return t_textures[i];
	

	if ( t_texturenum < MAX_CACHEDTEXTURES ) {
		arr = T_ReadArr( ident );
		if (!arr) {
			return t_textures[0]; // use default
		}
		else {
			t_textures[t_texturenum++] = arr;
			printf(" cache texture.\n");	
			return t_textures[t_texturenum-1];
		}
	}
	else {
		printf(" reached MAX_CACHEDTEXTURES.\n");
		return t_textures[0]; // use default
	}	
	return NULL;
}
