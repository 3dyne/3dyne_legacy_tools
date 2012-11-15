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



// Customize.cc

#include "Customize.hh"

#include <stdio.h>
#include <stdlib.h>

//
// colors
//

QColor		*colorblack_i;
QColor		*colorblue_i;
QColor		*colorred_i;
QColor		*coloryellow_i;
QColor		*colorgreen_i;
QColor		*colorviolet_i;
QColor		*colorgray30_i;
QColor		*colordgreen_i;	// hint brush
QColor		*colordblue_i;	// liquid brush

//
// pixmaps
// 
#include "b_arche.xpm"
#include "b_atlight.xpm"
#include "b_atspot.xpm"
#include "b_atflood.xpm"
#include "b_atdlight.xpm"
#include "b_atswitch.xpm"

QPixmap		*xpm_arche_i;
QPixmap		*xpm_atflood_i;
QPixmap		*xpm_atlight_i;
QPixmap		*xpm_atdlight_i;
QPixmap		*xpm_atspot_i;
QPixmap		*xpm_atswitch_i;

//
// archetype templates
//
/*
typedef struct {
	char	*menu_ident;
	char	*type;
	char	*key;
	char	*value;
} archetype_template_t;
*/
//archetype_template_t	c_att[] = { 
//	{ "light", "STRING

const char	*c_att[] = { "_light",
		     "STRING", "type", "light",
		     "VEC3D", "origin", "0 0 0",
		     "FLOAT", "value", "100",
		     "VEC3D", "color", "1 1 1",
		     "STRING", "style", "default",

		     "_spotlight",
		     "STRING", "type", "spotlight",
		     "VEC3D", "origin", "0 0 0",
		     "VEC3D", "direction", "0 -1 0",
		     "FLOAT", "value", "100",
		     "FLOAT", "angle", "45",
		     "FLOAT", "falloff", "0.8",
		     "VEC3D", "color", "1 1 1",

		     "_pos_player_start",
		     "STRING", "type", "pos_player_start",
		     "VEC3D", "origin", "0 0 0",
		     "VEC3D", "direction", "0 0 1",
		     
		     "_tool_chainsaw",
		     "STRING", "type", "tool_chainsaw",
		     "VEC3D", "origin", "0 0 0",
		     "VEC3D", "dirction", "0 0 1",
		     
		     "_sector",
		     "STRING", "type", "sector",
		     "STRING", "name", "none",
		     "VEC3D", "origin", "0 0 0",
		     "VEC3D", "ambient", "0 0 0",
		     "VEC3D", "fog", "0 0 0",
		     "FLOAT", "fog_dense", "0.1",

		     "_dlight",
		     "STRING", "type", "dlight",
		     "VEC3D", "origin", "0 0 0",
		     "FLOAT", "value", "100",
		     "VEC3D", "color", "1 1 1",		     
		     "STRING", "style", "default",
		     
		     "_archetype",
		     "STRING", "type", "generic",
		     "VEC3D", "origin", "0 0 0",
		     
		     "_switch",
		     "STRING", "type", "switch",
		     "VEC3D", "origin", "0 0 0",
		     "STRING", "style", "default",
		     "INT", "state", "0",
		     "FLOAT", "dist", "32.0",
		     
		     "_facelight",
		     "STRING", "type", "facelight",
		     "FLOAT", "dist", "24.0",
		     "STRING", "style", "default",
		     "VEC3D", "origin", "0 0 0",

		     NULL };


const char	*c_surfaces[] = { "open",	C_SURFACE_CONTENTS_OPEN,
			  "close",	C_SURFACE_CONTENTS_CLOSE,
			  "texture",	C_SURFACE_CONTENTS_TEXTURE,
			  "window",	C_SURFACE_CONTENTS_WINDOW,
			  "sf_close",	C_SURFACE_CONTENTS_SF_CLOSE,
			  "substruct",	C_SURFACE_CONTENTS_SUBSTRUCT,
			  NULL };

const char	*c_brushes[] = { "solid",	C_BRUSH_CONTENTS_SOLID, 
			 C_SURFACE_CONTENTS_CLOSE "|" C_SURFACE_CONTENTS_TEXTURE,

			 "liquid",	C_BRUSH_CONTENTS_LIQUID,
			 C_SURFACE_CONTENTS_OPEN "|" C_SURFACE_CONTENTS_TEXTURE "|" C_SURFACE_CONTENTS_WINDOW,

			 "deco",	C_BRUSH_CONTENTS_DECO,
			 C_SURFACE_CONTENTS_CLOSE "|" C_SURFACE_CONTENTS_TEXTURE,

			 "local",	C_BRUSH_CONTENTS_LOCAL,
			 C_SURFACE_CONTENTS_OPEN,

			 "substruct",	C_BRUSH_CONTENTS_SUBSTRUCT,
			 C_SURFACE_CONTENTS_CLOSE "|" C_SURFACE_CONTENTS_TEXTURE "|" C_SURFACE_CONTENTS_SUBSTRUCT,
			 NULL };

unsigned int OrNumberString( const char *text )
{
	char		tmp[256];
	char		c;
	unsigned int	value;
	int		i;

	value = 0;
	printf( "text: %s\n", text );
	for (;;)
	{
		for ( i=0;;)
		{
			c = *text;
			text++;
			tmp[i++] = c;
			if ( c == '|' || c == 0 )
			{
				tmp[i-1]=0;
				break;
			}
		}
		printf( "sub string: %s\n", tmp );
		value|=atoi(tmp);
		if ( c == 0 )
			break;
	}
	return value;
}

void InitCustomize( void )
{
	int		i;

	//
	// init colors
	// 
	colorblack_i = new QColor( "black" );
	colorblue_i = new QColor( "blue" );
	colorred_i = new QColor( "red" );
	coloryellow_i = new QColor( "yellow" );
	colorgreen_i = new QColor( "green4" );
	colorviolet_i = new QColor( "orchid4" );
	colorgray30_i = new QColor( "gray30" );
	colordgreen_i = new QColor( "lightgreen" );
	colordblue_i = new QColor( "lightblue" );


	//
	// init pixmaps
	//
	xpm_arche_i = new QPixmap(( const char** ) b_arche_xpm );
	xpm_atflood_i = new QPixmap(( const char** ) b_atflood_xpm );
	xpm_atlight_i = new QPixmap(( const char** ) b_atlight_xpm );
	xpm_atspot_i = new QPixmap(( const char** ) b_atspot_xpm );
	xpm_atdlight_i = new QPixmap(( const char** ) b_atdlight_xpm );
	xpm_atswitch_i = new QPixmap(( const char** ) b_atswitch_xpm );

	printf( "c_surf:\n" );
	for ( i = 0; c_surfaces[i]; i++ )
		printf( " %s\n", c_surfaces[i] );
}
