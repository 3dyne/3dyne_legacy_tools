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



// Customize.hh

#ifndef __Customize
#define __Customize

#include <qcolor.h>
#include <qpixmap.h>

//
// common defines
//

#define		BRUSH_CONTENTS_SOLID	( 16 )
#define		BRUSH_CONTENTS_LIQUID	( 8 )
#define		BRUSH_CONTENTS_DECO	( 6 )
#define		BRUSH_CONTENTS_LOCAL	( 4 )
#define 	BRUSH_CONTENTS_SUBSTRUCT	( 5 )

#define		SURFACE_CONTENTS_OPEN		( 1 )
#define		SURFACE_CONTENTS_CLOSE		( 2 )
#define		SURFACE_CONTENTS_TEXTURE	( 4 )
#define		SURFACE_CONTENTS_WINDOW		( 8 )
#define		SURFACE_CONTENTS_SF_CLOSE	( 16 )
#define		SURFACE_CONTENTS_SUBSTRUCT	( 32 )

#define C_SURFACE_CONTENTS_OPEN		"1"
#define C_SURFACE_CONTENTS_CLOSE	"2"
#define C_SURFACE_CONTENTS_TEXTURE	"4"
#define C_SURFACE_CONTENTS_WINDOW	"8"
#define C_SURFACE_CONTENTS_SF_CLOSE	"16"
#define C_SURFACE_CONTENTS_SUBSTRUCT	"32"

#define C_BRUSH_CONTENTS_SOLID		"16"
#define C_BRUSH_CONTENTS_LIQUID		"8"
#define C_BRUSH_CONTENTS_DECO		"6"
#define C_BRUSH_CONTENTS_LOCAL		"4"
#define C_BRUSH_CONTENTS_SUBSTRUCT	"5"

#define DEFAULT_BRUSH_CONTENTS		( BRUSH_CONTENTS_SOLID )
#define DEFAULT_SURFACE_CONTENTS	( SURFACE_CONTENTS_CLOSE | SURFACE_CONTENTS_TEXTURE )

//
// colors of wired
//

extern QColor		*colorblack_i;
extern QColor		*colorblue_i;
extern QColor		*colorred_i;
extern QColor		*coloryellow_i;
extern QColor		*colorgreen_i;
extern QColor		*colorviolet_i;
extern QColor		*colorgray30_i;
extern QColor		*colordgreen_i;	// hint brush
extern QColor		*colordblue_i;	// liquid brush



//
// pixmaps of wired
//

extern QPixmap		*xpm_arche_i;
extern QPixmap		*xpm_atflood_i;
extern QPixmap		*xpm_atlight_i;
extern QPixmap		*xpm_atspot_i;
extern QPixmap		*xpm_atdlight_i;
extern QPixmap		*xpm_atswitch_i;

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

extern const char	*c_att[];
extern const char	*c_surfaces[];
extern const char	*c_brushes[];

void InitCustomize( void );

unsigned int OrNumberString( const char *text );

#endif
