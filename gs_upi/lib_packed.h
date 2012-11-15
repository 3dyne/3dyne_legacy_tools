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



 // u_packed.h

#ifndef __u_packed
#define __u_packed

#define	PACKED_ASC		( 1 )
#define PACKED_BIN		( 2 )
#define PACKED_BIG		( 4 )
#define PACKED_LITTLE		( 8 )
#define PACKED_MACHINE_BIN	( 16 )

void BeginPack( int mode, void *ptr, int maxsize );
int EndPack( void );	// returns actually packed bytes
void BeginUnpack( int mode, void *ptr, int size );
int EndUnpack( void ); // returns actually unpacked bytes

void * GetPackPtr( void );
int	GetPackSize( void );

void Packs8( char );
void Packs16( short );
void Packs32( int );
void Packfp32( float );
void PackntString( char * );
void PackString( char *, int size );

void Unpacks8( char * );
void Unpacks16( short * );
void Unpacks32( int * );
void Unpackfp32( float * );
void UnpackntString( char *, int maxsize );
void UnpackString( char *, int size );
void UnpackSkip( int );

#endif
