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



#ifndef __CDB_SERVICE_HH_INCLUDED
#define __CDB_SERVICE_HH_INCLUDED
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

//#include <sys/mem.h>

#define CDB_FILE ".gsconf.cdb"
#define	CDB_ID	"LCDB"
#define CDB_KEY_SIZE ( 32 )
#define CDB_MAX_KEY_SIZE ( CDB_KEY_SIZE - 1 )  
#define CDB_STRING_SIZE ( 256 )
#define CDB_MAX_STRING_SIZE ( CDB_STRING_SIZE - 1 )

#ifdef __cplusplus
extern "C"
{
#endif


void	CDB_StartUp( int );
void	CDB_Save();
void	CDB_ChangeIntEntry( const char*, int );
int	CDB_GetIntValue( const char* );
void	CDB_ChangeStringEntry( const char*, char* );
char*	CDB_GetString( const char* );
void	CDB_RemoveIntEntry( const char* );
void	CDB_RemoveStringEntry( const char* );

#endif
#ifdef __cplusplus
}
#endif

