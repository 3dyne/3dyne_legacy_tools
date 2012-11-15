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



// lib_gldbg.h

#ifndef __lib_gldbg
#define __lib_gldbg

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "lib_math.h"
#include "lib_poly.h"

typedef struct gld_session_s
{
	char		name[256];
	bool_t		record_active;
	FILE		*handle;
} gld_session_t;

gld_session_t * GLD_BeginSession( char *name );
void GLD_EndSession( gld_session_t *s );

void GLD_StartRecord( gld_session_t *s );
void GLD_StopRecord( gld_session_t *s );
bool_t GLD_IsRecording( gld_session_t *s );

// record cmds
void GLD_Pause( gld_session_t *s );
void GLD_Update( gld_session_t *s );

void GLD_BeginList( gld_session_t *s, char *name, char *polygon_mode );
void GLD_EndList( gld_session_t *s );

void GLD_Color3f( gld_session_t *s, fp_t r, fp_t g, fp_t b );

void GLD_Begin( gld_session_t *s, char *primitive );
void GLD_End( gld_session_t *s );
void GLD_Vertex3f( gld_session_t *s, fp_t x, fp_t y, fp_t z );
void GLD_Vertex3fv( gld_session_t *s, vec3d_t v );

// utils
void GLD_EasyPolygon( gld_session_t *s, polygon_t *poly );

#endif
