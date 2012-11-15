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



// shock.h

#ifndef __shock_include
#define __shock_include

//#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#ifdef __cplusplus
extern "C"
{
#endif
// It's filthy, it's dirty 
///                         and it's highly fragile!

//#define ERROR SOS_Seperator(), SOS_Locate( __FILE__, __PRETTY_FUNCTION__, __LINE__), SOS_Error 
#define WARNING printf("Warning: %s: ",__PRETTY_FUNCTION__), SOS_Message
#define MESSAGE SOS_Message
#define NAMED_MESSAGE printf("%s: ",__PRETTY_FUNCTION__), SOS_Message
#define LOCATE SOS_Locate( __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define CHKPTR(x) SOS_ChkPtr(__FILE__, __PRETTY_FUNCTION__, __LINE__, x)

// new shock style 

#define __locate		SOS_Locate( __FILE__, __PRETTY_FUNCTION__, __LINE__)
#define __message		SOS_Message
#define __named_message		SOS_LocateFunction( __PRETTY_FUNCTION__ ), SOS_Message
#define __warning		SOS_LocateFunction( __PRETTY_FUNCTION__ ), \
				SOS_Message( "Warning: " ), SOS_Message
#define __error			SOS_Seperator(), \
				SOS_Locate( __FILE__, __PRETTY_FUNCTION__, __LINE__), \
				SOS_Error

#define __chkptr(ptr)	if ( ptr == NULL ) { \
				SOS_Seperator(); \
				SOS_Locate( __FILE__, __PRETTY_FUNCTION__, __LINE__); \
				SOS_Error("Null pointer.\n"); \
			}
	
#ifdef __DEBUG
#define __debug_message		SOS_Message
#else
#define __debug_message
#endif
	

// prototypes ...

void SOS_Seperator( void );
void SOS_Locate( const char* file, const char* func, int line);
void SOS_LocateFunction( const char* func );
void SOS_Error( const char* errtext_ptr, ... );
void SOS_Warning( const char* warntext_ptr, ... );
void SOS_Message( const char* text_ptr, ... );
void SOS_ChkPtr( char* file, char* func, int line, void* ptr );

void SOS_SetShockHandler( void func_shock() );
void SOS_CallShockHandler();
void SOS_DefaultShockHandler();

#ifdef __cplusplus
}
#endif
#endif