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



// gl_client.h

#ifndef __gl_client
#define __gl_client

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include "gl_server.h"
#include "lib_math.h"
#include "lib_poly.h"

#define GLC_COLOR_WHITE		( GLS_COLOR_WHITE )
#define GLC_COLOR_RED		( GLS_COLOR_RED )
#define GLC_COLOR_GREEN		( GLS_COLOR_GREEN )
#define GLC_COLOR_BLUE		( GLS_COLOR_BLUE )
#define GLC_COLOR_YELLOW	( GLS_COLOR_YELLOW )
#define GLC_COLOR_MAGIC_CLEAR_LISTS	( GLS_COLOR_MAGIC_CLEAR_LISTS )

void GLC_ConnectServer( char* server );
void GLC_DisconnectServer( void );
void GLC_BeginList( char *name, int enable );
void GLC_EndList( void );
void GLC_DrawPolygon( polygon_t *in );
void GLC_SetColor( int glc_color );

#endif
