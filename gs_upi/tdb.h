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



#ifndef __TDB_H_INCLUDED
#define __TDB_H_INCLUDED
#include <sys/types.h>
#define TDB_MAXENTRIES	( 1000 )
#define TDB_ENTRYOK	( 0 )
#define TDB_ENTRYEXISTS ( 1 )

extern int	matches[];

typedef struct {
	char	id[4];
	unsigned int	entrynum;
}	tdbheader_t;

typedef struct {
	int	creationtime;
	char	creator[32];
	char	arr[32];
	char	tga[256];
	char	comment[256];
}	tdbentry_t;

void	TDB_Load( char* );
void	TDB_Write( char* );

void	TDB_GetPointer( tdbentry_t** );
int	TDB_GetNum( void );

int	TDB_AddEntry( char*, char*, char*, char* );
int	TDB_RemoveEntry( char* );
int	TDB_Query( char*, char* );
void	TDB_Ls();
#endif
