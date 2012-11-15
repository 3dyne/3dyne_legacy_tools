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



// gl_server.h

typedef unsigned int gls_cmd_t;

#define GLS_CMD_SIZE	( sizeof( unsigned int ) )

#define GLS_CMD_CONNECT		( ('C'<<24)+('O'<<16)+('N'<<8)+'N' )
#define GLS_CMD_DISCONNECT	( ('D'<<24)+('I'<<16)+('S'<<8)+'C' )	

#define	GLS_CMD_BEGINLIST	( ('B'<<24)+('L'<<16)+('S'<<8)+'T' )
#define GLS_CMD_ENDLIST		( ('E'<<24)+('L'<<16)+('S'<<8)+'T' )
#define GLS_CMD_POLYGON		( ('P'<<24)+('O'<<16)+('L'<<8)+'Y' )

#define GLS_CMD_SETCOLOR	( ('S'<<24)+('C'<<16)+('O'<<8)+'L' )

#define GLS_COLOR_WHITE		( 0 )
#define GLS_COLOR_RED		( 1 )
#define GLS_COLOR_GREEN		( 2 )
#define GLS_COLOR_BLUE		( 3 )
#define GLS_COLOR_YELLOW	( 4 )

#define GLS_COLOR_MAGIC_CLEAR_LISTS	( 255 )
